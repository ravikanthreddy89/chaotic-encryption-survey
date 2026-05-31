# Security Summary

Input image: `images/datasets/real/kodak/kodim18.png` (512x768x3)

| cipher | MB/s | entropy | NPCR | UACI | key sensitivity | KPA score | CPA score | correct |
|---|---:|---:|---:|---:|---:|---:|---:|---:|

KPA/CPA scores are empirical byte recovery ratios under deterministic key/nonce reuse. High values indicate a stream-XOR style vulnerability under reused keystream assumptions.

## Replaceable Stage Benchmarks

See `stage_bench.csv` for independent keystream, permutation, and diffusion primitive timings. See `candidate_schemes.csv` for the five SIMD-native candidate pipelines.

| candidate | keystream | permutation | diffusion | MB/s |
|---|---|---|---|---:|
| CA-Feistel-ARX | cellular_automata | feistel_index | arx_block | 53.38 |
| Checkerboard-CA-ARX | cellular_automata | checkerboard_swaps | arx_block | 267.36 |
| Affine-CA-PrefixTree | cellular_automata | affine_bitplane | prefix_tree_reverse | 45.44 |
| Checkerboard-CA-MultilaneTree | cellular_automata | checkerboard_swaps | multilane_tree | 226.95 |
| CML-Feistel-Stencil | cml | feistel_index | stencil | 28.19 |
| Hamiltonian-Block-Stencil | hamiltonian | block_feistel | stencil | 9.47 |
| Affine-CML-Bitplane | cml | affine_bitplane | bitplane | 30.52 |
