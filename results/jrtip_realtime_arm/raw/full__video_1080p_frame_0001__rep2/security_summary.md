# Security Summary

Input image: `images/datasets/real/video_xiph_ducks/1080p/frame_0001.ppm` (1920x1080x3)

| cipher | MB/s | entropy | NPCR | UACI | key sensitivity | KPA score | CPA score | correct |
|---|---:|---:|---:|---:|---:|---:|---:|---:|
| logistic_xor | 228.61 | 7.9956 | 0.00 | 0.00 | 99.41 | 1.0000 | 1.0000 | 1 |
| logistic_permute_xor | 14.39 | 7.9956 | 0.00 | 0.00 | 99.60 | 1.0000 | 0.0066 | 1 |
| arnold_xor | 156.69 | 7.9955 | 0.00 | 0.00 | 99.41 | 1.0000 | 0.1031 | 1 |
| tiled_arnold_xor | 136.88 | 7.9956 | 0.00 | 0.00 | 99.41 | 1.0000 | 0.0202 | 1 |
| tent_block_xor | 154.04 | 8.0000 | 0.00 | 0.00 | 99.61 | 1.0000 | 1.0000 | 1 |
| sine_xor | 33.94 | 7.9983 | 0.00 | 0.00 | 99.30 | 1.0000 | 1.0000 | 1 |
| coupled_lattice_xor | 98.18 | 7.9987 | 0.00 | 0.00 | 99.41 | 1.0000 | 1.0000 | 1 |
| hamiltonian_lattice_xor | 14.82 | 7.9976 | 0.00 | 0.00 | 99.54 | 1.0000 | 1.0000 | 1 |
| chaotic_seed_blake3_xor | 442.94 | 8.0000 | 0.00 | 0.00 | 99.61 | 1.0000 | 1.0000 | 1 |
| aes_ctr | 1163.70 | 8.0000 | 0.00 | 0.00 | 99.61 | 1.0000 | 1.0000 | 1 |
| chacha20 | 854.04 | 8.0000 | 0.00 | 0.00 | 99.61 | 1.0000 | 1.0000 | 1 |
| aes_256_gcm | 1474.75 | 8.0000 | 0.00 | 0.00 | 99.61 | 1.0000 | 1.0000 | 1 |
| chacha20_poly1305 | 626.26 | 8.0000 | 0.00 | 0.00 | 99.61 | 1.0000 | 1.0000 | 1 |

KPA/CPA scores are empirical byte recovery ratios under deterministic key/nonce reuse. High values indicate a stream-XOR style vulnerability under reused keystream assumptions.

## Replaceable Stage Benchmarks

See `stage_bench.csv` for independent keystream, permutation, and diffusion primitive timings. See `candidate_schemes.csv` for the five SIMD-native candidate pipelines.

| candidate | keystream | permutation | diffusion | MB/s |
|---|---|---|---|---:|
