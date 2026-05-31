#!/usr/bin/env python3
"""Generate deterministic PPM images for paper-ready benchmark runs."""

from __future__ import annotations

import argparse
import random
from pathlib import Path


def write_ppm(path: Path, width: int, height: int, kind: str) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    kind_seed = sum((i + 1) * ord(c) for i, c in enumerate(kind))
    rng = random.Random((width << 32) ^ height ^ kind_seed)
    data = bytearray(width * height * 3)
    for y in range(height):
        row = y * width * 3
        for x in range(width):
            i = row + x * 3
            if kind == "gradient":
                r = (x + y) & 255
                g = (2 * x + y // 2) & 255
                b = (x // 2 + 3 * y) & 255
            elif kind == "checker":
                v = 255 if ((x // 32) ^ (y // 32)) & 1 else 0
                r, g, b = v, 255 - v, (x + y) & 255
            elif kind == "flat":
                r, g, b = 96, 96, 96
            elif kind == "texture":
                r = ((x * 13) ^ (y * 7) ^ ((x * y) >> 3)) & 255
                g = ((x * 3 + y * 11) ^ (x >> 2)) & 255
                b = ((x * 5) ^ (y * 17) ^ (y >> 1)) & 255
            elif kind == "noise":
                r = rng.randrange(256)
                g = rng.randrange(256)
                b = rng.randrange(256)
            else:
                raise ValueError(f"unknown image kind: {kind}")
            data[i] = r
            data[i + 1] = g
            data[i + 2] = b
    with path.open("wb") as f:
        f.write(f"P6\n{width} {height}\n255\n".encode("ascii"))
        f.write(data)


def main() -> int:
    p = argparse.ArgumentParser()
    p.add_argument("--out", default="images/datasets/synthetic")
    p.add_argument("--sizes", nargs="+", type=int, default=[512, 1024, 2048, 4096])
    p.add_argument("--kinds", nargs="+", default=["gradient", "texture", "noise"])
    args = p.parse_args()

    out = Path(args.out)
    for size in args.sizes:
        for kind in args.kinds:
            path = out / f"{kind}_{size}x{size}.ppm"
            write_ppm(path, size, size, kind)
            print(path)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
