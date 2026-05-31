# Security Summary

Input image: `images/datasets/synthetic/noise_2048x2048.ppm` (2048x2048x3)

| cipher | MB/s | entropy | NPCR | UACI | key sensitivity | KPA score | CPA score | correct |
|---|---:|---:|---:|---:|---:|---:|---:|---:|

KPA/CPA scores are empirical byte recovery ratios under deterministic key/nonce reuse. High values indicate a stream-XOR style vulnerability under reused keystream assumptions.

## Replaceable Stage Benchmarks

See `stage_bench.csv` for independent keystream, permutation, and diffusion primitive timings. See `candidate_schemes.csv` for the five SIMD-native candidate pipelines.

| candidate | keystream | permutation | diffusion | MB/s |
|---|---|---|---|---:|
| CA-Feistel-ARX | cellular_automata | feistel_index | arx_block | 62.38 |
| Checkerboard-CA-ARX | cellular_automata | checkerboard_swaps | arx_block | 202.91 |
| Affine-CA-PrefixTree | cellular_automata | affine_bitplane | prefix_tree_reverse | 36.42 |
| Checkerboard-CA-MultilaneTree | cellular_automata | checkerboard_swaps | multilane_tree | 148.17 |
| CML-Feistel-Stencil | cml | feistel_index | stencil | 27.92 |
| Hamiltonian-Block-Stencil | hamiltonian | block_feistel | stencil | 9.35 |
| Affine-CML-Bitplane | cml | affine_bitplane | bitplane | 27.60 |
