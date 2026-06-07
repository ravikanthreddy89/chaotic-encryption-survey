# Security Summary

Input image: `images/datasets/real/kodak/kodim13.png` (768x512x3)

| cipher | MB/s | entropy | NPCR | UACI | key sensitivity | KPA score | CPA score | correct |
|---|---:|---:|---:|---:|---:|---:|---:|---:|

KPA/CPA scores are empirical byte recovery ratios under deterministic key/nonce reuse. High values indicate a stream-XOR style vulnerability under reused keystream assumptions.

## Replaceable Stage Benchmarks

See `stage_bench.csv` for independent keystream, permutation, and diffusion primitive timings. See `candidate_schemes.csv` for the five SIMD-native candidate pipelines.

| candidate | keystream | permutation | diffusion | MB/s |
|---|---|---|---|---:|
| CA-Feistel-ARX | cellular_automata | feistel_index | arx_block | 53.05 |
| Checkerboard-CA-ARX | cellular_automata | checkerboard_swaps | arx_block | 255.49 |
| Affine-CA-PrefixTree | cellular_automata | affine_bitplane | prefix_tree_reverse | 46.58 |
| Checkerboard-CA-MultilaneTree | cellular_automata | checkerboard_swaps | multilane_tree | 206.25 |
| CML-Feistel-Stencil | cml | feistel_index | stencil | 27.51 |
| Hamiltonian-Block-Stencil | hamiltonian | block_feistel | stencil | 9.23 |
| Affine-CML-Bitplane | cml | affine_bitplane | bitplane | 32.02 |
