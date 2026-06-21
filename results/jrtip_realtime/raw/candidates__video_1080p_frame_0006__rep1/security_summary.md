# Security Summary

Input image: `images/datasets/real/video_xiph_ducks/1080p/frame_0006.ppm` (1920x1080x3)

| cipher | MB/s | entropy | NPCR | UACI | key sensitivity | KPA score | CPA score | correct |
|---|---:|---:|---:|---:|---:|---:|---:|---:|

KPA/CPA scores are empirical byte recovery ratios under deterministic key/nonce reuse. High values indicate a stream-XOR style vulnerability under reused keystream assumptions.

## Replaceable Stage Benchmarks

See `stage_bench.csv` for independent keystream, permutation, and diffusion primitive timings. See `candidate_schemes.csv` for the five SIMD-native candidate pipelines.

| candidate | keystream | permutation | diffusion | MB/s |
|---|---|---|---|---:|
| CA-Feistel-ARX | cellular_automata | feistel_index | arx_block | 77.96 |
| Checkerboard-CA-ARX | cellular_automata | checkerboard_swaps | arx_block | 193.30 |
| Affine-CA-PrefixTree | cellular_automata | affine_bitplane | prefix_tree_reverse | 40.70 |
| Checkerboard-CA-MultilaneTree | cellular_automata | checkerboard_swaps | multilane_tree | 158.47 |
| CML-Feistel-Stencil | cml | feistel_index | stencil | 31.70 |
| Hamiltonian-Block-Stencil | hamiltonian | block_feistel | stencil | 9.60 |
| Affine-CML-Bitplane | cml | affine_bitplane | bitplane | 29.42 |
