# Security Summary

Input image: `images/datasets/real/kodak/kodim10.png` (512x768x3)

| cipher | MB/s | entropy | NPCR | UACI | key sensitivity | KPA score | CPA score | correct |
|---|---:|---:|---:|---:|---:|---:|---:|---:|
| logistic_xor | 130.23 | 7.9526 | 0.00 | 0.00 | 99.39 | 1.0000 | 1.0000 | 1 |
| logistic_permute_xor | 14.18 | 7.9529 | 0.00 | 0.00 | 99.57 | 1.0000 | 0.0088 | 1 |
| arnold_xor | 107.25 | 7.9529 | 0.00 | 0.00 | 99.39 | 1.0000 | 0.1752 | 1 |
| tiled_arnold_xor | 87.53 | 7.9530 | 0.00 | 0.00 | 99.39 | 1.0000 | 0.0480 | 1 |
| tent_block_xor | 73.37 | 7.9998 | 0.00 | 0.00 | 99.62 | 1.0000 | 1.0000 | 1 |
| sine_xor | 22.40 | 7.9857 | 0.00 | 0.00 | 99.30 | 1.0000 | 1.0000 | 1 |
| coupled_lattice_xor | 67.34 | 7.9878 | 0.00 | 0.00 | 99.41 | 1.0000 | 1.0000 | 1 |
| hamiltonian_lattice_xor | 10.38 | 7.9749 | 0.00 | 0.00 | 99.53 | 1.0000 | 1.0000 | 1 |
| chaotic_seed_blake3_xor | 294.84 | 7.9999 | 0.00 | 0.00 | 99.61 | 1.0000 | 1.0000 | 1 |
| aes_ctr | 332.84 | 7.9998 | 0.00 | 0.00 | 99.61 | 1.0000 | 1.0000 | 1 |
| chacha20 | 872.02 | 7.9998 | 0.00 | 0.00 | 99.62 | 1.0000 | 1.0000 | 1 |
| aes_256_gcm | 949.62 | 7.9998 | 0.00 | 0.00 | 99.60 | 1.0000 | 1.0000 | 1 |
| chacha20_poly1305 | 636.86 | 7.9998 | 0.00 | 0.00 | 99.61 | 1.0000 | 1.0000 | 1 |

KPA/CPA scores are empirical byte recovery ratios under deterministic key/nonce reuse. High values indicate a stream-XOR style vulnerability under reused keystream assumptions.

## Replaceable Stage Benchmarks

See `stage_bench.csv` for independent keystream, permutation, and diffusion primitive timings. See `candidate_schemes.csv` for the five SIMD-native candidate pipelines.

| candidate | keystream | permutation | diffusion | MB/s |
|---|---|---|---|---:|
