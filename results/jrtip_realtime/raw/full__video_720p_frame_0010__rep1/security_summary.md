# Security Summary

Input image: `images/datasets/real/video_xiph_ducks/720p/frame_0010.ppm` (1280x720x3)

| cipher | MB/s | entropy | NPCR | UACI | key sensitivity | KPA score | CPA score | correct |
|---|---:|---:|---:|---:|---:|---:|---:|---:|
| logistic_xor | 117.73 | 7.9952 | 0.00 | 0.00 | 99.40 | 1.0000 | 1.0000 | 1 |
| logistic_permute_xor | 12.35 | 7.9952 | 0.00 | 0.00 | 99.60 | 1.0000 | 0.0065 | 1 |
| arnold_xor | 103.12 | 7.9953 | 0.00 | 0.00 | 99.40 | 1.0000 | 0.1032 | 1 |
| tiled_arnold_xor | 76.42 | 7.9953 | 0.00 | 0.00 | 99.40 | 1.0000 | 0.0250 | 1 |
| tent_block_xor | 75.93 | 7.9999 | 0.00 | 0.00 | 99.62 | 1.0000 | 1.0000 | 1 |
| sine_xor | 23.67 | 7.9980 | 0.00 | 0.00 | 99.31 | 1.0000 | 1.0000 | 1 |
| coupled_lattice_xor | 67.30 | 7.9984 | 0.00 | 0.00 | 99.42 | 1.0000 | 1.0000 | 1 |
| hamiltonian_lattice_xor | 11.17 | 7.9973 | 0.00 | 0.00 | 99.54 | 1.0000 | 1.0000 | 1 |
| chaotic_seed_blake3_xor | 279.12 | 7.9999 | 0.00 | 0.00 | 99.61 | 1.0000 | 1.0000 | 1 |
| aes_ctr | 517.75 | 7.9999 | 0.00 | 0.00 | 99.62 | 1.0000 | 1.0000 | 1 |
| chacha20 | 755.64 | 7.9999 | 0.00 | 0.00 | 99.61 | 1.0000 | 1.0000 | 1 |
| aes_256_gcm | 927.28 | 7.9999 | 0.00 | 0.00 | 99.61 | 1.0000 | 1.0000 | 1 |
| chacha20_poly1305 | 607.33 | 7.9999 | 0.00 | 0.00 | 99.61 | 1.0000 | 1.0000 | 1 |

KPA/CPA scores are empirical byte recovery ratios under deterministic key/nonce reuse. High values indicate a stream-XOR style vulnerability under reused keystream assumptions.

## Replaceable Stage Benchmarks

See `stage_bench.csv` for independent keystream, permutation, and diffusion primitive timings. See `candidate_schemes.csv` for the five SIMD-native candidate pipelines.

| candidate | keystream | permutation | diffusion | MB/s |
|---|---|---|---|---:|
