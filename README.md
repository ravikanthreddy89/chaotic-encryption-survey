# SIMD-Native Chaotic Image Encryption Benchmark

A reproducible C++17 benchmark suite for studying the performance and security
tradeoffs of chaotic image-encryption pipelines.

The project decomposes image encryption into three replaceable stages:

1. **Keystream generation**
2. **Permutation**
3. **Diffusion**

It measures each stage independently and as part of complete pipelines. The
main research question is whether chaotic image transforms become more
efficient when sequential, irregular operations are replaced with
SIMD-friendly designs.

The suite includes scalar and AVX2 implementations, OpenSSL AES-256-CTR and
ChaCha20 baselines, image-domain security diagnostics, public dataset tooling,
CSV output, publication plots, and an IEEE Access manuscript.

> **Security notice:** The experimental chaotic and candidate pipelines are
> research prototypes. They have not received the cryptanalysis required for
> production use. Use a standardized authenticated-encryption scheme for real
> applications.

## Highlights

- Stage-level timing for keystream generation, permutation, diffusion, and
  total runtime
- Scalar, SSE2/AVX2-oriented, and portable comparison paths
- Full-scheme AES-256-CTR, ChaCha20, BLAKE3-XOR, and chaotic-map baselines
- SIMD-friendly permutation and diffusion candidates
- Entropy, chi-square, adjacent-pixel correlation, NPCR, UACI, key
  sensitivity, known-plaintext, and chosen-plaintext diagnostics
- Synthetic and Kodak PhotoCD dataset support
- Reproducible CSV aggregation, confidence intervals, plots, and manuscript

## Requirements

- CMake 3.16 or newer
- C++17 compiler
- OpenCV development libraries
- OpenSSL development libraries
- Python 3 for dataset, aggregation, and plotting scripts

On Ubuntu or Debian:

```bash
sudo apt-get install build-essential cmake libopencv-dev libssl-dev python3-pip
python3 -m pip install --user -r scripts/plot_requirements.txt
```

AVX2 benchmarks require an AVX2-capable x86-64 processor. The project falls
back to portable implementations on other architectures.

## Build

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
```

This produces:

- `build/chaos_image_bench`: image benchmark and security-analysis suite
- `build/chaos_ref`: focused scalar/SSE2/AVX2 kernel benchmark

## Quick Start

Run the full suite on the included sample image:

```bash
./build/chaos_image_bench \
  --input images/sample.png \
  --results results/quickstart \
  --mode all
```

Run only the replaceable-stage benchmarks:

```bash
./build/chaos_image_bench \
  --input images/sample.png \
  --results results/stages \
  --mode stages
```

Run the candidate pipelines:

```bash
./build/chaos_image_bench \
  --input images/sample.png \
  --results results/candidates \
  --mode candidates
```

Useful options:

- `--fast-stages-only`: skip intentionally slow reference stages on large
  images
- `--fast-candidates-only`: skip slower candidate stress cases
- `--no-avx2`: disable AVX2 execution

Each run writes benchmark CSVs, security diagnostics, encrypted images, and
histograms under the selected results directory.

## Implemented Experiments

### Full schemes

| Category | Implementations |
| --- | --- |
| Chaotic-map baselines | `logistic_xor`, `logistic_permute_xor`, `arnold_xor`, `tiled_arnold_xor`, `tent_block_xor`, `sine_xor`, `coupled_lattice_xor`, `hamiltonian_lattice_xor` |
| Cryptographic keystream | `chaotic_seed_blake3_xor` using the official BLAKE3 C implementation |
| Standard baselines | OpenSSL EVP AES-256-CTR and ChaCha20 |

### Replaceable stages

| Stage | Implementations |
| --- | --- |
| Keystream | Logistic double, sine map, fixed-point tent, coupled-map lattice, cellular automata, Hamiltonian lattice, reaction-diffusion |
| Permutation | Chaotic sort, random walk, Arnold/cat map, affine modular, Feistel index, block permutation, checkerboard swaps |
| Diffusion | Global chain, block-local chain, prefix scan, stencil, ARX block, bit-plane, AVX2 multi-lane, AVX2 tree |

The regular block/checkerboard permutations and candidate diffusion stages are
performance templates. They are included to isolate implementation costs, not
to claim cryptographic security.

## Reproduce the Paper Experiments

The publication harness generates deterministic synthetic images, optionally
downloads the public Kodak PhotoCD dataset, runs repeated experiments, and
aggregates the results.

Standard run:

```bash
REPS=10 REAL_REPS=10 SIZES="512 1024 2048 4096" \
  ./scripts/run_paper_ready_experiments.sh
```

Include 8K stage and candidate measurements:

```bash
INCLUDE_8K=1 REPS=3 ./scripts/run_paper_ready_experiments.sh
```

Publication runs clean `results/final` by default and reject Lena/Lenna inputs.
Set `CLEAN_RESULTS=0` only when intentionally appending to an existing run.

Key outputs:

- `results/final/bench_stats.csv`: aggregated full-scheme results
- `results/final/stage_stats.csv`: aggregated replaceable-stage results
- `results/final/candidate_stats.csv`: aggregated candidate-pipeline results
- `results/final/analysis.csv`: security diagnostics
- `results/final/dataset_manifest.csv`: images included in the run
- `results/final/metadata.md`: benchmark configuration and environment

## Generate Publication Plots

```bash
python3 scripts/generate_publication_plots.py
```

The script reads the aggregated files in `results/final` and writes vector PDF
figures plus high-resolution PNG review copies to `paper/access/figures`.

## Build the Manuscript

The maintained IEEE Access LaTeX manuscript and generated review PDF are under
`paper/access`.

```bash
cd paper/access
pdflatex -interaction=nonstopmode -halt-on-error access_paper.tex
pdflatex -interaction=nonstopmode -halt-on-error access_paper.tex
```

## Focused SIMD Kernel Benchmark

`chaos_ref` compares the reference scalar, scan-form, SSE2, and AVX2-oriented
kernel paths:

```bash
./build/chaos_ref --bench --size 1024 --channels 3 --warmup 1 --iters 4 \
  --scheme all --kernel all --csv results/bench_ref.csv

./build/chaos_ref --stages --size 1024 --channels 3 --warmup 1 --iters 4 \
  --scheme map --kernel all --stages-csv results/bench_stages.csv

./build/chaos_ref --microbench --size 8192 --channels 3 --warmup 1 --iters 4 \
  --micro-csv results/bench_micro.csv
```

For a controlled compiler-level scalar/SSE2/AVX2 comparison:

```bash
WARMUP=1 ITERS=4 ./scripts/paper1_run_matrix.sh
```

The scalar build disables explicit SIMD intrinsics and compiler
auto-vectorization.

## Repository Layout

```text
include/chaosref/       Focused reference benchmark headers
src/core/               Shared image-cipher interfaces and utilities
src/scalar/             Scalar implementations
src/simd/avx2/          AVX2 implementations
src/crypto/             OpenSSL and BLAKE3 baselines
src/analysis/           Security metrics and reporting
scripts/                Dataset, benchmark, aggregation, and plotting tools
images/datasets/        Synthetic and public real-image datasets
results/final/          Aggregated publication results
paper/access/           IEEE Access manuscript and figures
archive/                Earlier reference implementation
```

## Interpreting the Results

The benchmark supports three main conclusions:

- Sort-based chaotic permutation is substantially more expensive than regular
  linear-time permutation.
- SIMD-friendly multi-lane and tree diffusion improve throughput by removing
  global byte-to-byte dependencies.
- Favorable image statistics do not establish cryptographic security. In
  particular, reused stream-cipher keystreams remain vulnerable even when
  entropy and correlation metrics look strong.

AES-256-CTR and ChaCha20 are included as performance and security reference
points. The project does not claim that the experimental pipelines replace
standard cryptography.
