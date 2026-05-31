# Security Summary

Input image: `images/datasets/synthetic/noise_1024x1024.ppm` (1024x1024x3)

| cipher | MB/s | entropy | NPCR | UACI | key sensitivity | KPA score | CPA score | correct |
|---|---:|---:|---:|---:|---:|---:|---:|---:|
| logistic_xor | 131.12 | 7.9999 | 0.00 | 0.00 | 99.40 | 1.0000 | 1.0000 | 1 |
| logistic_permute_xor | 12.72 | 7.9999 | 0.00 | 0.00 | 99.61 | 1.0000 | 0.0039 | 1 |
| arnold_xor | 92.01 | 7.9999 | 0.00 | 0.00 | 99.40 | 1.0000 | 0.0039 | 1 |
| tiled_arnold_xor | 79.73 | 7.9999 | 0.00 | 0.00 | 99.40 | 1.0000 | 0.0049 | 1 |
| tent_block_xor | 81.34 | 7.9999 | 0.00 | 0.00 | 99.62 | 1.0000 | 1.0000 | 1 |
| sine_xor | 23.17 | 7.9999 | 0.00 | 0.00 | 99.30 | 1.0000 | 1.0000 | 1 |
| coupled_lattice_xor | 70.66 | 7.9999 | 0.00 | 0.00 | 99.41 | 1.0000 | 1.0000 | 1 |
| hamiltonian_lattice_xor | 11.47 | 7.9999 | 0.00 | 0.00 | 99.54 | 1.0000 | 1.0000 | 1 |
| chaotic_seed_mix_xor | 351.47 | 7.9999 | 0.00 | 0.00 | 99.61 | 1.0000 | 1.0000 | 1 |
| aes_ctr | 507.30 | 7.9999 | 0.00 | 0.00 | 99.62 | 1.0000 | 1.0000 | 1 |
| chacha20 | 719.66 | 7.9999 | 0.00 | 0.00 | 99.61 | 1.0000 | 1.0000 | 1 |

KPA/CPA scores are empirical byte recovery ratios under deterministic key/nonce reuse. High values indicate a stream-XOR style vulnerability under reused keystream assumptions.

## Replaceable Stage Benchmarks

See `stage_bench.csv` for independent keystream, permutation, and diffusion primitive timings. See `candidate_schemes.csv` for the five SIMD-native candidate pipelines.

| candidate | keystream | permutation | diffusion | MB/s |
|---|---|---|---|---:|
