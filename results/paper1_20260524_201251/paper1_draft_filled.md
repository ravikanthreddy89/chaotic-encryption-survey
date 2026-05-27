# Reference SIMD/Vectorized Implementations of Shared Chaotic Image Encryption Pipelines

## Abstract
Chaos-based image encryption papers frequently reuse a permutation-diffusion pipeline but report implementations that are difficult to reproduce and rarely optimized for modern CPUs. This work presents a reference implementation that factors common pipeline components into reusable kernels and benchmarks scalar versus vectorized variants. We implement two representative candidate families: (i) map-based permutation with chained diffusion (Fridrich-style bucket), and (ii) sort-based permutation with chained diffusion (YeHuang/LSCM-like bucket). For diffusion, we provide both a scalar recurrence and an exact scan-form implementation suitable for SIMD acceleration. Results show that diffusion vectorization provides measurable speedups, while sort-based permutation remains the dominant bottleneck. The released code and scripts provide a reproducible baseline for future chaotic-image-encryption performance research.

## 1. Introduction
Chaotic image encryption literature is broad but fragmented. Many schemes differ in chaotic maps and symbolic operations, yet share the same pipeline skeleton: key-to-parameter mapping, chaotic sequence generation, permutation, and diffusion. The absence of optimized reference code makes fair performance comparison difficult.

This paper focuses on practical progress rather than exhaustive algorithm coverage. We isolate shared buckets and quantify how much speedup vectorization offers in those buckets.

## 2. Scope and Threats to Validity
- This paper is a performance and reproducibility study, not a full cryptanalytic proof.
- We do not claim coverage of every published chaotic scheme.
- Security metrics and advanced attacks are deferred to follow-up work.

## 3. Shared Pipeline Buckets
1. Map-based permutation bucket.
2. Sort-based permutation bucket.
3. Diffusion bucket: scalar chain vs exact scan-form.
4. Deterministic chaotic keystream generation.

## 4. Implemented Candidates
1. **Candidate A (Map+Chain)**: Fridrich-style map permutation + chained XOR diffusion.
2. **Candidate B (Sort+Chain)**: chaotic-score sort permutation + chained XOR diffusion.

Each candidate is tested under two diffusion kernels:
- `scalar_chain`
- `scan_exact` (vectorization-friendly)

Each is compiled under two builds:
- `scalar_build` (`ENABLE_AVX2=OFF`)
- `simd_build` (`ENABLE_AVX2=ON`, x86 targets)

## 5. Experimental Setup
Use:
```bash
SIZES="512 1024 2048" WARMUP=1 ITERS=4 ./scripts/paper1_run_matrix.sh
```
Then generate tables:
```bash
python3 scripts/paper1_make_tables.py results/<run_dir>/paper1_combined.csv
```

Report:
- CPU model and ISA support
- Compiler and flags
- OS and thread settings

## 6. Results

# Paper 1 Tables

Largest-image subset bytes=196608

## SIMD Build Speedup (largest image)

| case | scalar enc MB/s | simd enc MB/s | enc speedup | scalar dec MB/s | simd dec MB/s | dec speedup |
|---|---:|---:|---:|---:|---:|---:|
| map_fridrich+scalar_chain | 80.6 | 113.2 | 1.40x | 88.7 | 123.1 | 1.39x |
| map_fridrich+scan_exact | 113.3 | 130.8 | 1.16x | 105.2 | 134.6 | 1.28x |
| sort_logistic+scalar_chain | 25.5 | 28.4 | 1.11x | 26.9 | 30.3 | 1.13x |
| sort_logistic+scan_exact | 31.7 | 33.2 | 1.05x | 32.7 | 32.2 | 0.98x |

## Full Results

| build_mode | case | simd_backend | shape | enc MB/s | dec MB/s | correct |
|---|---|---|---|---:|---:|---:|
| scalar | map_fridrich+scalar_chain | scalar | 256x256x3 | 80.6 | 88.7 | 1 |
| scalar | map_fridrich+scan_exact | scalar | 256x256x3 | 113.3 | 105.2 | 1 |
| scalar | sort_logistic+scalar_chain | scalar | 256x256x3 | 25.5 | 26.9 | 1 |
| scalar | sort_logistic+scan_exact | scalar | 256x256x3 | 31.7 | 32.7 | 1 |
| simd | map_fridrich+scalar_chain | scalar | 256x256x3 | 113.2 | 123.1 | 1 |
| simd | map_fridrich+scan_exact | scalar | 256x256x3 | 130.8 | 134.6 | 1 |
| simd | sort_logistic+scalar_chain | scalar | 256x256x3 | 28.4 | 30.3 | 1 |
| simd | sort_logistic+scan_exact | scalar | 256x256x3 | 33.2 | 32.2 | 1 |


Paste generated tables here.

### 6.1 SIMD Build Speedup (largest image)
<!-- paste output block from paper1_make_tables.py -->

### 6.2 Full Result Matrix
<!-- paste output block from paper1_make_tables.py -->

## 7. Discussion
Expected interpretation pattern:
1. Diffusion acceleration is visible in both candidates.
2. Map-based candidate benefits more from diffusion optimization because permutation is lighter.
3. Sort-based candidate remains limited by permutation sort (`O(N log N)` memory-heavy stage).

## 8. Reproducibility
Code structure:
- `include/chaosref/permutation.hpp`
- `include/chaosref/diffusion.hpp`
- `include/chaosref/schemes.hpp`
- `include/chaosref/bench.hpp`
- `src/main.cpp`
- `scripts/paper1_run_matrix.sh`
- `scripts/paper1_make_tables.py`

## 9. Conclusion
A bucketed reference implementation enables transparent, reproducible comparison of chaotic-image-encryption performance. SIMD/vectorization improves diffusion throughput, but permutation design largely determines total runtime in sort-based families. This baseline supports broader follow-up studies (bit-plane/symbolic layers, multicore/GPU scaling, and deeper security evaluation).
