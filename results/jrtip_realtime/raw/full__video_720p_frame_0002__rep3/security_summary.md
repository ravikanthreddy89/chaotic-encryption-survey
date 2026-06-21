# Security Summary

Input image: `images/datasets/real/video_xiph_ducks/720p/frame_0002.ppm` (1280x720x3)

| cipher | MB/s | entropy | NPCR | UACI | key sensitivity | KPA score | CPA score | correct |
|---|---:|---:|---:|---:|---:|---:|---:|---:|
| logistic_xor | 124.12 | 7.9957 | 0.00 | 0.00 | 99.40 | 1.0000 | 1.0000 | 1 |
| logistic_permute_xor | 12.23 | 7.9957 | 0.00 | 0.00 | 99.60 | 1.0000 | 0.0067 | 1 |
| arnold_xor | 98.84 | 7.9958 | 0.00 | 0.00 | 99.40 | 1.0000 | 0.1032 | 1 |
| tiled_arnold_xor | 76.98 | 7.9957 | 0.00 | 0.00 | 99.40 | 1.0000 | 0.0247 | 1 |
| tent_block_xor | 75.69 | 7.9999 | 0.00 | 0.00 | 99.62 | 1.0000 | 1.0000 | 1 |
| sine_xor | 22.52 | 7.9982 | 0.00 | 0.00 | 99.31 | 1.0000 | 1.0000 | 1 |
| coupled_lattice_xor | 67.13 | 7.9986 | 0.00 | 0.00 | 99.42 | 1.0000 | 1.0000 | 1 |
| hamiltonian_lattice_xor | 10.18 | 7.9977 | 0.00 | 0.00 | 99.54 | 1.0000 | 1.0000 | 1 |
| chaotic_seed_blake3_xor | 224.05 | 7.9999 | 0.00 | 0.00 | 99.61 | 1.0000 | 1.0000 | 1 |
| aes_ctr | 495.51 | 7.9999 | 0.00 | 0.00 | 99.62 | 1.0000 | 1.0000 | 1 |
| chacha20 | 838.96 | 7.9999 | 0.00 | 0.00 | 99.61 | 1.0000 | 1.0000 | 1 |
| aes_256_gcm | 852.02 | 7.9999 | 0.00 | 0.00 | 99.61 | 1.0000 | 1.0000 | 1 |
| chacha20_poly1305 | 582.72 | 7.9999 | 0.00 | 0.00 | 99.61 | 1.0000 | 1.0000 | 1 |

KPA/CPA scores are empirical byte recovery ratios under deterministic key/nonce reuse. High values indicate a stream-XOR style vulnerability under reused keystream assumptions.

## Replaceable Stage Benchmarks

See `stage_bench.csv` for independent keystream, permutation, and diffusion primitive timings. See `candidate_schemes.csv` for the five SIMD-native candidate pipelines.

| candidate | keystream | permutation | diffusion | MB/s |
|---|---|---|---|---:|
