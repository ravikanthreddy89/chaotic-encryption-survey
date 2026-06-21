# Security Summary

Input image: `images/datasets/real/video_xiph_ducks/1080p/frame_0007.ppm` (1920x1080x3)

| cipher | MB/s | entropy | NPCR | UACI | key sensitivity | KPA score | CPA score | correct |
|---|---:|---:|---:|---:|---:|---:|---:|---:|
| logistic_xor | 227.06 | 7.9953 | 0.00 | 0.00 | 99.41 | 1.0000 | 1.0000 | 1 |
| logistic_permute_xor | 14.15 | 7.9953 | 0.00 | 0.00 | 99.61 | 1.0000 | 0.0067 | 1 |
| arnold_xor | 155.86 | 7.9953 | 0.00 | 0.00 | 99.41 | 1.0000 | 0.1032 | 1 |
| tiled_arnold_xor | 137.21 | 7.9953 | 0.00 | 0.00 | 99.41 | 1.0000 | 0.0203 | 1 |
| tent_block_xor | 152.23 | 8.0000 | 0.00 | 0.00 | 99.61 | 1.0000 | 1.0000 | 1 |
| sine_xor | 33.85 | 7.9978 | 0.00 | 0.00 | 99.30 | 1.0000 | 1.0000 | 1 |
| coupled_lattice_xor | 97.16 | 7.9984 | 0.00 | 0.00 | 99.41 | 1.0000 | 1.0000 | 1 |
| hamiltonian_lattice_xor | 14.80 | 7.9975 | 0.00 | 0.00 | 99.54 | 1.0000 | 1.0000 | 1 |
| chaotic_seed_blake3_xor | 421.79 | 8.0000 | 0.00 | 0.00 | 99.61 | 1.0000 | 1.0000 | 1 |
| aes_ctr | 1053.14 | 8.0000 | 0.00 | 0.00 | 99.61 | 1.0000 | 1.0000 | 1 |
| chacha20 | 808.96 | 8.0000 | 0.00 | 0.00 | 99.61 | 1.0000 | 1.0000 | 1 |
| aes_256_gcm | 1466.52 | 8.0000 | 0.00 | 0.00 | 99.61 | 1.0000 | 1.0000 | 1 |
| chacha20_poly1305 | 622.49 | 8.0000 | 0.00 | 0.00 | 99.61 | 1.0000 | 1.0000 | 1 |

KPA/CPA scores are empirical byte recovery ratios under deterministic key/nonce reuse. High values indicate a stream-XOR style vulnerability under reused keystream assumptions.

## Replaceable Stage Benchmarks

See `stage_bench.csv` for independent keystream, permutation, and diffusion primitive timings. See `candidate_schemes.csv` for the five SIMD-native candidate pipelines.

| candidate | keystream | permutation | diffusion | MB/s |
|---|---|---|---|---:|
