# Security Summary

Input image: `images/datasets/real/video_xiph_ducks/720p/frame_0010.ppm` (1280x720x3)

| cipher | MB/s | entropy | NPCR | UACI | key sensitivity | KPA score | CPA score | correct |
|---|---:|---:|---:|---:|---:|---:|---:|---:|
| logistic_xor | 202.30 | 7.9952 | 0.00 | 0.00 | 99.40 | 1.0000 | 1.0000 | 1 |
| logistic_permute_xor | 16.08 | 7.9952 | 0.00 | 0.00 | 99.60 | 1.0000 | 0.0065 | 1 |
| arnold_xor | 155.67 | 7.9953 | 0.00 | 0.00 | 99.40 | 1.0000 | 0.1032 | 1 |
| tiled_arnold_xor | 131.12 | 7.9953 | 0.00 | 0.00 | 99.40 | 1.0000 | 0.0250 | 1 |
| tent_block_xor | 151.61 | 7.9999 | 0.00 | 0.00 | 99.62 | 1.0000 | 1.0000 | 1 |
| sine_xor | 33.78 | 7.9980 | 0.00 | 0.00 | 99.31 | 1.0000 | 1.0000 | 1 |
| coupled_lattice_xor | 96.91 | 7.9984 | 0.00 | 0.00 | 99.42 | 1.0000 | 1.0000 | 1 |
| hamiltonian_lattice_xor | 14.80 | 7.9973 | 0.00 | 0.00 | 99.54 | 1.0000 | 1.0000 | 1 |
| chaotic_seed_blake3_xor | 419.58 | 7.9999 | 0.00 | 0.00 | 99.61 | 1.0000 | 1.0000 | 1 |
| aes_ctr | 762.36 | 7.9999 | 0.00 | 0.00 | 99.62 | 1.0000 | 1.0000 | 1 |
| chacha20 | 838.05 | 7.9999 | 0.00 | 0.00 | 99.61 | 1.0000 | 1.0000 | 1 |
| aes_256_gcm | 1451.29 | 7.9999 | 0.00 | 0.00 | 99.61 | 1.0000 | 1.0000 | 1 |
| chacha20_poly1305 | 627.89 | 7.9999 | 0.00 | 0.00 | 99.61 | 1.0000 | 1.0000 | 1 |

KPA/CPA scores are empirical byte recovery ratios under deterministic key/nonce reuse. High values indicate a stream-XOR style vulnerability under reused keystream assumptions.

## Replaceable Stage Benchmarks

See `stage_bench.csv` for independent keystream, permutation, and diffusion primitive timings. See `candidate_schemes.csv` for the five SIMD-native candidate pipelines.

| candidate | keystream | permutation | diffusion | MB/s |
|---|---|---|---|---:|
