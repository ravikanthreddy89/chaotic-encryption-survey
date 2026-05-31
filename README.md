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
- Benchmark CLI with correctness check, stage timing, and CSV export

## Build
```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
```

This builds two executables:
- `chaos_ref`: the Paper 1 shared-kernel benchmark.
- `chaos_image_bench`: the OpenCV/OpenSSL benchmark and cryptanalysis suite.

## CLI
```bash
./build/chaos_ref --list
./build/chaos_ref --buckets
./build/chaos_ref --roadmap

./build/chaos_ref --bench --size 1024 --channels 3 --warmup 1 --iters 4 \
  --scheme all --kernel all --csv results/bench_ref.csv

./build/chaos_ref --stages --size 1024 --channels 3 --warmup 1 --iters 4 \
  --scheme map --kernel all --stages-csv results/bench_stages.csv

./build/chaos_ref --microbench --size 8192 --channels 3 --warmup 1 --iters 4 \
  --micro-csv results/bench_micro.csv
```

Options:
- `--scheme all|map|sort`
- `--kernel all|scalar|scan`

## Image Benchmark Suite
```bash
./build/chaos_image_bench --input images/sample.png --results results
./build/chaos_image_bench --input images/sample.png --results results/stages --mode stages
./build/chaos_image_bench --input images/sample.png --results results/candidates --mode candidates
```

If the input image does not exist, the suite creates a deterministic sample image. It writes:
- `results/bench.csv`
- `results/analysis.csv`
- `results/stage_bench.csv`
- `results/candidate_schemes.csv`
- `results/security_summary.md`
- `results/encrypted/*.png`
- `results/histograms/*.png`

Implemented suite ciphers:
- chaotic: `logistic_xor`, `logistic_permute_xor`, `arnold_xor`, `tiled_arnold_xor`, `tent_block_xor`, `sine_xor`, `coupled_lattice_xor`, `hamiltonian_lattice_xor`, `chaotic_seed_blake3_xor`
- baselines: `aes_ctr`, `chacha20`

The suite also benchmarks replaceable primitives independently:
- keystream: logistic double, sine map, fixed-point tent, CML, cellular automata, Hamiltonian, reaction-diffusion
- permutation: chaotic sort, random walk, Arnold/cat map, affine modular, Feistel index, block permutation, checkerboard swaps
- diffusion: global chain, block-local chain, parallel prefix, stencil, ARX block, bit-plane

`--mode all|full|stages|candidates` controls which measurements run. Use `--fast-stages-only` for large images to skip sort/random-walk and other intentionally slow reference stages. Use `--fast-candidates-only` for 4K/8K candidate scaling without the Hamiltonian/CML stress cases.

## Q2/Q3 Paper-Ready Experiment Harness
```bash
REPS=5 SIZES="512 1024 2048 4096" ./scripts/run_paper_ready_experiments.sh
INCLUDE_8K=1 REPS=3 ./scripts/run_paper_ready_experiments.sh
REPS=10 REAL_REPS=10 SIZES="512 1024 2048 4096" ./scripts/run_paper_ready_experiments.sh
```

The final harness generates deterministic synthetic datasets, downloads the public Kodak PhotoCD real-image set by default, caps slow full-scheme measurements to practical sizes, scales SIMD-friendly stages/candidates to larger images, and writes:
- `results/final/bench.csv`, `analysis.csv`, `stage_bench.csv`, `candidate_schemes.csv`
- `results/final/*_stats.csv`
- `results/final/tables.md`
- `results/final/performance_summary.md`
- `results/final/metadata.md`
- `results/final/figures/*.png` using Matplotlib when available, otherwise dependency-free fallback bar charts
- `summary.out`
- `paper_draft.md`

The paper framing is applied performance: stage decomposition, SIMD-native redesign, and honest comparison to AES-CTR/ChaCha20. `chaotic_seed_blake3_xor` uses the official BLAKE3 C implementation as a keyed XOF keystream generator.

Real images are stored under `images/datasets/real/kodak`. To only download the dataset:
```bash
python3 scripts/download_real_datasets.py --out images/datasets/real/kodak --limit 24
```

## Batch run helper
```bash
WARMUP=1 ITERS=4 ./scripts/paper1_run_matrix.sh
```

The Paper 1 runner compares `scalar_build`, `sse2_build`, and `avx2_build`. By default it runs full map-pipeline cases through 8192, full sort-pipeline cases through 2048, and isolated diffusion/keygen microbenchmarks through 8192. Set `RUN_EXPENSIVE_SORT_8K=1` to include 4096/8192 sort cases.

## Notes
- AVX2 compile flags are enabled only on x86 targets.
- On non-x86 hosts, SIMD path falls back to portable implementation.
- `scalar_build` disables explicit SIMD intrinsics and compiler auto-vectorization for a true scalar baseline.

## Next milestones
1. Add user-provided public image datasets under `images/datasets/real`.
2. Add BLAKE3 SIMD dispatch sources if BLAKE3 itself should be measured with x86/ARM hardware acceleration.
3. Add deeper cryptanalysis before any security-deployment claim.
