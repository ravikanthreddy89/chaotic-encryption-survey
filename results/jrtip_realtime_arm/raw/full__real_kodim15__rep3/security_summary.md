# Security Summary

Input image: `images/datasets/real/kodak/kodim15.png` (768x512x3)

| cipher | MB/s | entropy | NPCR | UACI | key sensitivity | KPA score | CPA score | correct |
|---|---:|---:|---:|---:|---:|---:|---:|---:|
| logistic_xor | 181.96 | 7.9846 | 0.00 | 0.00 | 99.39 | 1.0000 | 1.0000 | 1 |
| logistic_permute_xor | 18.61 | 7.9847 | 0.00 | 0.00 | 99.60 | 1.0000 | 0.0119 | 1 |
| arnold_xor | 155.59 | 7.9843 | 0.00 | 0.00 | 99.39 | 1.0000 | 0.1742 | 1 |
| tiled_arnold_xor | 140.85 | 7.9845 | 0.00 | 0.00 | 99.39 | 1.0000 | 0.0859 | 1 |
| tent_block_xor | 158.13 | 7.9998 | 0.00 | 0.00 | 99.62 | 1.0000 | 1.0000 | 1 |
| sine_xor | 33.97 | 7.9914 | 0.00 | 0.00 | 99.30 | 1.0000 | 1.0000 | 1 |
| coupled_lattice_xor | 98.49 | 7.9863 | 0.00 | 0.00 | 99.41 | 1.0000 | 1.0000 | 1 |
| hamiltonian_lattice_xor | 14.82 | 7.9928 | 0.00 | 0.00 | 99.53 | 1.0000 | 1.0000 | 1 |
| chaotic_seed_blake3_xor | 431.83 | 7.9998 | 0.00 | 0.00 | 99.61 | 1.0000 | 1.0000 | 1 |
| aes_ctr | 472.02 | 7.9999 | 0.00 | 0.00 | 99.61 | 1.0000 | 1.0000 | 1 |
| chacha20 | 857.00 | 7.9998 | 0.00 | 0.00 | 99.62 | 1.0000 | 1.0000 | 1 |
| aes_256_gcm | 1448.93 | 7.9998 | 0.00 | 0.00 | 99.60 | 1.0000 | 1.0000 | 1 |
| chacha20_poly1305 | 621.97 | 7.9998 | 0.00 | 0.00 | 99.61 | 1.0000 | 1.0000 | 1 |

KPA/CPA scores are empirical byte recovery ratios under deterministic key/nonce reuse. High values indicate a stream-XOR style vulnerability under reused keystream assumptions.

## Replaceable Stage Benchmarks

See `stage_bench.csv` for independent keystream, permutation, and diffusion primitive timings. See `candidate_schemes.csv` for the five SIMD-native candidate pipelines.

| candidate | keystream | permutation | diffusion | MB/s |
|---|---|---|---|---:|
