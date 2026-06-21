# Security Summary

Input image: `images/datasets/real/video_xiph_ducks/720p/frame_0007.ppm` (1280x720x3)

| cipher | MB/s | entropy | NPCR | UACI | key sensitivity | KPA score | CPA score | correct |
|---|---:|---:|---:|---:|---:|---:|---:|---:|

KPA/CPA scores are empirical byte recovery ratios under deterministic key/nonce reuse. High values indicate a stream-XOR style vulnerability under reused keystream assumptions.

## Replaceable Stage Benchmarks

See `stage_bench.csv` for independent keystream, permutation, and diffusion primitive timings. See `candidate_schemes.csv` for the five SIMD-native candidate pipelines.

| candidate | keystream | permutation | diffusion | MB/s |
|---|---|---|---|---:|
| CA-Feistel-ARX | cellular_automata | feistel_index | arx_block | 89.76 |
| Checkerboard-CA-ARX | cellular_automata | checkerboard_swaps | arx_block | 497.39 |
| Affine-CA-PrefixTree | cellular_automata | affine_bitplane | prefix_tree_reverse | 64.33 |
| Checkerboard-CA-MultilaneTree | cellular_automata | checkerboard_swaps | multilane_tree | 91.42 |
| CML-Feistel-Stencil | cml | feistel_index | stencil | 46.18 |
| Hamiltonian-Block-Stencil | hamiltonian | block_feistel | stencil | 12.73 |
| Affine-CML-Bitplane | cml | affine_bitplane | bitplane | 62.42 |
