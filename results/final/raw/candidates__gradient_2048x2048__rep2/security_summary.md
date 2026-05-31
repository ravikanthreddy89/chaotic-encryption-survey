# Security Summary

Input image: `images/datasets/synthetic/gradient_2048x2048.ppm` (2048x2048x3)

| cipher | MB/s | entropy | NPCR | UACI | key sensitivity | KPA score | CPA score | correct |
|---|---:|---:|---:|---:|---:|---:|---:|---:|

KPA/CPA scores are empirical byte recovery ratios under deterministic key/nonce reuse. High values indicate a stream-XOR style vulnerability under reused keystream assumptions.

## Replaceable Stage Benchmarks

See `stage_bench.csv` for independent keystream, permutation, and diffusion primitive timings. See `candidate_schemes.csv` for the five SIMD-native candidate pipelines.

| candidate | keystream | permutation | diffusion | MB/s |
|---|---|---|---|---:|
| CA-Feistel-ARX | cellular_automata | feistel_index | arx_block | 65.04 |
| CML-Feistel-Stencil | cml | feistel_index | stencil | 30.23 |
| Hamiltonian-Block-Stencil | hamiltonian | block_feistel | stencil | 9.51 |
| Affine-CML-Bitplane | cml | affine_bitplane | bitplane | 29.91 |
| Checkerboard-CA-ARX | cellular_automata | checkerboard_swaps | arx_block | 217.37 |
| Affine-CA-PrefixTree | cellular_automata | affine_bitplane | prefix_tree_reverse | 41.29 |
| Checkerboard-CA-MultilaneTree | cellular_automata | checkerboard_swaps | multilane_tree | 168.51 |
