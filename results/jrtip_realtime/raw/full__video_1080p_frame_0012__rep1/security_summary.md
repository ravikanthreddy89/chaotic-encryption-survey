# Security Summary

Input image: `images/datasets/real/video_xiph_ducks/1080p/frame_0012.ppm` (1920x1080x3)

| cipher | MB/s | entropy | NPCR | UACI | key sensitivity | KPA score | CPA score | correct |
|---|---:|---:|---:|---:|---:|---:|---:|---:|
| logistic_xor | 117.95 | 7.9947 | 0.00 | 0.00 | 99.41 | 1.0000 | 1.0000 | 1 |
| logistic_permute_xor | 9.75 | 7.9947 | 0.00 | 0.00 | 99.61 | 1.0000 | 0.0065 | 1 |
| arnold_xor | 99.94 | 7.9947 | 0.00 | 0.00 | 99.41 | 1.0000 | 0.1029 | 1 |
| tiled_arnold_xor | 78.37 | 7.9948 | 0.00 | 0.00 | 99.41 | 1.0000 | 0.0206 | 1 |
| tent_block_xor | 76.13 | 8.0000 | 0.00 | 0.00 | 99.61 | 1.0000 | 1.0000 | 1 |
| sine_xor | 23.11 | 7.9977 | 0.00 | 0.00 | 99.30 | 1.0000 | 1.0000 | 1 |
| coupled_lattice_xor | 65.61 | 7.9968 | 0.00 | 0.00 | 99.41 | 1.0000 | 1.0000 | 1 |
| hamiltonian_lattice_xor | 10.95 | 7.9971 | 0.00 | 0.00 | 99.54 | 1.0000 | 1.0000 | 1 |
| chaotic_seed_blake3_xor | 236.81 | 8.0000 | 0.00 | 0.00 | 99.61 | 1.0000 | 1.0000 | 1 |
| aes_ctr | 579.65 | 8.0000 | 0.00 | 0.00 | 99.61 | 1.0000 | 1.0000 | 1 |
| chacha20 | 679.82 | 8.0000 | 0.00 | 0.00 | 99.61 | 1.0000 | 1.0000 | 1 |
| aes_256_gcm | 770.54 | 8.0000 | 0.00 | 0.00 | 99.61 | 1.0000 | 1.0000 | 1 |
| chacha20_poly1305 | 557.77 | 8.0000 | 0.00 | 0.00 | 99.61 | 1.0000 | 1.0000 | 1 |

KPA/CPA scores are empirical byte recovery ratios under deterministic key/nonce reuse. High values indicate a stream-XOR style vulnerability under reused keystream assumptions.

## Replaceable Stage Benchmarks

See `stage_bench.csv` for independent keystream, permutation, and diffusion primitive timings. See `candidate_schemes.csv` for the five SIMD-native candidate pipelines.

| candidate | keystream | permutation | diffusion | MB/s |
|---|---|---|---|---:|
