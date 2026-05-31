# Security Summary

Input image: `images/datasets/synthetic/gradient_512x512.ppm` (512x512x3)

| cipher | MB/s | entropy | NPCR | UACI | key sensitivity | KPA score | CPA score | correct |
|---|---:|---:|---:|---:|---:|---:|---:|---:|

KPA/CPA scores are empirical byte recovery ratios under deterministic key/nonce reuse. High values indicate a stream-XOR style vulnerability under reused keystream assumptions.

## Replaceable Stage Benchmarks

See `stage_bench.csv` for independent keystream, permutation, and diffusion primitive timings. See `candidate_schemes.csv` for the five SIMD-native candidate pipelines.

| candidate | keystream | permutation | diffusion | MB/s |
|---|---|---|---|---:|
| CA-Feistel-ARX | cellular_automata | feistel_index | arx_block | 79.57 |
| Checkerboard-CA-ARX | cellular_automata | checkerboard_swaps | arx_block | 258.16 |
| Affine-CA-PrefixTree | cellular_automata | affine_bitplane | prefix_tree_reverse | 46.12 |
| Checkerboard-CA-MultilaneTree | cellular_automata | checkerboard_swaps | multilane_tree | 238.11 |
| CML-Feistel-Stencil | cml | feistel_index | stencil | 35.97 |
| Hamiltonian-Block-Stencil | hamiltonian | block_feistel | stencil | 9.92 |
| Affine-CML-Bitplane | cml | affine_bitplane | bitplane | 31.37 |
