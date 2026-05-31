# Security Summary

Input image: `images/datasets/real/kodak/kodim11.png` (768x512x3)

| cipher | MB/s | entropy | NPCR | UACI | key sensitivity | KPA score | CPA score | correct |
|---|---:|---:|---:|---:|---:|---:|---:|---:|

KPA/CPA scores are empirical byte recovery ratios under deterministic key/nonce reuse. High values indicate a stream-XOR style vulnerability under reused keystream assumptions.

## Replaceable Stage Benchmarks

See `stage_bench.csv` for independent keystream, permutation, and diffusion primitive timings. See `candidate_schemes.csv` for the five SIMD-native candidate pipelines.

| candidate | keystream | permutation | diffusion | MB/s |
|---|---|---|---|---:|
| CA-Feistel-ARX | cellular_automata | feistel_index | arx_block | 55.48 |
| Checkerboard-CA-ARX | cellular_automata | checkerboard_swaps | arx_block | 277.93 |
| Affine-CA-PrefixTree | cellular_automata | affine_bitplane | prefix_tree_reverse | 48.34 |
| Checkerboard-CA-MultilaneTree | cellular_automata | checkerboard_swaps | multilane_tree | 244.34 |
| CML-Feistel-Stencil | cml | feistel_index | stencil | 30.76 |
| Hamiltonian-Block-Stencil | hamiltonian | block_feistel | stencil | 9.65 |
| Affine-CML-Bitplane | cml | affine_bitplane | bitplane | 32.12 |
