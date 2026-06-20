#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT_DIR"

SIZES="${SIZES:-512 1024 2048 4096}"
REPS="${REPS:-10}"
WARMUP="${WARMUP:-2}"
ITERS="${ITERS:-6}"
REAL_IMAGE_LIMIT="${REAL_IMAGE_LIMIT:-0}"
DATASET_ROOT="${DATASET_ROOT:-images/datasets/real}"
OUT_DIR="${OUT_DIR:-results/cross_arch_$(date -u +%Y%m%d_%H%M%S)}"
PLATFORM_ID="${PLATFORM_ID:-$(uname -m)}"
mkdir -p "$OUT_DIR/raw"

ARCH="$(uname -m)"
case "$ARCH" in
  x86_64|amd64)
    NATIVE_MODE="avx2_build"
    NATIVE_AVX2="ON"
    ;;
  aarch64|arm64)
    NATIVE_MODE="neon_build"
    NATIVE_AVX2="OFF"
    ;;
  *)
    echo "unsupported publication architecture: $ARCH" >&2
    exit 1
    ;;
esac

configure_build() {
  local mode="$1"
  local simd="$2"
  local avx2="$3"
  local autovec="$4"
  local build_dir="build-cross-${mode}"
  cmake -S . -B "$build_dir" -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON \
    -DENABLE_SIMD_INTRINSICS="$simd" -DENABLE_AVX2="$avx2" \
    -DDISABLE_AUTO_VECTORIZATION="$autovec" -DENABLE_NATIVE_ARCH=OFF
  cmake --build "$build_dir" -j
  ctest --test-dir "$build_dir" --output-on-failure
}

configure_build scalar_build OFF OFF ON
configure_build "$NATIVE_MODE" ON "$NATIVE_AVX2" OFF

FULL_CSV="$OUT_DIR/full_results.csv"
STAGE_CSV="$OUT_DIR/stage_results.csv"
echo "platform,build_mode,repeat,$(./build-cross-scalar_build/chaos_ref --bench --size 1 --iters 1 --csv /tmp/chaosref_schema.csv >/dev/null; head -n1 /tmp/chaosref_schema.csv)" > "$FULL_CSV"
echo "platform,build_mode,repeat,$(./build-cross-scalar_build/chaos_ref --stages --size 1 --iters 1 --stages-csv /tmp/chaosref_stage_schema.csv >/dev/null; head -n1 /tmp/chaosref_stage_schema.csv)" > "$STAGE_CSV"

append_rows() {
  local destination="$1"
  local mode="$2"
  local repeat="$3"
  local source="$4"
  tail -n +2 "$source" | awk -v p="$PLATFORM_ID" -v m="$mode" -v r="$repeat" \
    'BEGIN { FS=OFS="," } { print p,m,r,$0 }' >> "$destination"
}

run_case() {
  local mode="$1"
  local repeat="$2"
  local scheme="$3"
  local label="$4"
  shift 4
  local build_dir="build-cross-${mode}"
  local full="$OUT_DIR/raw/${mode}__rep${repeat}__${label}__${scheme}.csv"
  local stages="$OUT_DIR/raw/${mode}__rep${repeat}__${label}__${scheme}__stages.csv"
  local order_seed=$((2026 + repeat * 101))
  "$build_dir/chaos_ref" --bench "$@" --warmup "$WARMUP" --iters "$ITERS" \
    --scheme "$scheme" --kernel all --order-seed "$order_seed" --csv "$full"
  "$build_dir/chaos_ref" --stages "$@" --warmup "$WARMUP" --iters "$ITERS" \
    --scheme "$scheme" --kernel all --order-seed "$order_seed" --stages-csv "$stages"
  append_rows "$FULL_CSV" "$mode" "$repeat" "$full"
  append_rows "$STAGE_CSV" "$mode" "$repeat" "$stages"
}

REAL_IMAGES=()
while IFS= read -r image; do
  REAL_IMAGES+=("$image")
done < <(find "$DATASET_ROOT" -type f \( -iname '*.png' -o -iname '*.ppm' -o -iname '*.tif' -o -iname '*.tiff' \) 2>/dev/null | sort)
if [ "$REAL_IMAGE_LIMIT" -gt 0 ]; then
  REAL_IMAGES=("${REAL_IMAGES[@]:0:$REAL_IMAGE_LIMIT}")
fi

for mode in scalar_build "$NATIVE_MODE"; do
  for repeat in $(seq 1 "$REPS"); do
    for size in $SIZES; do
      for scheme in map bitplane symbolic; do
        run_case "$mode" "$repeat" "$scheme" "synthetic_${size}" --size "$size" --seed "$repeat"
      done
      if [ "$size" -le 2048 ]; then
        run_case "$mode" "$repeat" sort "synthetic_${size}" --size "$size" --seed "$repeat"
      fi
    done
    for image in "${REAL_IMAGES[@]}"; do
      label="$(basename "$image")"
      label="${label//[^A-Za-z0-9_.-]/_}"
      for scheme in map sort bitplane symbolic; do
        run_case "$mode" "$repeat" "$scheme" "$label" --input "$image"
      done
    done
  done
done

{
  echo "timestamp_utc=$(date -u +%Y-%m-%dT%H:%M:%SZ)"
  echo "platform_id=$PLATFORM_ID"
  echo "architecture=$ARCH"
  echo "native_mode=$NATIVE_MODE"
  echo "sizes=$SIZES"
  echo "repetitions=$REPS"
  echo "warmup=$WARMUP"
  echo "iterations=$ITERS"
  echo "dataset_root=$DATASET_ROOT"
  echo "real_images=${#REAL_IMAGES[@]}"
  echo "git_head=$(git rev-parse HEAD 2>/dev/null || echo unavailable)"
  uname -a
  "${CXX:-c++}" --version | head -n1
  lscpu 2>/dev/null || true
  sysctl -n machdep.cpu.brand_string 2>/dev/null || true
  sysctl -n hw.model 2>/dev/null || true
  sysctl -n hw.ncpu 2>/dev/null || true
} > "$OUT_DIR/metadata.txt"

python3 scripts/paired_speedup_analysis.py "$FULL_CSV" --candidate "$NATIVE_MODE" \
  --output "$OUT_DIR/paired_speedups.csv" > "$OUT_DIR/paired_speedups.md"

echo "publication results: $OUT_DIR"
