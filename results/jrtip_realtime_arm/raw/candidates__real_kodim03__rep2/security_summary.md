# Security Summary

Input image: `images/datasets/real/kodak/kodim03.png` (768x512x3)

| cipher | MB/s | entropy | NPCR | UACI | key sensitivity | KPA score | CPA score | correct |
|---|---:|---:|---:|---:|---:|---:|---:|---:|

KPA/CPA scores are empirical byte recovery ratios under deterministic key/nonce reuse. High values indicate a stream-XOR style vulnerability under reused keystream assumptions.

## Replaceable Stage Benchmarks

See `stage_bench.csv` for independent keystream, permutation, and diffusion primitive timings. See `candidate_schemes.csv` for the five SIMD-native candidate pipelines.

| candidate | keystream | permutation | diffusion | MB/s |
|---|---|---|---|---:|
| CA-Feistel-ARX | cellular_automata | feistel_index | arx_block | 61.32 |
| Checkerboard-CA-ARX | cellular_automata | checkerboard_swaps | arx_block | 434.78 |
| Affine-CA-PrefixTree | cellular_automata | affine_bitplane | prefix_tree_reverse | 68.19 |
| Checkerboard-CA-MultilaneTree | cellular_automata | checkerboard_swaps | multilane_tree | 87.90 |
| CML-Feistel-Stencil | cml | feistel_index | stencil | 37.69 |
| Hamiltonian-Block-Stencil | hamiltonian | block_feistel | stencil | 11.84 |
| Affine-CML-Bitplane | cml | affine_bitplane | bitplane | 68.37 |
