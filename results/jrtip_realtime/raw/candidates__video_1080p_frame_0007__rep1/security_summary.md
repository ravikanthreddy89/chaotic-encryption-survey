# Security Summary

Input image: `images/datasets/real/video_xiph_ducks/1080p/frame_0007.ppm` (1920x1080x3)

| cipher | MB/s | entropy | NPCR | UACI | key sensitivity | KPA score | CPA score | correct |
|---|---:|---:|---:|---:|---:|---:|---:|---:|

KPA/CPA scores are empirical byte recovery ratios under deterministic key/nonce reuse. High values indicate a stream-XOR style vulnerability under reused keystream assumptions.

## Replaceable Stage Benchmarks

See `stage_bench.csv` for independent keystream, permutation, and diffusion primitive timings. See `candidate_schemes.csv` for the five SIMD-native candidate pipelines.

| candidate | keystream | permutation | diffusion | MB/s |
|---|---|---|---|---:|
| CA-Feistel-ARX | cellular_automata | feistel_index | arx_block | 76.07 |
| Checkerboard-CA-ARX | cellular_automata | checkerboard_swaps | arx_block | 198.97 |
| Affine-CA-PrefixTree | cellular_automata | affine_bitplane | prefix_tree_reverse | 41.36 |
| Checkerboard-CA-MultilaneTree | cellular_automata | checkerboard_swaps | multilane_tree | 174.28 |
| CML-Feistel-Stencil | cml | feistel_index | stencil | 33.78 |
| Hamiltonian-Block-Stencil | hamiltonian | block_feistel | stencil | 9.80 |
| Affine-CML-Bitplane | cml | affine_bitplane | bitplane | 29.46 |
