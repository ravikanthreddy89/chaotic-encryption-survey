# Security Summary

Input image: `images/datasets/real/video_xiph_ducks/1080p/frame_0001.ppm` (1920x1080x3)

| cipher | MB/s | entropy | NPCR | UACI | key sensitivity | KPA score | CPA score | correct |
|---|---:|---:|---:|---:|---:|---:|---:|---:|

KPA/CPA scores are empirical byte recovery ratios under deterministic key/nonce reuse. High values indicate a stream-XOR style vulnerability under reused keystream assumptions.

## Replaceable Stage Benchmarks

See `stage_bench.csv` for independent keystream, permutation, and diffusion primitive timings. See `candidate_schemes.csv` for the five SIMD-native candidate pipelines.

| candidate | keystream | permutation | diffusion | MB/s |
|---|---|---|---|---:|
| CA-Feistel-ARX | cellular_automata | feistel_index | arx_block | 73.15 |
| Checkerboard-CA-ARX | cellular_automata | checkerboard_swaps | arx_block | 203.41 |
| Affine-CA-PrefixTree | cellular_automata | affine_bitplane | prefix_tree_reverse | 40.88 |
| Checkerboard-CA-MultilaneTree | cellular_automata | checkerboard_swaps | multilane_tree | 161.22 |
| CML-Feistel-Stencil | cml | feistel_index | stencil | 31.58 |
| Hamiltonian-Block-Stencil | hamiltonian | block_feistel | stencil | 9.55 |
| Affine-CML-Bitplane | cml | affine_bitplane | bitplane | 29.12 |
