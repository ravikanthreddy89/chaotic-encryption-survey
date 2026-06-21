# Security Summary

Input image: `images/datasets/real/video_xiph_ducks/1080p/frame_0001.ppm` (1920x1080x3)

| cipher | MB/s | entropy | NPCR | UACI | key sensitivity | KPA score | CPA score | correct |
|---|---:|---:|---:|---:|---:|---:|---:|---:|
| logistic_xor | 118.18 | 7.9956 | 0.00 | 0.00 | 99.41 | 1.0000 | 1.0000 | 1 |
| logistic_permute_xor | 9.30 | 7.9956 | 0.00 | 0.00 | 99.60 | 1.0000 | 0.0066 | 1 |
| arnold_xor | 87.71 | 7.9955 | 0.00 | 0.00 | 99.41 | 1.0000 | 0.1031 | 1 |
| tiled_arnold_xor | 79.44 | 7.9956 | 0.00 | 0.00 | 99.41 | 1.0000 | 0.0202 | 1 |
| tent_block_xor | 77.05 | 8.0000 | 0.00 | 0.00 | 99.61 | 1.0000 | 1.0000 | 1 |
| sine_xor | 23.32 | 7.9983 | 0.00 | 0.00 | 99.30 | 1.0000 | 1.0000 | 1 |
| coupled_lattice_xor | 65.61 | 7.9987 | 0.00 | 0.00 | 99.41 | 1.0000 | 1.0000 | 1 |
| hamiltonian_lattice_xor | 10.82 | 7.9976 | 0.00 | 0.00 | 99.54 | 1.0000 | 1.0000 | 1 |
| chaotic_seed_blake3_xor | 232.46 | 8.0000 | 0.00 | 0.00 | 99.61 | 1.0000 | 1.0000 | 1 |
| aes_ctr | 613.40 | 8.0000 | 0.00 | 0.00 | 99.61 | 1.0000 | 1.0000 | 1 |
| chacha20 | 632.56 | 8.0000 | 0.00 | 0.00 | 99.61 | 1.0000 | 1.0000 | 1 |
| aes_256_gcm | 717.66 | 8.0000 | 0.00 | 0.00 | 99.61 | 1.0000 | 1.0000 | 1 |
| chacha20_poly1305 | 547.62 | 8.0000 | 0.00 | 0.00 | 99.61 | 1.0000 | 1.0000 | 1 |

KPA/CPA scores are empirical byte recovery ratios under deterministic key/nonce reuse. High values indicate a stream-XOR style vulnerability under reused keystream assumptions.

## Replaceable Stage Benchmarks

See `stage_bench.csv` for independent keystream, permutation, and diffusion primitive timings. See `candidate_schemes.csv` for the five SIMD-native candidate pipelines.

| candidate | keystream | permutation | diffusion | MB/s |
|---|---|---|---|---:|
