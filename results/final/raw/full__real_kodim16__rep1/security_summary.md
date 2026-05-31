# Security Summary

Input image: `images/datasets/real/kodak/kodim16.png` (768x512x3)

| cipher | MB/s | entropy | NPCR | UACI | key sensitivity | KPA score | CPA score | correct |
|---|---:|---:|---:|---:|---:|---:|---:|---:|
| logistic_xor | 131.44 | 7.9782 | 0.00 | 0.00 | 99.39 | 1.0000 | 1.0000 | 1 |
| logistic_permute_xor | 14.06 | 7.9779 | 0.00 | 0.00 | 99.61 | 1.0000 | 0.0078 | 1 |
| arnold_xor | 121.36 | 7.9780 | 0.00 | 0.00 | 99.39 | 1.0000 | 0.1732 | 1 |
| tiled_arnold_xor | 88.27 | 7.9780 | 0.00 | 0.00 | 99.39 | 1.0000 | 0.0437 | 1 |
| tent_block_xor | 77.25 | 7.9998 | 0.00 | 0.00 | 99.62 | 1.0000 | 1.0000 | 1 |
| sine_xor | 23.26 | 7.9778 | 0.00 | 0.00 | 99.30 | 1.0000 | 1.0000 | 1 |
| coupled_lattice_xor | 70.19 | 7.9645 | 0.00 | 0.00 | 99.41 | 1.0000 | 1.0000 | 1 |
| hamiltonian_lattice_xor | 11.46 | 7.9868 | 0.00 | 0.00 | 99.53 | 1.0000 | 1.0000 | 1 |
| chaotic_seed_blake3_xor | 318.29 | 7.9999 | 0.00 | 0.00 | 99.61 | 1.0000 | 1.0000 | 1 |
| aes_ctr | 358.72 | 7.9999 | 0.00 | 0.00 | 99.61 | 1.0000 | 1.0000 | 1 |
| chacha20 | 757.23 | 7.9998 | 0.00 | 0.00 | 99.62 | 1.0000 | 1.0000 | 1 |

KPA/CPA scores are empirical byte recovery ratios under deterministic key/nonce reuse. High values indicate a stream-XOR style vulnerability under reused keystream assumptions.

## Replaceable Stage Benchmarks

See `stage_bench.csv` for independent keystream, permutation, and diffusion primitive timings. See `candidate_schemes.csv` for the five SIMD-native candidate pipelines.

| candidate | keystream | permutation | diffusion | MB/s |
|---|---|---|---|---:|
