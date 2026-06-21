# Security Summary

Input image: `images/datasets/real/video_xiph_ducks/720p/frame_0006.ppm` (1280x720x3)

| cipher | MB/s | entropy | NPCR | UACI | key sensitivity | KPA score | CPA score | correct |
|---|---:|---:|---:|---:|---:|---:|---:|---:|

KPA/CPA scores are empirical byte recovery ratios under deterministic key/nonce reuse. High values indicate a stream-XOR style vulnerability under reused keystream assumptions.

## Replaceable Stage Benchmarks

See `stage_bench.csv` for independent keystream, permutation, and diffusion primitive timings. See `candidate_schemes.csv` for the five SIMD-native candidate pipelines.

| candidate | keystream | permutation | diffusion | MB/s |
|---|---|---|---|---:|
| CA-Feistel-ARX | cellular_automata | feistel_index | arx_block | 63.83 |
| Checkerboard-CA-ARX | cellular_automata | checkerboard_swaps | arx_block | 208.64 |
| Affine-CA-PrefixTree | cellular_automata | affine_bitplane | prefix_tree_reverse | 43.41 |
| Checkerboard-CA-MultilaneTree | cellular_automata | checkerboard_swaps | multilane_tree | 162.16 |
| CML-Feistel-Stencil | cml | feistel_index | stencil | 29.83 |
| Hamiltonian-Block-Stencil | hamiltonian | block_feistel | stencil | 9.35 |
| Affine-CML-Bitplane | cml | affine_bitplane | bitplane | 29.24 |
