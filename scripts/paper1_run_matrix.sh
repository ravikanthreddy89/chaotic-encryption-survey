#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT_DIR"

MAP_SIZES="${MAP_SIZES:-${SIZES:-512 1024 2048 4096 8192}}"
SORT_SIZES="${SORT_SIZES:-512 1024 2048}"
MICRO_SIZES="${MICRO_SIZES:-${SIZES:-512 1024 2048 4096 8192}}"
CHANNELS="${CHANNELS:-3}"
WARMUP="${WARMUP:-1}"
ITERS="${ITERS:-4}"
KERNEL="${KERNEL:-all}"
PIN_CPU="${PIN_CPU:-0}"
RUN_EXPENSIVE_SORT_8K="${RUN_EXPENSIVE_SORT_8K:-0}"
STAMP="$(date +%Y%m%d_%H%M%S)"
OUT_DIR="results/paper1_${STAMP}"
mkdir -p "$OUT_DIR"

if [ "$RUN_EXPENSIVE_SORT_8K" = "1" ]; then
  SORT_SIZES="${SORT_SIZES} 4096 8192"
fi

PIN_CMD=()
PIN_STATUS="not requested"
if command -v taskset >/dev/null 2>&1; then
  if taskset -c "$PIN_CPU" true >/dev/null 2>&1; then
    PIN_CMD=(taskset -c "$PIN_CPU")
    PIN_STATUS="taskset -c ${PIN_CPU}"
  else
    PIN_STATUS="taskset unavailable for CPU ${PIN_CPU}; ran unpinned"
  fi
else
  PIN_STATUS="taskset not found; ran unpinned"
fi

write_metadata() {
  local meta="$OUT_DIR/paper1_metadata.txt"
  {
    echo "Paper 1 benchmark metadata"
    echo "timestamp_utc=$(date -u +%Y-%m-%dT%H:%M:%SZ)"
    echo "root_dir=$ROOT_DIR"
    echo "map_sizes=$MAP_SIZES"
    echo "sort_sizes=$SORT_SIZES"
    echo "micro_sizes=$MICRO_SIZES"
    echo "channels=$CHANNELS"
    echo "warmup=$WARMUP"
    echo "iters=$ITERS"
    echo "kernel=$KERNEL"
    echo "run_expensive_sort_8k=$RUN_EXPENSIVE_SORT_8K"
    echo "pinning=$PIN_STATUS"
    echo
    echo "[host]"
    uname -a || true
    if command -v lscpu >/dev/null 2>&1; then
      lscpu | sed -n '/^Architecture:/p;/^CPU op-mode/p;/^CPU(s):/p;/^On-line CPU(s) list:/p;/^Model name:/p;/^Thread(s) per core:/p;/^Core(s) per socket:/p;/^Socket(s):/p;/^Flags:/p'
    elif [ -r /proc/cpuinfo ]; then
      sed -n '/^model name/p;/^flags/p' /proc/cpuinfo | head -n 2
    fi
    echo
    echo "[compiler]"
    "${CXX:-c++}" --version | head -n 1 || true
    echo "CXX=${CXX:-c++}"
    echo "CMAKE_BUILD_TYPE=Release"
    echo "CMAKE_CXX_FLAGS_RELEASE=-O3 -fno-omit-frame-pointer -DNDEBUG"
    echo "scalar_build: ENABLE_SIMD_INTRINSICS=OFF ENABLE_AVX2=OFF DISABLE_AUTO_VECTORIZATION=ON ENABLE_NATIVE_ARCH=OFF"
    echo "sse2_build: ENABLE_SIMD_INTRINSICS=ON ENABLE_AVX2=OFF DISABLE_AUTO_VECTORIZATION=OFF ENABLE_NATIVE_ARCH=OFF"
    echo "avx2_build: ENABLE_SIMD_INTRINSICS=ON ENABLE_AVX2=ON DISABLE_AUTO_VECTORIZATION=OFF ENABLE_NATIVE_ARCH=OFF"
  } > "$meta"
  echo "[+] metadata: $meta"
}

run_chaos_ref() {
  if [ "${#PIN_CMD[@]}" -gt 0 ]; then
    "${PIN_CMD[@]}" "$@"
  else
    "$@"
  fi
}

combine_csvs() {
  local out_csv="$1"
  shift

  local first_file=""
  for f in "$@"; do
    if [ -f "$f" ]; then
      first_file="$f"
      break
    fi
  done

  if [ -z "$first_file" ]; then
    echo "[!] no input csv files for $out_csv"
    return
  fi

  local header
  header="$(head -n1 "$first_file")"
  echo "build_mode,$header" > "$out_csv"

  for f in "$@"; do
    if [ ! -f "$f" ]; then
      continue
    fi
    local base
    local mode
    base="$(basename "$f")"
    mode="${base%%__*}"
    tail -n +2 "$f" | awk -v m="$mode" 'BEGIN{FS=OFS=","} {print m,$0}' >> "$out_csv"
  done
}

run_mode() {
  local mode="$1"
  local simd_flag="$2"
  local avx_flag="$3"
  local autovec_flag="$4"
  local native_flag="$5"
  local build_dir="build-paper1-${mode}"

  cmake -S . -B "$build_dir" \
    -DCMAKE_BUILD_TYPE=Release \
    -DENABLE_SIMD_INTRINSICS="$simd_flag" \
    -DENABLE_AVX2="$avx_flag" \
    -DDISABLE_AUTO_VECTORIZATION="$autovec_flag" \
    -DENABLE_NATIVE_ARCH="$native_flag"
  cmake --build "$build_dir" -j

  for sz in $MAP_SIZES; do
    local csv_path="$OUT_DIR/${mode}__full_map_${sz}.csv"
    local stages_csv_path="$OUT_DIR/${mode}__stages_map_${sz}.csv"
    echo "[+] mode=${mode} full/stages scheme=map size=${sz}"
    run_chaos_ref "$build_dir/chaos_ref" --bench \
      --size "$sz" \
      --channels "$CHANNELS" \
      --warmup "$WARMUP" \
      --iters "$ITERS" \
      --scheme map \
      --kernel "$KERNEL" \
      --csv "$csv_path"
    run_chaos_ref "$build_dir/chaos_ref" --stages \
      --size "$sz" \
      --channels "$CHANNELS" \
      --warmup "$WARMUP" \
      --iters "$ITERS" \
      --scheme map \
      --kernel "$KERNEL" \
      --stages-csv "$stages_csv_path"
  done

  for sz in $SORT_SIZES; do
    local csv_path="$OUT_DIR/${mode}__full_sort_${sz}.csv"
    local stages_csv_path="$OUT_DIR/${mode}__stages_sort_${sz}.csv"
    echo "[+] mode=${mode} full/stages scheme=sort size=${sz}"
    run_chaos_ref "$build_dir/chaos_ref" --bench \
      --size "$sz" \
      --channels "$CHANNELS" \
      --warmup "$WARMUP" \
      --iters "$ITERS" \
      --scheme sort \
      --kernel "$KERNEL" \
      --csv "$csv_path"
    run_chaos_ref "$build_dir/chaos_ref" --stages \
      --size "$sz" \
      --channels "$CHANNELS" \
      --warmup "$WARMUP" \
      --iters "$ITERS" \
      --scheme sort \
      --kernel "$KERNEL" \
      --stages-csv "$stages_csv_path"
  done

  for sz in $MICRO_SIZES; do
    local micro_csv_path="$OUT_DIR/${mode}__micro_${sz}.csv"
    echo "[+] mode=${mode} microbench size=${sz}"
    run_chaos_ref "$build_dir/chaos_ref" --microbench \
      --size "$sz" \
      --channels "$CHANNELS" \
      --warmup "$WARMUP" \
      --iters "$ITERS" \
      --micro-csv "$micro_csv_path"
  done
}

write_metadata
run_mode "scalar_build" "OFF" "OFF" "ON" "OFF"
run_mode "sse2_build" "ON" "OFF" "OFF" "OFF"
run_mode "avx2_build" "ON" "ON" "OFF" "OFF"

FULL_COMBINED="$OUT_DIR/paper1_combined.csv"
STAGES_COMBINED="$OUT_DIR/paper1_stages_combined.csv"
MICRO_COMBINED="$OUT_DIR/paper1_micro_combined.csv"

combine_csvs "$FULL_COMBINED" "$OUT_DIR"/*__full_*.csv
combine_csvs "$STAGES_COMBINED" "$OUT_DIR"/*__stages_*.csv
combine_csvs "$MICRO_COMBINED" "$OUT_DIR"/*__micro_*.csv

echo "[+] combined full csv: $FULL_COMBINED"
echo "[+] combined stage csv: $STAGES_COMBINED"
echo "[+] combined micro csv: $MICRO_COMBINED"
echo "[+] generate full table with: python3 scripts/paper1_make_tables.py $FULL_COMBINED"
echo "[+] generate stage table with: python3 scripts/paper1_make_stage_tables.py $STAGES_COMBINED $MICRO_COMBINED"
