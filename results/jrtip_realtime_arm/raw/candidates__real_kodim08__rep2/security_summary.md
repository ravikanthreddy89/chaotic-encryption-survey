# Security Summary

Input image: `images/datasets/real/kodak/kodim08.png` (768x512x3)

| cipher | MB/s | entropy | NPCR | UACI | key sensitivity | KPA score | CPA score | correct |
|---|---:|---:|---:|---:|---:|---:|---:|---:|

KPA/CPA scores are empirical byte recovery ratios under deterministic key/nonce reuse. High values indicate a stream-XOR style vulnerability under reused keystream assumptions.

## Replaceable Stage Benchmarks

See `stage_bench.csv` for independent keystream, permutation, and diffusion primitive timings. See `candidate_schemes.csv` for the five SIMD-native candidate pipelines.

| candidate | keystream | permutation | diffusion | MB/s |
|---|---|---|---|---:|
| CA-Feistel-ARX | cellular_automata | feistel_index | arx_block | 65.89 |
| Checkerboard-CA-ARX | cellular_automata | checkerboard_swaps | arx_block | 471.47 |
| Affine-CA-PrefixTree | cellular_automata | affine_bitplane | prefix_tree_reverse | 67.86 |
| Checkerboard-CA-MultilaneTree | cellular_automata | checkerboard_swaps | multilane_tree | 88.88 |
| CML-Feistel-Stencil | cml | feistel_index | stencil | 38.98 |
| Hamiltonian-Block-Stencil | hamiltonian | block_feistel | stencil | 12.10 |
| Affine-CML-Bitplane | cml | affine_bitplane | bitplane | 68.84 |
