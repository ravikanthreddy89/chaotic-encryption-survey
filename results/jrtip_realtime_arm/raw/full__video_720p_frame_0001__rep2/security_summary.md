# Security Summary

Input image: `images/datasets/real/video_xiph_ducks/720p/frame_0001.ppm` (1280x720x3)

| cipher | MB/s | entropy | NPCR | UACI | key sensitivity | KPA score | CPA score | correct |
|---|---:|---:|---:|---:|---:|---:|---:|---:|
| logistic_xor | 201.30 | 7.9956 | 0.00 | 0.00 | 99.40 | 1.0000 | 1.0000 | 1 |
| logistic_permute_xor | 15.69 | 7.9957 | 0.00 | 0.00 | 99.61 | 1.0000 | 0.0066 | 1 |
| arnold_xor | 155.44 | 7.9958 | 0.00 | 0.00 | 99.40 | 1.0000 | 0.1031 | 1 |
| tiled_arnold_xor | 136.57 | 7.9956 | 0.00 | 0.00 | 99.40 | 1.0000 | 0.0249 | 1 |
| tent_block_xor | 153.60 | 7.9999 | 0.00 | 0.00 | 99.62 | 1.0000 | 1.0000 | 1 |
| sine_xor | 33.79 | 7.9982 | 0.00 | 0.00 | 99.31 | 1.0000 | 1.0000 | 1 |
| coupled_lattice_xor | 94.13 | 7.9987 | 0.00 | 0.00 | 99.42 | 1.0000 | 1.0000 | 1 |
| hamiltonian_lattice_xor | 14.78 | 7.9976 | 0.00 | 0.00 | 99.54 | 1.0000 | 1.0000 | 1 |
| chaotic_seed_blake3_xor | 417.49 | 7.9999 | 0.00 | 0.00 | 99.61 | 1.0000 | 1.0000 | 1 |
| aes_ctr | 766.97 | 7.9999 | 0.00 | 0.00 | 99.62 | 1.0000 | 1.0000 | 1 |
| chacha20 | 843.57 | 7.9999 | 0.00 | 0.00 | 99.61 | 1.0000 | 1.0000 | 1 |
| aes_256_gcm | 1431.63 | 7.9999 | 0.00 | 0.00 | 99.61 | 1.0000 | 1.0000 | 1 |
| chacha20_poly1305 | 617.12 | 7.9999 | 0.00 | 0.00 | 99.61 | 1.0000 | 1.0000 | 1 |

KPA/CPA scores are empirical byte recovery ratios under deterministic key/nonce reuse. High values indicate a stream-XOR style vulnerability under reused keystream assumptions.

## Replaceable Stage Benchmarks

See `stage_bench.csv` for independent keystream, permutation, and diffusion primitive timings. See `candidate_schemes.csv` for the five SIMD-native candidate pipelines.

| candidate | keystream | permutation | diffusion | MB/s |
|---|---|---|---|---:|
