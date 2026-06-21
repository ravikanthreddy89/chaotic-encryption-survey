# Security Summary

Input image: `images/datasets/real/kodak/kodim24.png` (768x512x3)

| cipher | MB/s | entropy | NPCR | UACI | key sensitivity | KPA score | CPA score | correct |
|---|---:|---:|---:|---:|---:|---:|---:|---:|
| logistic_xor | 185.17 | 7.9852 | 0.00 | 0.00 | 99.39 | 1.0000 | 1.0000 | 1 |
| logistic_permute_xor | 18.61 | 7.9852 | 0.00 | 0.00 | 99.59 | 1.0000 | 0.0110 | 1 |
| arnold_xor | 152.07 | 7.9851 | 0.00 | 0.00 | 99.39 | 1.0000 | 0.1802 | 1 |
| tiled_arnold_xor | 135.86 | 7.9851 | 0.00 | 0.00 | 99.39 | 1.0000 | 0.0638 | 1 |
| tent_block_xor | 153.54 | 7.9998 | 0.00 | 0.00 | 99.62 | 1.0000 | 1.0000 | 1 |
| sine_xor | 33.18 | 7.9732 | 0.00 | 0.00 | 99.30 | 1.0000 | 1.0000 | 1 |
| coupled_lattice_xor | 96.79 | 7.9624 | 0.00 | 0.00 | 99.41 | 1.0000 | 1.0000 | 1 |
| hamiltonian_lattice_xor | 14.64 | 7.9925 | 0.00 | 0.00 | 99.53 | 1.0000 | 1.0000 | 1 |
| chaotic_seed_blake3_xor | 406.62 | 7.9998 | 0.00 | 0.00 | 99.61 | 1.0000 | 1.0000 | 1 |
| aes_ctr | 444.19 | 7.9999 | 0.00 | 0.00 | 99.61 | 1.0000 | 1.0000 | 1 |
| chacha20 | 753.04 | 7.9999 | 0.00 | 0.00 | 99.62 | 1.0000 | 1.0000 | 1 |
| aes_256_gcm | 1190.97 | 7.9999 | 0.00 | 0.00 | 99.60 | 1.0000 | 1.0000 | 1 |
| chacha20_poly1305 | 578.02 | 7.9998 | 0.00 | 0.00 | 99.61 | 1.0000 | 1.0000 | 1 |

KPA/CPA scores are empirical byte recovery ratios under deterministic key/nonce reuse. High values indicate a stream-XOR style vulnerability under reused keystream assumptions.

## Replaceable Stage Benchmarks

See `stage_bench.csv` for independent keystream, permutation, and diffusion primitive timings. See `candidate_schemes.csv` for the five SIMD-native candidate pipelines.

| candidate | keystream | permutation | diffusion | MB/s |
|---|---|---|---|---:|
