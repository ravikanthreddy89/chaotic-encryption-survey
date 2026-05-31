#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT_DIR"

BUILD_DIR="${BUILD_DIR:-build}"
INPUT="${INPUT:-images/sample.png}"
RESULTS_DIR="${RESULTS_DIR:-results}"

cmake -S . -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE=Release
cmake --build "$BUILD_DIR" --target chaos_image_bench -j
"$BUILD_DIR/chaos_image_bench" --input "$INPUT" --results "$RESULTS_DIR"
