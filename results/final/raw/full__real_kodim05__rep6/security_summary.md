# Security Summary

Input image: `images/datasets/real/kodak/kodim05.png` (768x512x3)

| cipher | MB/s | entropy | NPCR | UACI | key sensitivity | KPA score | CPA score | correct |
|---|---:|---:|---:|---:|---:|---:|---:|---:|
| logistic_xor | 140.61 | 7.9953 | 0.00 | 0.00 | 99.39 | 1.0000 | 1.0000 | 1 |
| logistic_permute_xor | 13.60 | 7.9954 | 0.00 | 0.00 | 99.60 | 1.0000 | 0.0069 | 1 |
| arnold_xor | 116.25 | 7.9953 | 0.00 | 0.00 | 99.39 | 1.0000 | 0.1730 | 1 |
| tiled_arnold_xor | 89.83 | 7.9952 | 0.00 | 0.00 | 99.39 | 1.0000 | 0.0158 | 1 |
| tent_block_xor | 80.52 | 7.9998 | 0.00 | 0.00 | 99.62 | 1.0000 | 1.0000 | 1 |
| sine_xor | 23.22 | 7.9755 | 0.00 | 0.00 | 99.30 | 1.0000 | 1.0000 | 1 |
| coupled_lattice_xor | 72.09 | 7.9381 | 0.00 | 0.00 | 99.41 | 1.0000 | 1.0000 | 1 |
| hamiltonian_lattice_xor | 11.09 | 7.9986 | 0.00 | 0.00 | 99.53 | 1.0000 | 1.0000 | 1 |
| chaotic_seed_blake3_xor | 315.10 | 7.9998 | 0.00 | 0.00 | 99.61 | 1.0000 | 1.0000 | 1 |
| aes_ctr | 315.61 | 7.9998 | 0.00 | 0.00 | 99.61 | 1.0000 | 1.0000 | 1 |
| chacha20 | 830.90 | 7.9999 | 0.00 | 0.00 | 99.62 | 1.0000 | 1.0000 | 1 |

KPA/CPA scores are empirical byte recovery ratios under deterministic key/nonce reuse. High values indicate a stream-XOR style vulnerability under reused keystream assumptions.

## Replaceable Stage Benchmarks

See `stage_bench.csv` for independent keystream, permutation, and diffusion primitive timings. See `candidate_schemes.csv` for the five SIMD-native candidate pipelines.

| candidate | keystream | permutation | diffusion | MB/s |
|---|---|---|---|---:|
