# Chaos Image Encryption SIMD/AVX Reference Project

## Status
Legacy implementation was moved to:
- `archive/legacy-chaos-ref-v1/`

This repository now focuses on a publishable reference implementation for shared chaotic-encryption buckets.

## Implemented (v0)
- Bucket B1: map-based permutation (`Fridrich-style`)
- Bucket B2: sort-based permutation (`chaotic score argsort`)
- Bucket C1: scalar chained diffusion
- Bucket C2: exact scan-form diffusion (SIMD-capable path)
- Deterministic logistic-map keystream/scores
- Benchmark CLI with correctness check and CSV export

## Build
```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
```

## CLI
```bash
./build/chaos_ref --list
./build/chaos_ref --buckets
./build/chaos_ref --roadmap

./build/chaos_ref --bench --size 1024 --channels 3 --warmup 1 --iters 4 \
  --scheme all --kernel all --csv results/bench_ref.csv
```

Options:
- `--scheme all|map|sort`
- `--kernel all|scalar|scan`

## Batch run helper
```bash
SIZE=2048 WARMUP=1 ITERS=4 ./scripts/run_bench_matrix.sh
```

## Notes
- AVX2 compile flags are enabled only on x86 targets.
- On non-x86 hosts, SIMD path falls back to portable implementation.

## Next milestones
1. Add per-stage profiling (permute vs diffuse vs keygen).
2. Add OpenMP variants for map/sort kernels.
3. Add dataset loader and figure-generation scripts.
4. Add a third representative family (bit-plane / symbolic layer).
