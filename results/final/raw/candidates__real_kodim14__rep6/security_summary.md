# Security Summary

Input image: `images/datasets/real/kodak/kodim14.png` (768x512x3)

| cipher | MB/s | entropy | NPCR | UACI | key sensitivity | KPA score | CPA score | correct |
|---|---:|---:|---:|---:|---:|---:|---:|---:|

KPA/CPA scores are empirical byte recovery ratios under deterministic key/nonce reuse. High values indicate a stream-XOR style vulnerability under reused keystream assumptions.

## Replaceable Stage Benchmarks

See `stage_bench.csv` for independent keystream, permutation, and diffusion primitive timings. See `candidate_schemes.csv` for the five SIMD-native candidate pipelines.

| candidate | keystream | permutation | diffusion | MB/s |
|---|---|---|---|---:|
| CA-Feistel-ARX | cellular_automata | feistel_index | arx_block | 58.06 |
| Checkerboard-CA-ARX | cellular_automata | checkerboard_swaps | arx_block | 274.47 |
| Affine-CA-PrefixTree | cellular_automata | affine_bitplane | prefix_tree_reverse | 51.05 |
| Checkerboard-CA-MultilaneTree | cellular_automata | checkerboard_swaps | multilane_tree | 246.15 |
| CML-Feistel-Stencil | cml | feistel_index | stencil | 30.32 |
| Hamiltonian-Block-Stencil | hamiltonian | block_feistel | stencil | 10.72 |
| Affine-CML-Bitplane | cml | affine_bitplane | bitplane | 34.96 |
