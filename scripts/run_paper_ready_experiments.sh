#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT_DIR"

BUILD_DIR="${BUILD_DIR:-build}"
RESULTS_ROOT="${RESULTS_ROOT:-results/final}"
DATASET_DIR="${DATASET_DIR:-images/datasets/synthetic}"
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

python3 scripts/generate_synthetic_dataset.py --out "$DATASET_DIR" --sizes $SIZES --kinds $KINDS

mkdir -p "$RESULTS_ROOT/raw"

for size in $SIZES; do
  for kind in $KINDS; do
    image="$DATASET_DIR/${kind}_${size}x${size}.ppm"
    for rep in $(seq 1 "$REPS"); do
      if (( size <= FULL_MAX_SIZE )); then
        out="$RESULTS_ROOT/raw/full__${kind}_${size}x${size}__rep${rep}"
        "$BUILD_DIR/chaos_image_bench" --input "$image" --results "$out" --mode full
      fi

      if (( size <= SLOW_STAGE_MAX_SIZE )); then
        out="$RESULTS_ROOT/raw/stages__${kind}_${size}x${size}__rep${rep}"
        "$BUILD_DIR/chaos_image_bench" --input "$image" --results "$out" --mode stages
      else
        out="$RESULTS_ROOT/raw/stages_fast__${kind}_${size}x${size}__rep${rep}"
        "$BUILD_DIR/chaos_image_bench" --input "$image" --results "$out" --mode stages --fast-stages-only
      fi

      if (( size <= SLOW_CANDIDATE_MAX_SIZE )); then
        out="$RESULTS_ROOT/raw/candidates__${kind}_${size}x${size}__rep${rep}"
        "$BUILD_DIR/chaos_image_bench" --input "$image" --results "$out" --mode candidates
      else
        out="$RESULTS_ROOT/raw/candidates_fast__${kind}_${size}x${size}__rep${rep}"
        "$BUILD_DIR/chaos_image_bench" --input "$image" --results "$out" --mode candidates --fast-candidates-only
      fi
    done
  done
done

python3 scripts/make_paper_outputs.py --root "$RESULTS_ROOT"

echo "Wrote $RESULTS_ROOT"
echo "Wrote summary.out"
echo "Wrote paper_draft.md"
