# Security Summary

Input image: `images/datasets/synthetic/gradient_512x512.ppm` (512x512x3)

| cipher | MB/s | entropy | NPCR | UACI | key sensitivity | KPA score | CPA score | correct |
|---|---:|---:|---:|---:|---:|---:|---:|---:|
| logistic_xor | 126.41 | 7.9998 | 0.00 | 0.00 | 99.38 | 1.0000 | 1.0000 | 1 |
| logistic_permute_xor | 13.87 | 7.9998 | 0.00 | 0.00 | 99.61 | 1.0000 | 0.0039 | 1 |
| arnold_xor | 96.09 | 7.9998 | 0.00 | 0.00 | 99.38 | 1.0000 | 0.0039 | 1 |
| tiled_arnold_xor | 82.46 | 7.9998 | 0.00 | 0.00 | 99.38 | 1.0000 | 0.0120 | 1 |
| tent_block_xor | 70.08 | 7.9997 | 0.00 | 0.00 | 99.63 | 1.0000 | 1.0000 | 1 |
| sine_xor | 23.28 | 7.9998 | 0.00 | 0.00 | 99.30 | 1.0000 | 1.0000 | 1 |
| coupled_lattice_xor | 70.93 | 7.9998 | 0.00 | 0.00 | 99.41 | 1.0000 | 1.0000 | 1 |
| hamiltonian_lattice_xor | 10.32 | 7.9998 | 0.00 | 0.00 | 99.53 | 1.0000 | 1.0000 | 1 |
| chaotic_seed_blake3_xor | 318.93 | 7.9998 | 0.00 | 0.00 | 99.60 | 1.0000 | 1.0000 | 1 |
| aes_ctr | 241.05 | 7.9997 | 0.00 | 0.00 | 99.61 | 1.0000 | 1.0000 | 1 |
| chacha20 | 966.16 | 7.9998 | 0.00 | 0.00 | 99.62 | 1.0000 | 1.0000 | 1 |

KPA/CPA scores are empirical byte recovery ratios under deterministic key/nonce reuse. High values indicate a stream-XOR style vulnerability under reused keystream assumptions.

## Replaceable Stage Benchmarks

See `stage_bench.csv` for independent keystream, permutation, and diffusion primitive timings. See `candidate_schemes.csv` for the five SIMD-native candidate pipelines.

| candidate | keystream | permutation | diffusion | MB/s |
|---|---|---|---|---:|
