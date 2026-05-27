#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT_DIR"

SIZES="${SIZES:-512 1024 2048}"
CHANNELS="${CHANNELS:-3}"
WARMUP="${WARMUP:-1}"
ITERS="${ITERS:-4}"
SCHEME="${SCHEME:-all}"
KERNEL="${KERNEL:-all}"
STAMP="$(date +%Y%m%d_%H%M%S)"
OUT_DIR="results/paper1_${STAMP}"
mkdir -p "$OUT_DIR"

run_mode() {
  local mode="$1"
  local avx_flag="$2"
  local build_dir="build-paper1-${mode}"

  cmake -S . -B "$build_dir" -DCMAKE_BUILD_TYPE=Release -DENABLE_AVX2="$avx_flag"
  cmake --build "$build_dir" -j

  for sz in $SIZES; do
    local csv_path="$OUT_DIR/${mode}_${sz}.csv"
    echo "[+] mode=${mode} size=${sz}"
    "$build_dir/chaos_ref" --bench \
      --size "$sz" \
      --channels "$CHANNELS" \
      --warmup "$WARMUP" \
      --iters "$ITERS" \
      --scheme "$SCHEME" \
      --kernel "$KERNEL" \
      --csv "$csv_path"
  done
}

run_mode "scalar_build" "OFF"
run_mode "simd_build" "ON"

COMBINED="$OUT_DIR/paper1_combined.csv"
first_file="$(ls "$OUT_DIR"/*.csv | head -n1)"
header="$(head -n1 "$first_file")"
echo "build_mode,$header" > "$COMBINED"

for f in "$OUT_DIR"/*.csv; do
  base="$(basename "$f")"
  mode="${base%%_*}"
  tail -n +2 "$f" | awk -v m="$mode" 'BEGIN{FS=OFS=","} {print m,$0}' >> "$COMBINED"
done

echo "[+] combined csv: $COMBINED"
echo "[+] generate markdown table with: python3 scripts/paper1_make_tables.py $COMBINED"
