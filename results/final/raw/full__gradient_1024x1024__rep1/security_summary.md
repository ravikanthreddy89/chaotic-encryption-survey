# Security Summary

Input image: `images/datasets/synthetic/gradient_1024x1024.ppm` (1024x1024x3)

| cipher | MB/s | entropy | NPCR | UACI | key sensitivity | KPA score | CPA score | correct |
|---|---:|---:|---:|---:|---:|---:|---:|---:|
| logistic_xor | 134.26 | 7.9999 | 0.00 | 0.00 | 99.40 | 1.0000 | 1.0000 | 1 |
| logistic_permute_xor | 11.75 | 7.9999 | 0.00 | 0.00 | 99.61 | 1.0000 | 0.0040 | 1 |
| arnold_xor | 100.14 | 7.9999 | 0.00 | 0.00 | 99.40 | 1.0000 | 0.0039 | 1 |
| tiled_arnold_xor | 77.62 | 7.9999 | 0.00 | 0.00 | 99.40 | 1.0000 | 0.0120 | 1 |
| tent_block_xor | 78.54 | 7.9999 | 0.00 | 0.00 | 99.62 | 1.0000 | 1.0000 | 1 |
| sine_xor | 24.41 | 7.9999 | 0.00 | 0.00 | 99.30 | 1.0000 | 1.0000 | 1 |
| coupled_lattice_xor | 66.87 | 7.9999 | 0.00 | 0.00 | 99.41 | 1.0000 | 1.0000 | 1 |
| hamiltonian_lattice_xor | 11.20 | 7.9999 | 0.00 | 0.00 | 99.54 | 1.0000 | 1.0000 | 1 |
| chaotic_seed_mix_xor | 312.98 | 7.9999 | 0.00 | 0.00 | 99.61 | 1.0000 | 1.0000 | 1 |
| aes_ctr | 578.25 | 7.9999 | 0.00 | 0.00 | 99.62 | 1.0000 | 1.0000 | 1 |
| chacha20 | 792.66 | 7.9999 | 0.00 | 0.00 | 99.61 | 1.0000 | 1.0000 | 1 |

KPA/CPA scores are empirical byte recovery ratios under deterministic key/nonce reuse. High values indicate a stream-XOR style vulnerability under reused keystream assumptions.

## Replaceable Stage Benchmarks

See `stage_bench.csv` for independent keystream, permutation, and diffusion primitive timings. See `candidate_schemes.csv` for the five SIMD-native candidate pipelines.

| candidate | keystream | permutation | diffusion | MB/s |
|---|---|---|---|---:|
