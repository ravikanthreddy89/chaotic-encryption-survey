# Security Summary

Input image: `images/datasets/real/kodak/kodim17.png` (512x768x3)

| cipher | MB/s | entropy | NPCR | UACI | key sensitivity | KPA score | CPA score | correct |
|---|---:|---:|---:|---:|---:|---:|---:|---:|

KPA/CPA scores are empirical byte recovery ratios under deterministic key/nonce reuse. High values indicate a stream-XOR style vulnerability under reused keystream assumptions.

## Replaceable Stage Benchmarks

See `stage_bench.csv` for independent keystream, permutation, and diffusion primitive timings. See `candidate_schemes.csv` for the five SIMD-native candidate pipelines.

| candidate | keystream | permutation | diffusion | MB/s |
|---|---|---|---|---:|
| CA-Feistel-ARX | cellular_automata | feistel_index | arx_block | 55.66 |
| Checkerboard-CA-ARX | cellular_automata | checkerboard_swaps | arx_block | 256.75 |
| Affine-CA-PrefixTree | cellular_automata | affine_bitplane | prefix_tree_reverse | 47.73 |
| Checkerboard-CA-MultilaneTree | cellular_automata | checkerboard_swaps | multilane_tree | 239.62 |
| CML-Feistel-Stencil | cml | feistel_index | stencil | 29.49 |
| Hamiltonian-Block-Stencil | hamiltonian | block_feistel | stencil | 9.99 |
| Affine-CML-Bitplane | cml | affine_bitplane | bitplane | 32.80 |
