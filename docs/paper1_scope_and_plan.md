# Paper 1 Scope (Submit First)

## Working title
**Reference SIMD/Vectorized Implementations of Shared Chaotic Image Encryption Pipelines**

## Honest scope
This paper does **not** claim to optimize every published chaotic-image cipher. It focuses on the common computational buckets that recur across many papers, then measures acceleration from vectorization.

## Candidate algorithms to include in Paper 1
1. **Map-permutation + chain-diffusion** (Fridrich-style bucket)
2. **Sort-permutation + chain-diffusion** (YeHuang/LSCM-like bucket)

For each candidate:
- scalar baseline
- scan-exact vectorizable diffusion kernel
- SIMD build ON/OFF comparison

## Claims we can safely make
1. A shared-kernel implementation can represent major chaotic pipeline families.
2. Diffusion recurrence can be rewritten exactly into scan-form and accelerated.
3. Sort-based candidates remain bottlenecked by permutation sort despite diffusion optimization.

## Required results before submission
1. Run `scripts/paper1_run_matrix.sh` on target x86 machine.
2. Generate tables via `scripts/paper1_make_tables.py`.
3. Include correctness column (`correct=1`) for all experiments.

## Experimental protocol (minimum)
- Sizes: 512, 1024, 2048
- Channels: 3
- Warmup: 1
- Iterations: 4 (increase if time allows)
- CPU pinning if possible
- Compiler flags and host metadata reported

## Current implementation status
- Implemented in `include/chaosref/*` and `src/main.cpp`.
- Benchmark CLI available via `--bench`.
