# Security Summary

Input image: `images/datasets/real/kodak/kodim10.png` (512x768x3)

| cipher | MB/s | entropy | NPCR | UACI | key sensitivity | KPA score | CPA score | correct |
|---|---:|---:|---:|---:|---:|---:|---:|---:|
| logistic_xor | 138.09 | 7.9526 | 0.00 | 0.00 | 99.39 | 1.0000 | 1.0000 | 1 |
| logistic_permute_xor | 13.57 | 7.9529 | 0.00 | 0.00 | 99.57 | 1.0000 | 0.0088 | 1 |
| arnold_xor | 108.62 | 7.9529 | 0.00 | 0.00 | 99.39 | 1.0000 | 0.1752 | 1 |
| tiled_arnold_xor | 74.03 | 7.9530 | 0.00 | 0.00 | 99.39 | 1.0000 | 0.0480 | 1 |
| tent_block_xor | 82.23 | 7.9998 | 0.00 | 0.00 | 99.62 | 1.0000 | 1.0000 | 1 |
| sine_xor | 24.06 | 7.9857 | 0.00 | 0.00 | 99.30 | 1.0000 | 1.0000 | 1 |
| coupled_lattice_xor | 70.01 | 7.9878 | 0.00 | 0.00 | 99.41 | 1.0000 | 1.0000 | 1 |
| hamiltonian_lattice_xor | 11.29 | 7.9749 | 0.00 | 0.00 | 99.53 | 1.0000 | 1.0000 | 1 |
| chaotic_seed_blake3_xor | 326.25 | 7.9999 | 0.00 | 0.00 | 99.61 | 1.0000 | 1.0000 | 1 |
| aes_ctr | 351.75 | 7.9998 | 0.00 | 0.00 | 99.61 | 1.0000 | 1.0000 | 1 |
| chacha20 | 888.53 | 7.9998 | 0.00 | 0.00 | 99.62 | 1.0000 | 1.0000 | 1 |

KPA/CPA scores are empirical byte recovery ratios under deterministic key/nonce reuse. High values indicate a stream-XOR style vulnerability under reused keystream assumptions.

## Replaceable Stage Benchmarks

See `stage_bench.csv` for independent keystream, permutation, and diffusion primitive timings. See `candidate_schemes.csv` for the five SIMD-native candidate pipelines.

| candidate | keystream | permutation | diffusion | MB/s |
|---|---|---|---|---:|
