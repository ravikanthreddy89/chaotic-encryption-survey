# Security Summary

Input image: `images/datasets/real/kodak/kodim08.png` (768x512x3)

| cipher | MB/s | entropy | NPCR | UACI | key sensitivity | KPA score | CPA score | correct |
|---|---:|---:|---:|---:|---:|---:|---:|---:|
| logistic_xor | 126.14 | 7.9928 | 0.00 | 0.00 | 99.39 | 1.0000 | 1.0000 | 1 |
| logistic_permute_xor | 14.21 | 7.9926 | 0.00 | 0.00 | 99.61 | 1.0000 | 0.0063 | 1 |
| arnold_xor | 115.03 | 7.9928 | 0.00 | 0.00 | 99.39 | 1.0000 | 0.1726 | 1 |
| tiled_arnold_xor | 86.00 | 7.9927 | 0.00 | 0.00 | 99.39 | 1.0000 | 0.0218 | 1 |
| tent_block_xor | 77.06 | 7.9998 | 0.00 | 0.00 | 99.62 | 1.0000 | 1.0000 | 1 |
| sine_xor | 23.98 | 7.9877 | 0.00 | 0.00 | 99.30 | 1.0000 | 1.0000 | 1 |
| coupled_lattice_xor | 67.81 | 7.9871 | 0.00 | 0.00 | 99.41 | 1.0000 | 1.0000 | 1 |
| hamiltonian_lattice_xor | 11.00 | 7.9960 | 0.00 | 0.00 | 99.53 | 1.0000 | 1.0000 | 1 |
| chaotic_seed_blake3_xor | 199.44 | 7.9999 | 0.00 | 0.00 | 99.61 | 1.0000 | 1.0000 | 1 |
| aes_ctr | 353.89 | 7.9999 | 0.00 | 0.00 | 99.61 | 1.0000 | 1.0000 | 1 |
| chacha20 | 789.02 | 7.9998 | 0.00 | 0.00 | 99.62 | 1.0000 | 1.0000 | 1 |

KPA/CPA scores are empirical byte recovery ratios under deterministic key/nonce reuse. High values indicate a stream-XOR style vulnerability under reused keystream assumptions.

## Replaceable Stage Benchmarks

See `stage_bench.csv` for independent keystream, permutation, and diffusion primitive timings. See `candidate_schemes.csv` for the five SIMD-native candidate pipelines.

| candidate | keystream | permutation | diffusion | MB/s |
|---|---|---|---|---:|
