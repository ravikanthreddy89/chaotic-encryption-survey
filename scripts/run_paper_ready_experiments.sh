#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT_DIR"

BUILD_DIR="${BUILD_DIR:-build}"
RESULTS_ROOT="${RESULTS_ROOT:-results/final}"
DATASET_DIR="${DATASET_DIR:-images/datasets/synthetic}"
REAL_DATASET_DIR="${REAL_DATASET_DIR:-images/datasets/real/kodak}"
DOWNLOAD_REAL="${DOWNLOAD_REAL:-1}"
REAL_LIMIT="${REAL_LIMIT:-24}"
REAL_REPS="${REAL_REPS:-$REPS}"
RUN_SYNTHETIC="${RUN_SYNTHETIC:-1}"
RUN_REAL="${RUN_REAL:-1}"
SIZES="${SIZES:-512 1024 2048 4096}"
KINDS="${KINDS:-gradient texture noise}"
REPS="${REPS:-5}"
FULL_MAX_SIZE="${FULL_MAX_SIZE:-1024}"
SLOW_STAGE_MAX_SIZE="${SLOW_STAGE_MAX_SIZE:-1024}"
SLOW_CANDIDATE_MAX_SIZE="${SLOW_CANDIDATE_MAX_SIZE:-2048}"
INCLUDE_8K="${INCLUDE_8K:-0}"

if [[ "$INCLUDE_8K" == "1" ]] && [[ " $SIZES " != *" 8192 "* ]]; then
  SIZES="$SIZES 8192"
fi

cmake -S . -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE=Release
cmake --build "$BUILD_DIR" --target chaos_image_bench -j

if [[ "$RUN_SYNTHETIC" == "1" ]]; then
  python3 scripts/generate_synthetic_dataset.py --out "$DATASET_DIR" --sizes $SIZES --kinds $KINDS
fi

if [[ "$RUN_REAL" == "1" && "$DOWNLOAD_REAL" == "1" ]]; then
  python3 scripts/download_real_datasets.py --out "$REAL_DATASET_DIR" --limit "$REAL_LIMIT"
fi

mkdir -p "$RESULTS_ROOT/raw"

run_one_image() {
  local image="$1"
  local label="$2"
  local size="$3"
  local reps="$4"

    for rep in $(seq 1 "$reps"); do
      if (( size <= FULL_MAX_SIZE )); then
        out="$RESULTS_ROOT/raw/full__${label}__rep${rep}"
        "$BUILD_DIR/chaos_image_bench" --input "$image" --results "$out" --mode full
      fi

      if (( size <= SLOW_STAGE_MAX_SIZE )); then
        out="$RESULTS_ROOT/raw/stages__${label}__rep${rep}"
        "$BUILD_DIR/chaos_image_bench" --input "$image" --results "$out" --mode stages
      else
        out="$RESULTS_ROOT/raw/stages_fast__${label}__rep${rep}"
        "$BUILD_DIR/chaos_image_bench" --input "$image" --results "$out" --mode stages --fast-stages-only
      fi

      if (( size <= SLOW_CANDIDATE_MAX_SIZE )); then
        out="$RESULTS_ROOT/raw/candidates__${label}__rep${rep}"
        "$BUILD_DIR/chaos_image_bench" --input "$image" --results "$out" --mode candidates
      else
        out="$RESULTS_ROOT/raw/candidates_fast__${label}__rep${rep}"
        "$BUILD_DIR/chaos_image_bench" --input "$image" --results "$out" --mode candidates --fast-candidates-only
      fi
    done
}

if [[ "$RUN_SYNTHETIC" == "1" ]]; then
  for size in $SIZES; do
    for kind in $KINDS; do
      image="$DATASET_DIR/${kind}_${size}x${size}.ppm"
      run_one_image "$image" "${kind}_${size}x${size}" "$size" "$REPS"
    done
  done
fi

if [[ "$RUN_REAL" == "1" ]]; then
  shopt -s nullglob
  for image in "$REAL_DATASET_DIR"/*.png "$REAL_DATASET_DIR"/*.jpg "$REAL_DATASET_DIR"/*.jpeg "$REAL_DATASET_DIR"/*.ppm; do
    stem="$(basename "$image")"
    stem="${stem%.*}"
    run_one_image "$image" "real_${stem}" "$FULL_MAX_SIZE" "$REAL_REPS"
  done
  shopt -u nullglob
fi

python3 scripts/make_paper_outputs.py --root "$RESULTS_ROOT"

echo "Wrote $RESULTS_ROOT"
echo "Wrote summary.out"
echo "Wrote paper_draft.md"
