#!/usr/bin/env python3
"""Download public real-image datasets used by the paper-ready benchmark.

The first supported dataset is the Kodak PhotoCD image set. These 24 natural
images are a common image-processing benchmark and are intentionally small
enough to keep repeated cryptographic benchmark runs manageable.
"""

from __future__ import annotations

import argparse
import sys
import time
import urllib.request
from pathlib import Path


KODAK_BASE = "https://r0k.us/graphics/kodak/kodak"
KODAK_IMAGES = [f"kodim{i:02d}.png" for i in range(1, 25)]


def download(url: str, path: Path, retries: int = 3) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    if path.exists() and path.stat().st_size > 0:
        print(f"exists {path}")
        return
    tmp = path.with_suffix(path.suffix + ".tmp")
    last_error: Exception | None = None
    for attempt in range(1, retries + 1):
        try:
            with urllib.request.urlopen(url, timeout=30) as response:
                data = response.read()
            if not data:
                raise RuntimeError("empty response")
            tmp.write_bytes(data)
            tmp.replace(path)
            print(f"downloaded {path}")
            return
        except Exception as exc:
            last_error = exc
            print(f"retry {attempt}/{retries} failed for {url}: {exc}", file=sys.stderr)
            time.sleep(1.0 * attempt)
    raise RuntimeError(f"failed to download {url}: {last_error}")


def main() -> int:
    p = argparse.ArgumentParser()
    p.add_argument("--out", default="images/datasets/real/kodak")
    p.add_argument("--limit", type=int, default=24)
    args = p.parse_args()

    out = Path(args.out)
    selected = KODAK_IMAGES[: max(1, min(args.limit, len(KODAK_IMAGES)))]
    for name in selected:
        download(f"{KODAK_BASE}/{name}", out / name)

    readme = out / "README.txt"
    readme.write_text(
        "Kodak PhotoCD image set\n"
        "Source: https://r0k.us/graphics/kodak/\n"
        "Use: real natural-image benchmark inputs for chaotic image encryption experiments.\n",
        encoding="utf-8",
    )
    print(readme)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
