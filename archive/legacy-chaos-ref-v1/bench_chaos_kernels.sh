#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT_DIR"

STAMP="$(date +%Y%m%d_%H%M%S)"
mkdir -p results

# Defaults tuned for large images so runs finish in practical time.
DIMS="${BENCH_DIMS:-2048}"
LARGE_AT="${BENCH_LARGE_AT:-2048}"
WARM_SMALL="${BENCH_WARMUP:-1}"
RUNS_SMALL="${BENCH_RUNS:-3}"
WARM_LARGE="${BENCH_WARMUP_LARGE:-1}"
RUNS_LARGE="${BENCH_RUNS_LARGE:-4}"
CIPHERS="${BENCH_CIPHERS:-chaos}"
KERNELS="${KERNELS:-0 2}"

echo "[+] Benchmark stamp: $STAMP"
echo "[+] Kernels: $KERNELS"
echo "[+] Dims: $DIMS"
echo "[+] Ciphers: $CIPHERS"
echo "[+] Sampling: small(w=$WARM_SMALL,r=$RUNS_SMALL) large(w=$WARM_LARGE,r=$RUNS_LARGE @ >=$LARGE_AT)"

for K in $KERNELS; do
  BUILD_DIR="build-k${K}"
  LOG_PATH="results/bench_k${K}_${STAMP}.log"
  CSV_PATH="results/bench_results_k${K}_${STAMP}.csv"
  PROFILE_PATH="results/chaos_stage_profile_k${K}_${STAMP}.csv"

  echo
  echo "[+] Configuring kernel=$K in $BUILD_DIR"
  cmake -S . -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE=Release -DCHAOS_DIFFUSE_KERNEL="$K"
  cmake --build "$BUILD_DIR" -j

  RUNNER=("$BUILD_DIR/imgcrypt_bench")
  if [[ -n "${BENCH_CPU_AFFINITY:-}" ]] && command -v taskset >/dev/null 2>&1; then
    RUNNER=(taskset -c "$BENCH_CPU_AFFINITY" "$BUILD_DIR/imgcrypt_bench")
    echo "[+] CPU affinity: $BENCH_CPU_AFFINITY"
  fi

  echo "[+] Running kernel=$K ..."
  BENCH_DIMS="$DIMS" \
  BENCH_LARGE_AT="$LARGE_AT" \
  BENCH_WARMUP="$WARM_SMALL" \
  BENCH_RUNS="$RUNS_SMALL" \
  BENCH_WARMUP_LARGE="$WARM_LARGE" \
  BENCH_RUNS_LARGE="$RUNS_LARGE" \
  BENCH_CIPHERS="$CIPHERS" \
  "${RUNNER[@]}" | tee "$LOG_PATH"

  cp results/bench_results.csv "$CSV_PATH"
  cp results/chaos_stage_profile.csv "$PROFILE_PATH"

  echo "[+] Saved: $CSV_PATH"
  echo "[+] Saved: $PROFILE_PATH"
  echo "[+] Saved: $LOG_PATH"
done

echo
echo "[+] Done. Use the *_k0_* vs *_k2_* CSV files for publishable comparisons."
