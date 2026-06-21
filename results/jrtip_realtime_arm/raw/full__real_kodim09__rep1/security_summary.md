# Security Summary

Input image: `images/datasets/real/kodak/kodim09.png` (512x768x3)

| cipher | MB/s | entropy | NPCR | UACI | key sensitivity | KPA score | CPA score | correct |
|---|---:|---:|---:|---:|---:|---:|---:|---:|
| logistic_xor | 180.63 | 7.9528 | 0.00 | 0.00 | 99.39 | 1.0000 | 1.0000 | 1 |
| logistic_permute_xor | 18.32 | 7.9527 | 0.00 | 0.00 | 99.58 | 1.0000 | 0.0092 | 1 |
| arnold_xor | 155.03 | 7.9529 | 0.00 | 0.00 | 99.39 | 1.0000 | 0.1741 | 1 |
| tiled_arnold_xor | 138.89 | 7.9529 | 0.00 | 0.00 | 99.39 | 1.0000 | 0.0598 | 1 |
| tent_block_xor | 153.50 | 7.9998 | 0.00 | 0.00 | 99.62 | 1.0000 | 1.0000 | 1 |
| sine_xor | 33.73 | 7.9801 | 0.00 | 0.00 | 99.30 | 1.0000 | 1.0000 | 1 |
| coupled_lattice_xor | 96.67 | 7.9880 | 0.00 | 0.00 | 99.41 | 1.0000 | 1.0000 | 1 |
| hamiltonian_lattice_xor | 14.76 | 7.9721 | 0.00 | 0.00 | 99.53 | 1.0000 | 1.0000 | 1 |
| chaotic_seed_blake3_xor | 406.00 | 7.9999 | 0.00 | 0.00 | 99.61 | 1.0000 | 1.0000 | 1 |
| aes_ctr | 457.94 | 7.9998 | 0.00 | 0.00 | 99.61 | 1.0000 | 1.0000 | 1 |
| chacha20 | 780.10 | 7.9998 | 0.00 | 0.00 | 99.62 | 1.0000 | 1.0000 | 1 |
| aes_256_gcm | 1244.21 | 7.9998 | 0.00 | 0.00 | 99.60 | 1.0000 | 1.0000 | 1 |
| chacha20_poly1305 | 570.47 | 7.9998 | 0.00 | 0.00 | 99.61 | 1.0000 | 1.0000 | 1 |

KPA/CPA scores are empirical byte recovery ratios under deterministic key/nonce reuse. High values indicate a stream-XOR style vulnerability under reused keystream assumptions.

## Replaceable Stage Benchmarks

See `stage_bench.csv` for independent keystream, permutation, and diffusion primitive timings. See `candidate_schemes.csv` for the five SIMD-native candidate pipelines.

| candidate | keystream | permutation | diffusion | MB/s |
|---|---|---|---|---:|
