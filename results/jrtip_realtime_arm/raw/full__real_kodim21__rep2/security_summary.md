# Security Summary

Input image: `images/datasets/real/kodak/kodim21.png` (768x512x3)

| cipher | MB/s | entropy | NPCR | UACI | key sensitivity | KPA score | CPA score | correct |
|---|---:|---:|---:|---:|---:|---:|---:|---:|
| logistic_xor | 184.14 | 7.9674 | 0.00 | 0.00 | 99.39 | 1.0000 | 1.0000 | 1 |
| logistic_permute_xor | 18.68 | 7.9672 | 0.00 | 0.00 | 99.59 | 1.0000 | 0.0115 | 1 |
| arnold_xor | 153.01 | 7.9672 | 0.00 | 0.00 | 99.39 | 1.0000 | 0.1775 | 1 |
| tiled_arnold_xor | 135.96 | 7.9671 | 0.00 | 0.00 | 99.39 | 1.0000 | 0.0590 | 1 |
| tent_block_xor | 150.98 | 7.9998 | 0.00 | 0.00 | 99.62 | 1.0000 | 1.0000 | 1 |
| sine_xor | 33.72 | 7.9830 | 0.00 | 0.00 | 99.30 | 1.0000 | 1.0000 | 1 |
| coupled_lattice_xor | 95.37 | 7.9937 | 0.00 | 0.00 | 99.41 | 1.0000 | 1.0000 | 1 |
| hamiltonian_lattice_xor | 14.78 | 7.9798 | 0.00 | 0.00 | 99.53 | 1.0000 | 1.0000 | 1 |
| chaotic_seed_blake3_xor | 407.48 | 7.9999 | 0.00 | 0.00 | 99.61 | 1.0000 | 1.0000 | 1 |
| aes_ctr | 456.43 | 7.9998 | 0.00 | 0.00 | 99.61 | 1.0000 | 1.0000 | 1 |
| chacha20 | 767.18 | 7.9998 | 0.00 | 0.00 | 99.62 | 1.0000 | 1.0000 | 1 |
| aes_256_gcm | 1232.01 | 7.9998 | 0.00 | 0.00 | 99.60 | 1.0000 | 1.0000 | 1 |
| chacha20_poly1305 | 580.37 | 7.9998 | 0.00 | 0.00 | 99.61 | 1.0000 | 1.0000 | 1 |

KPA/CPA scores are empirical byte recovery ratios under deterministic key/nonce reuse. High values indicate a stream-XOR style vulnerability under reused keystream assumptions.

## Replaceable Stage Benchmarks

See `stage_bench.csv` for independent keystream, permutation, and diffusion primitive timings. See `candidate_schemes.csv` for the five SIMD-native candidate pipelines.

| candidate | keystream | permutation | diffusion | MB/s |
|---|---|---|---|---:|
