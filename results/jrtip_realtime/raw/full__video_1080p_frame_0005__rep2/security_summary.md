# Security Summary

Input image: `images/datasets/real/video_xiph_ducks/1080p/frame_0005.ppm` (1920x1080x3)

| cipher | MB/s | entropy | NPCR | UACI | key sensitivity | KPA score | CPA score | correct |
|---|---:|---:|---:|---:|---:|---:|---:|---:|
| logistic_xor | 120.14 | 7.9953 | 0.00 | 0.00 | 99.41 | 1.0000 | 1.0000 | 1 |
| logistic_permute_xor | 9.13 | 7.9953 | 0.00 | 0.00 | 99.60 | 1.0000 | 0.0067 | 1 |
| arnold_xor | 90.18 | 7.9953 | 0.00 | 0.00 | 99.41 | 1.0000 | 0.1032 | 1 |
| tiled_arnold_xor | 73.62 | 7.9953 | 0.00 | 0.00 | 99.41 | 1.0000 | 0.0203 | 1 |
| tent_block_xor | 68.97 | 8.0000 | 0.00 | 0.00 | 99.61 | 1.0000 | 1.0000 | 1 |
| sine_xor | 23.15 | 7.9980 | 0.00 | 0.00 | 99.30 | 1.0000 | 1.0000 | 1 |
| coupled_lattice_xor | 66.42 | 7.9985 | 0.00 | 0.00 | 99.41 | 1.0000 | 1.0000 | 1 |
| hamiltonian_lattice_xor | 10.35 | 7.9975 | 0.00 | 0.00 | 99.54 | 1.0000 | 1.0000 | 1 |
| chaotic_seed_blake3_xor | 271.57 | 8.0000 | 0.00 | 0.00 | 99.61 | 1.0000 | 1.0000 | 1 |
| aes_ctr | 602.00 | 8.0000 | 0.00 | 0.00 | 99.61 | 1.0000 | 1.0000 | 1 |
| chacha20 | 712.44 | 8.0000 | 0.00 | 0.00 | 99.61 | 1.0000 | 1.0000 | 1 |
| aes_256_gcm | 805.79 | 8.0000 | 0.00 | 0.00 | 99.61 | 1.0000 | 1.0000 | 1 |
| chacha20_poly1305 | 588.05 | 8.0000 | 0.00 | 0.00 | 99.61 | 1.0000 | 1.0000 | 1 |

KPA/CPA scores are empirical byte recovery ratios under deterministic key/nonce reuse. High values indicate a stream-XOR style vulnerability under reused keystream assumptions.

## Replaceable Stage Benchmarks

See `stage_bench.csv` for independent keystream, permutation, and diffusion primitive timings. See `candidate_schemes.csv` for the five SIMD-native candidate pipelines.

| candidate | keystream | permutation | diffusion | MB/s |
|---|---|---|---|---:|
