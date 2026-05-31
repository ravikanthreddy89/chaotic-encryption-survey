# Security Summary

Input image: `images/datasets/real/kodak/kodim15.png` (768x512x3)

| cipher | MB/s | entropy | NPCR | UACI | key sensitivity | KPA score | CPA score | correct |
|---|---:|---:|---:|---:|---:|---:|---:|---:|
| logistic_xor | 141.67 | 7.9846 | 0.00 | 0.00 | 99.39 | 1.0000 | 1.0000 | 1 |
| logistic_permute_xor | 13.77 | 7.9847 | 0.00 | 0.00 | 99.60 | 1.0000 | 0.0119 | 1 |
| arnold_xor | 128.90 | 7.9843 | 0.00 | 0.00 | 99.39 | 1.0000 | 0.1742 | 1 |
| tiled_arnold_xor | 96.03 | 7.9845 | 0.00 | 0.00 | 99.39 | 1.0000 | 0.0859 | 1 |
| tent_block_xor | 76.83 | 7.9998 | 0.00 | 0.00 | 99.62 | 1.0000 | 1.0000 | 1 |
| sine_xor | 23.65 | 7.9914 | 0.00 | 0.00 | 99.30 | 1.0000 | 1.0000 | 1 |
| coupled_lattice_xor | 69.86 | 7.9863 | 0.00 | 0.00 | 99.41 | 1.0000 | 1.0000 | 1 |
| hamiltonian_lattice_xor | 10.92 | 7.9928 | 0.00 | 0.00 | 99.53 | 1.0000 | 1.0000 | 1 |
| chaotic_seed_blake3_xor | 295.49 | 7.9998 | 0.00 | 0.00 | 99.61 | 1.0000 | 1.0000 | 1 |
| aes_ctr | 345.28 | 7.9999 | 0.00 | 0.00 | 99.61 | 1.0000 | 1.0000 | 1 |
| chacha20 | 853.60 | 7.9998 | 0.00 | 0.00 | 99.62 | 1.0000 | 1.0000 | 1 |

KPA/CPA scores are empirical byte recovery ratios under deterministic key/nonce reuse. High values indicate a stream-XOR style vulnerability under reused keystream assumptions.

## Replaceable Stage Benchmarks

See `stage_bench.csv` for independent keystream, permutation, and diffusion primitive timings. See `candidate_schemes.csv` for the five SIMD-native candidate pipelines.

| candidate | keystream | permutation | diffusion | MB/s |
|---|---|---|---|---:|
