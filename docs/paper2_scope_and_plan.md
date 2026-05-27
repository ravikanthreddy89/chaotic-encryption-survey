# Paper 2 Scope (Follow-up)

## Working title
**Beyond Core Buckets: Symbolic Layers, Bit-Plane Methods, and Multicore/GPU Scaling in Chaotic Image Encryption**

## Why Paper 2
Paper 2 absorbs the parts that are too large for a rapid first submission:
- DNA/CA symbolic stages
- bit-plane cross permutation-diffusion families
- OpenMP scaling and GPU kernels
- larger cryptanalytic stress tests

## Planned additions
1. Bit-plane candidate with cross-plane diffusion.
2. Symbolic (DNA/CA) candidate with rule-switching.
3. Stage-level PMU profiling (cache misses, IPC, branch stats).
4. Multicore scaling curves and optional GPU baseline.

## Planned claims
1. Which stages are actually SIMD-limited vs memory-limited.
2. Where multicore parallelism helps beyond SIMD.
3. Cost of symbolic layers versus pure byte-level kernels.

## Dependency on Paper 1
Paper 2 reuses Paper 1 shared kernel framework and benchmark harness as baseline.
