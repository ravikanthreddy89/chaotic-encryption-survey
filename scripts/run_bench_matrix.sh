#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT_DIR"

SIZE="${SIZE:-1024}"
CHANNELS="${CHANNELS:-3}"
WARMUP="${WARMUP:-1}"
ITERS="${ITERS:-4}"
STAMP="$(date +%Y%m%d_%H%M%S)"
CSV="results/bench_matrix_${STAMP}.csv"

cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j

./build/chaos_ref --bench \
  --size "$SIZE" \
  --channels "$CHANNELS" \
  --warmup "$WARMUP" \
  --iters "$ITERS" \
  --scheme all \
  --kernel all \
  --csv "$CSV"

echo "[+] matrix csv: $CSV"
