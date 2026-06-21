#!/usr/bin/env python3
"""Stream public Y4M video frames into publication-safe PPM inputs.

The script intentionally avoids ffmpeg so the experiment can run in minimal
CI/VM environments. It reads only the requested number of frames from a public
Y4M stream and writes RGB PPM frames at the source resolution and at 720p.
"""

from __future__ import annotations

import argparse
import re
import sys
import time
import urllib.request
from pathlib import Path
from typing import BinaryIO


DEFAULT_URL = "https://media.xiph.org/video/derf/y4m/ducks_take_off_1080p50.y4m"


def read_line(stream: BinaryIO) -> bytes:
    line = bytearray()
    while True:
        c = stream.read(1)
        if not c:
            raise EOFError("unexpected end of Y4M stream")
        line.extend(c)
        if c == b"\n":
            return bytes(line)


def parse_header(line: bytes) -> tuple[int, int, str]:
    text = line.decode("ascii", errors="replace").strip()
    if not text.startswith("YUV4MPEG2 "):
        raise RuntimeError(f"not a Y4M stream: {text[:80]}")
    width_m = re.search(r"(?:^| )W(\d+)", text)
    height_m = re.search(r"(?:^| )H(\d+)", text)
    chroma_m = re.search(r"(?:^| )C([^ ]+)", text)
    if not width_m or not height_m:
        raise RuntimeError(f"Y4M width/height missing: {text}")
    chroma = chroma_m.group(1) if chroma_m else "420"
    return int(width_m.group(1)), int(height_m.group(1)), chroma


def clamp(v: int) -> int:
    return 0 if v < 0 else 255 if v > 255 else v


def yuv_to_rgb(y: int, u: int, v: int) -> tuple[int, int, int]:
    c = y - 16
    d = u - 128
    e = v - 128
    r = (298 * c + 409 * e + 128) >> 8
    g = (298 * c - 100 * d - 208 * e + 128) >> 8
    b = (298 * c + 516 * d + 128) >> 8
    return clamp(r), clamp(g), clamp(b)


def write_ppm_420(path: Path,
                  y_plane: bytes,
                  u_plane: bytes,
                  v_plane: bytes,
                  src_w: int,
                  src_h: int,
                  dst_w: int,
                  dst_h: int) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    with path.open("wb") as f:
        f.write(f"P6\n{dst_w} {dst_h}\n255\n".encode("ascii"))
        row = bytearray(dst_w * 3)
        for dy in range(dst_h):
            sy = min(src_h - 1, dy * src_h // dst_h)
            uv_y = sy // 2
            for dx in range(dst_w):
                sx = min(src_w - 1, dx * src_w // dst_w)
                y = y_plane[sy * src_w + sx]
                uv_i = uv_y * (src_w // 2) + (sx // 2)
                r, g, b = yuv_to_rgb(y, u_plane[uv_i], v_plane[uv_i])
                i = dx * 3
                row[i] = r
                row[i + 1] = g
                row[i + 2] = b
            f.write(row)


def stream_frames(url: str, out: Path, limit: int, stride: int, include_1080p: bool) -> None:
    out.mkdir(parents=True, exist_ok=True)
    request = urllib.request.Request(url, headers={"User-Agent": "chaotic-image-encryption-benchmark/1.0"})
    with urllib.request.urlopen(request, timeout=60) as response:
        width, height, chroma = parse_header(read_line(response))
        if not chroma.startswith("420"):
            raise RuntimeError(f"unsupported Y4M chroma format: {chroma}")
        y_size = width * height
        uv_size = (width // 2) * (height // 2)
        written = 0
        seen = 0
        while written < limit:
            frame_header = read_line(response)
            if not frame_header.startswith(b"FRAME"):
                raise RuntimeError(f"expected FRAME header, got {frame_header[:80]!r}")
            y_plane = response.read(y_size)
            u_plane = response.read(uv_size)
            v_plane = response.read(uv_size)
            if len(y_plane) != y_size or len(u_plane) != uv_size or len(v_plane) != uv_size:
                raise EOFError("truncated Y4M frame")
            if seen % stride == 0:
                frame_no = written + 1
                if include_1080p:
                    write_ppm_420(out / "1080p" / f"frame_{frame_no:04d}.ppm",
                                  y_plane, u_plane, v_plane, width, height, 1920, 1080)
                write_ppm_420(out / "720p" / f"frame_{frame_no:04d}.ppm",
                              y_plane, u_plane, v_plane, width, height, 1280, 720)
                print(f"wrote frame {frame_no}/{limit}")
                written += 1
            seen += 1

    (out / "README.txt").write_text(
        "Xiph Derf Y4M video-frame workload\n"
        f"Source: {url}\n"
        "Default sequence: ducks_take_off_1080p50.y4m\n"
        "Use: real-time frame-latency benchmark inputs for image encryption experiments.\n",
        encoding="utf-8",
    )


def main() -> int:
    p = argparse.ArgumentParser()
    p.add_argument("--url", default=DEFAULT_URL)
    p.add_argument("--out", default="images/datasets/real/video_xiph_ducks")
    p.add_argument("--limit", type=int, default=12)
    p.add_argument("--stride", type=int, default=10)
    p.add_argument("--skip-1080p", action="store_true")
    args = p.parse_args()

    if args.limit < 1:
        raise SystemExit("--limit must be positive")
    if args.stride < 1:
        raise SystemExit("--stride must be positive")

    last_error: Exception | None = None
    for attempt in range(1, 4):
        try:
            stream_frames(args.url, Path(args.out), args.limit, args.stride, not args.skip_1080p)
            return 0
        except Exception as exc:
            last_error = exc
            print(f"retry {attempt}/3 failed: {exc}", file=sys.stderr)
            time.sleep(attempt)
    raise RuntimeError(f"failed to download video frames: {last_error}")


if __name__ == "__main__":
    raise SystemExit(main())
