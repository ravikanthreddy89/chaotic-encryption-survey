# Security Summary

Input image: `images/datasets/smoke/gradient_64x64.ppm` (64x64x3)

| cipher | MB/s | entropy | NPCR | UACI | key sensitivity | KPA score | CPA score | correct |
|---|---:|---:|---:|---:|---:|---:|---:|---:|
| logistic_xor | 101.04 | 7.9782 | 0.01 | 0.00 | 99.26 | 1.0000 | 1.0000 | 1 |
| logistic_permute_xor | 25.07 | 7.9752 | 0.01 | 0.00 | 99.62 | 0.9998 | 0.0068 | 1 |
| arnold_xor | 101.45 | 7.9790 | 0.01 | 0.00 | 99.26 | 1.0000 | 0.0059 | 1 |
| tiled_arnold_xor | 62.31 | 7.9792 | 0.01 | 0.00 | 99.26 | 1.0000 | 0.0120 | 1 |
| tent_block_xor | 90.68 | 7.9834 | 0.01 | 0.00 | 99.59 | 1.0000 | 1.0000 | 1 |
| sine_xor | 20.69 | 7.9599 | 0.01 | 0.00 | 99.34 | 1.0000 | 1.0000 | 1 |
| coupled_lattice_xor | 59.37 | 7.9355 | 0.01 | 0.00 | 99.59 | 1.0000 | 1.0000 | 1 |
| hamiltonian_lattice_xor | 11.44 | 7.9807 | 0.01 | 0.00 | 99.53 | 1.0000 | 1.0000 | 1 |
| chaotic_seed_mix_xor | 200.06 | 7.9855 | 0.01 | 0.00 | 99.63 | 1.0000 | 1.0000 | 1 |
| aes_ctr | 5.02 | 7.9852 | 0.01 | 0.00 | 99.59 | 1.0000 | 1.0000 | 1 |
| chacha20 | 524.21 | 7.9857 | 0.01 | 0.00 | 99.62 | 1.0000 | 1.0000 | 1 |

KPA/CPA scores are empirical byte recovery ratios under deterministic key/nonce reuse. High values indicate a stream-XOR style vulnerability under reused keystream assumptions.

## Replaceable Stage Benchmarks

See `stage_bench.csv` for independent keystream, permutation, and diffusion primitive timings. See `candidate_schemes.csv` for the five SIMD-native candidate pipelines.

| candidate | keystream | permutation | diffusion | MB/s |
|---|---|---|---|---:|
