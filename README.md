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

The suite includes scalar, AVX2, and portable ARM/NEON-compatible paths,
OpenSSL AES-256-CTR and ChaCha20 baselines, image-domain security diagnostics,
public dataset tooling, CSV output, publication plots, a finite-precision
orbit probe, and a manuscript now being reframed for the Journal of Real-Time
Image Processing.

> **Security notice:** The experimental chaotic and candidate pipelines are
> research prototypes. They have not received the cryptanalysis required for
> production use. Use a standardized authenticated-encryption scheme for real
> applications.

## Highlights

- Stage-level timing for keystream generation, permutation, diffusion, and
  total runtime
- Scalar, SSE2/AVX2-oriented, and portable comparison paths
- Full-scheme AES-256-CTR, ChaCha20, BLAKE3-XOR, and chaotic-map baselines
- Authenticated AES-256-GCM and ChaCha20-Poly1305 baselines with tamper tests
- SIMD-friendly permutation and diffusion candidates
- Entropy, chi-square, adjacent-pixel correlation, NPCR, UACI, key
  sensitivity, known-plaintext, chosen-plaintext, and finite-precision orbit
  diagnostics
- Synthetic and Kodak PhotoCD dataset support
- Public video-frame workload support for 720p/1080p real-time latency checks
- Reproducible CSV aggregation, confidence intervals, real-time metrics,
  plots, and manuscript

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

### Finite-precision probe

The publication harness now includes a dedicated orbit-length probe for the
chaotic map recurrence:

```bash
./build/chaos_ref --precision --warmup 500 --precision-samples 16 \
  --precision-max-steps 1000000 --precision-bits 16 \
  --precision-csv results/final/precision_orbits.csv
```

This probe records exact double-precision orbit bounds and a reduced-precision
stress test. The current manuscript reports the measured 16-bit quantized
collapse as a limitation check, not as a security proof.

## Implemented Experiments

### Full schemes

| Category | Implementations |
| --- | --- |
| Chaotic-map baselines | `logistic_xor`, `logistic_permute_xor`, `arnold_xor`, `tiled_arnold_xor`, `tent_block_xor`, `sine_xor`, `coupled_lattice_xor`, `hamiltonian_lattice_xor` |
| Cryptographic keystream | `chaotic_seed_blake3_xor` using the official BLAKE3 C implementation |
| Standard baselines | OpenSSL EVP AES-256-CTR, ChaCha20, AES-256-GCM, and ChaCha20-Poly1305 |

### Replaceable stages

| Stage | Implementations |
| --- | --- |
| Keystream | Logistic double, sine map, fixed-point tent, coupled-map lattice, cellular automata, Hamiltonian lattice, reaction-diffusion |
| Permutation | Chaotic sort, random walk, Arnold/cat map, affine modular, Feistel index, block permutation, checkerboard swaps |
| Diffusion | Global chain, block-local chain, prefix scan, stencil, ARX block, bit-plane, AVX2 multi-lane, AVX2 tree |

The regular block/checkerboard permutations and candidate diffusion stages are
performance templates. They are included to isolate implementation costs, not
to claim cryptographic security.

### Publication reference families

The `chaos_ref` executable is the correctness-first publication harness. It
contains four reversible family proxies:

- Fridrich-style modular-shear permutation and chain diffusion
- Chaotic sort permutation and chain diffusion
- Bit-plane transpose and chain diffusion
- Key-derived symbolic two-bit substitution and chain diffusion

For every family, `scalar_chain` and `scan_exact` produce byte-identical
ciphertext. Builds report `scalar`, `sse2`, `avx2`, or `neon` as appropriate.
These implementations are family proxies unless a manuscript explicitly
documents an exact reproduction of a cited algorithm.

## Reproduce the Paper Experiments

The publication harness generates deterministic synthetic images, optionally
downloads the public Kodak PhotoCD and video-frame datasets, runs repeated
experiments, and aggregates the results.

Standard run:

```bash
REPS=10 REAL_REPS=10 SIZES="512 1024 2048 4096" \
  ./scripts/run_paper_ready_experiments.sh
```

JRTIP real-time run with video frames:

```bash
CLEAN_RESULTS=1 RUN_SYNTHETIC=1 RUN_REAL=1 RUN_VIDEO=1 \
  DOWNLOAD_REAL=0 DOWNLOAD_VIDEO=0 REPS=3 REAL_REPS=3 \
  SIZES="512 1024 2048 4096" \
  ./scripts/run_paper_ready_experiments.sh
```

The current JRTIP result-data commit keeps CSV/Markdown outputs only:

- `results/jrtip_realtime/`: x86-64 run with synthetic, Kodak, and 24
  720p/1080p video frames
- `results/jrtip_realtime_arm/`: ARM64 run with the same workload
- `real_time_metrics.csv`: mean ms/frame, p95 ms/frame, FPS equivalent, and
  30/60 FPS pass/fail status

Encrypted output PNGs, histogram PNGs, generated review PNGs, and downloaded
datasets are local artifacts and are intentionally not versioned.

For the revised cross-architecture study, run the same command on the x86 and
ARM hosts with different output directories:

```bash
PLATFORM_ID=xeon_e5 OUT_DIR=results/cross_arch_x86 \
  ./scripts/run_cross_arch_publication.sh

PLATFORM_ID=graviton OUT_DIR=results/cross_arch_arm \
  ./scripts/run_cross_arch_publication.sh
```

The runner builds forced-scalar and native backends, runs CTest before the
matrix, discovers real images under `images/datasets/real`, randomizes case
order deterministically, and writes raw full/stage records plus paired
image-level bootstrap intervals. Use a small smoke run before a full run:

```bash
SIZES=64 REPS=1 WARMUP=0 ITERS=1 REAL_IMAGE_LIMIT=1 \
  OUT_DIR=/tmp/chaos-cross-smoke ./scripts/run_cross_arch_publication.sh
```

Statistical claims must use `paired_speedups.csv`; repetitions are averaged
within each image before images become the units of analysis.

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
Review PNGs are generated artifacts and should not be committed with result
data.

## Build the Manuscript

The maintained LaTeX manuscript and generated review PDF are under
`paper/access`. The current branch reframes the work for the Journal of
Real-Time Image Processing by emphasizing frame latency, video-frame
workloads, and x86/ARM reproducibility.

```bash
cd paper/access
pdflatex -interaction=nonstopmode -halt-on-error access_paper.tex
pdflatex -interaction=nonstopmode -halt-on-error access_paper.tex
```

The current submission bundle is staged around the paper source, figures, and
supporting evidence files rather than a journal portal integration. The
workspace also keeps a local archive at `/tmp/q2_submission_bundle.tar.gz`
for upload to an external submission system.

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
results/final/          Baseline aggregated publication results
results/jrtip_realtime/ x86 JRTIP real-time CSV/Markdown result data
results/jrtip_realtime_arm/ ARM JRTIP real-time CSV/Markdown result data
paper/access/           Manuscript and figures
archive/                Earlier reference implementation
```

## Interpreting the Results

The benchmark supports three main conclusions:

- Sort-based chaotic permutation is substantially more expensive than regular
  linear-time permutation.
- SIMD-friendly multi-lane and tree diffusion improve throughput by removing
  global byte-to-byte dependencies.
- Real-time feasibility should be reported as ms/frame, p95 latency, FPS
  equivalent, and 30/60 FPS pass/fail status on both x86-64 and ARM64.
- Favorable image statistics do not establish cryptographic security. In
  particular, reused stream-cipher keystreams remain vulnerable even when
  entropy and correlation metrics look strong.

AES-256-CTR and ChaCha20 are included as performance and security reference
points. The project does not claim that the experimental pipelines replace
standard cryptography.
