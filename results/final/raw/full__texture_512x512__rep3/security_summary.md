# Security Summary

Input image: `images/datasets/synthetic/texture_512x512.ppm` (512x512x3)

| cipher | MB/s | entropy | NPCR | UACI | key sensitivity | KPA score | CPA score | correct |
|---|---:|---:|---:|---:|---:|---:|---:|---:|
| logistic_xor | 147.89 | 7.9998 | 0.00 | 0.00 | 99.38 | 1.0000 | 1.0000 | 1 |
| logistic_permute_xor | 15.57 | 7.9998 | 0.00 | 0.00 | 99.60 | 1.0000 | 0.0039 | 1 |
| arnold_xor | 121.84 | 7.9998 | 0.00 | 0.00 | 99.38 | 1.0000 | 0.0039 | 1 |
| tiled_arnold_xor | 94.88 | 7.9998 | 0.00 | 0.00 | 99.38 | 1.0000 | 0.0061 | 1 |
| tent_block_xor | 83.26 | 7.9998 | 0.00 | 0.00 | 99.63 | 1.0000 | 1.0000 | 1 |
| sine_xor | 25.24 | 7.9998 | 0.00 | 0.00 | 99.30 | 1.0000 | 1.0000 | 1 |
| coupled_lattice_xor | 80.50 | 7.9997 | 0.00 | 0.00 | 99.41 | 1.0000 | 1.0000 | 1 |
| hamiltonian_lattice_xor | 11.79 | 7.9998 | 0.00 | 0.00 | 99.53 | 1.0000 | 1.0000 | 1 |
| chaotic_seed_mix_xor | 433.45 | 7.9998 | 0.00 | 0.00 | 99.61 | 1.0000 | 1.0000 | 1 |
| aes_ctr | 262.98 | 7.9998 | 0.00 | 0.00 | 99.61 | 1.0000 | 1.0000 | 1 |
| chacha20 | 921.83 | 7.9998 | 0.00 | 0.00 | 99.62 | 1.0000 | 1.0000 | 1 |

KPA/CPA scores are empirical byte recovery ratios under deterministic key/nonce reuse. High values indicate a stream-XOR style vulnerability under reused keystream assumptions.

## Replaceable Stage Benchmarks

See `stage_bench.csv` for independent keystream, permutation, and diffusion primitive timings. See `candidate_schemes.csv` for the five SIMD-native candidate pipelines.

| candidate | keystream | permutation | diffusion | MB/s |
|---|---|---|---|---:|
