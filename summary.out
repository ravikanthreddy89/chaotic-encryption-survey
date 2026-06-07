# Q2/Q3 Applied Performance Summary

This experiment package is positioned as an applied performance study, not as a claim that chaotic schemes beat AES or ChaCha.

## Main Empirical Story

- Traditional chaotic schemes are dominated by implementation-hostile stages: floating-point keystreams, sort-based permutation, and serial diffusion chains.
- SIMD-friendly redesigns move the useful contribution to replaceable stages: cellular automata keystreams, checkerboard/block/Feistel-style permutations, and lane-local/tree/prefix diffusion.
- AES-CTR and ChaCha20 remain the correct cryptographic baselines; the redesigned chaotic candidates are research prototypes for image-domain transformation tradeoffs.

## Best Full Schemes
| cipher | image_size | MBps | total_ms |
|---|---|---|---|
| chacha20 | 512x512 | 1254.72 | 0.597744 |
| aes_ctr | 1024x1024 | 637.37 | 4.70684 |
| chaotic_seed_blake3_xor | 512x512 | 367.918 | 2.0385 |
| logistic_xor | 768x512 | 155.428 | 7.23806 |
| arnold_xor | 768x512 | 136.274 | 8.25541 |
| tiled_arnold_xor | 512x512 | 101.158 | 7.41415 |
| tent_block_xor | 768x512 | 96.9435 | 11.6047 |
| coupled_lattice_xor | 512x512 | 80.3683 | 9.33203 |

## Aggregated Full-Scheme Statistics
| cipher | image_size | n | MBps_mean | MBps_ci95 | MBps_mean_speedup_vs_baseline |
|---|---|---|---|---|---|
| chacha20 | 512x512 | 30 | 996.357 | 42.849 | 7.128 |
| chacha20 | 1024x1024 | 30 | 903.400 | 18.640 | 6.532 |
| chacha20 | 512x768 | 60 | 902.702 | 15.305 | 6.507 |
| chacha20 | 768x512 | 180 | 895.707 | 10.484 | 6.389 |
| aes_ctr | 1024x1024 | 30 | 594.958 | 9.202 | 4.302 |
| aes_ctr | 768x512 | 180 | 355.663 | 3.399 | 2.537 |
| aes_ctr | 512x768 | 60 | 352.983 | 4.705 | 2.544 |
| chaotic_seed_blake3_xor | 512x512 | 30 | 320.435 | 7.216 | 2.292 |

## Best Stage Primitives
| category | stage | image_size | MBps | ms |
|---|---|---|---|---|
| permutation | block_permutation | 768x512 | 583.816 | 1.92698 |
| diffusion | multilane_chain_avx2 | 768x512 | 562.072 | 2.00152 |
| diffusion | tree_xor_avx2 | 768x512 | 523.017 | 2.15098 |
| diffusion | arx_block_diffusion | 768x512 | 479.167 | 2.34782 |
| diffusion | prefix_xor_avx2 | 768x512 | 411.249 | 2.73557 |
| diffusion | block_local_chain | 512x768 | 342.094 | 3.28857 |
| diffusion | bitplane_diffusion | 768x512 | 331.345 | 3.39525 |
| diffusion | parallel_prefix | 768x512 | 328.641 | 3.42319 |
| keystream | cellular_automata | 512x768 | 328.159 | 3.42822 |
| diffusion | reverse_prefix_xor_avx2 | 512x512 | 321.233 | 2.33475 |

## Aggregated Stage Statistics
| category | stage | image_size | n | MBps_mean | MBps_ci95 | MBps_mean_speedup_vs_baseline |
|---|---|---|---|---|---|---|
| permutation | block_permutation | 512x512 | 30 | 512.371 | 6.473 | 100.634 |
| diffusion | multilane_chain_avx2 | 512x768 | 60 | 503.969 | 4.579 | 2.126 |
| permutation | block_permutation | 768x512 | 180 | 499.812 | 2.866 | 109.371 |
| diffusion | multilane_chain_avx2 | 512x512 | 30 | 499.419 | 7.774 | 2.076 |
| permutation | block_permutation | 512x768 | 60 | 499.309 | 4.729 | 110.013 |
| diffusion | multilane_chain_avx2 | 768x512 | 180 | 499.120 | 3.046 | 2.095 |
| permutation | block_permutation | 1024x1024 | 30 | 497.839 | 5.289 | 131.861 |
| diffusion | multilane_chain_avx2 | 1024x1024 | 30 | 493.067 | 8.000 | 2.054 |
| diffusion | tree_xor_avx2 | 512x768 | 60 | 471.125 | 4.190 | 1.987 |
| diffusion | tree_xor_avx2 | 512x512 | 30 | 470.202 | 7.667 | 1.955 |

## Best Redesigned Candidates
| scheme | image_size | MBps | keygen_ms | permutation_ms | diffusion_ms |
|---|---|---|---|---|---|
| Checkerboard-CA-ARX | 768x512 | 311.296 | 1.94053 | 0.733323 | 0.93736 |
| Checkerboard-CA-MultilaneTree | 768x512 | 283.066 | 1.8641 | 0.793522 | 1.31383 |
| CA-Feistel-ARX | 512x512 | 92.4434 | 1.60552 | 5.65407 | 0.850495 |
| Affine-CA-PrefixTree | 768x512 | 57.3085 | 1.90883 | 13.2733 | 4.44547 |
| CML-Feistel-Stencil | 1024x1024 | 39.4498 | 42.2717 | 22.31 | 11.4604 |
| Affine-CML-Bitplane | 768x512 | 35.5435 | 15.8712 | 13.5681 | 2.20943 |
| Hamiltonian-Block-Stencil | 512x512 | 11.1046 | 59.0086 | 5.6921 | 2.83596 |

## Aggregated Candidate Statistics
| scheme | image_size | n | MBps_mean | MBps_ci95 | MBps_mean_speedup_vs_baseline |
|---|---|---|---|---|---|
| Checkerboard-CA-ARX | 512x512 | 30 | 267.596 | 3.771 | 7.218 |
| Checkerboard-CA-ARX | 768x512 | 180 | 259.138 | 2.666 | 8.768 |
| Checkerboard-CA-ARX | 512x768 | 60 | 256.482 | 4.195 | 8.731 |
| Checkerboard-CA-MultilaneTree | 512x512 | 30 | 240.316 | 3.634 | 6.482 |
| Checkerboard-CA-ARX | 1024x1024 | 30 | 234.149 | 4.490 | 6.494 |
| Checkerboard-CA-MultilaneTree | 768x512 | 180 | 232.142 | 2.111 | 7.854 |
| Checkerboard-CA-MultilaneTree | 512x768 | 60 | 229.120 | 4.756 | 7.800 |
| Checkerboard-CA-ARX | 2048x2048 | 30 | 216.487 | 3.933 | 6.555 |

## Security Caveats

- Entropy, histogram, NPCR, UACI, and correlation are image-statistical diagnostics, not cryptographic proofs.
- Deterministic stream-XOR variants remain weak under key/nonce reuse and should be reported as negative controls.
- `chaotic_seed_blake3_xor` uses official BLAKE3 as a keyed XOF keystream generator.
- A Q2/Q3-ready manuscript should present the proposed candidates as SIMD-native redesigns with measured tradeoffs, not as standardized secure ciphers.

## Plot Status

Matplotlib unavailable; generated dependency-free PNG bar charts instead (No module named 'matplotlib').
