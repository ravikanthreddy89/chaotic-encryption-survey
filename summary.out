# Q2/Q3 Applied Performance Summary

This experiment package is positioned as an applied performance study, not as a claim that chaotic schemes beat AES or ChaCha.

## Main Empirical Story

- Traditional chaotic schemes are dominated by implementation-hostile stages: floating-point keystreams, sort-based permutation, and serial diffusion chains.
- SIMD-friendly redesigns move the useful contribution to replaceable stages: cellular automata keystreams, checkerboard/block/Feistel-style permutations, and lane-local/tree/prefix diffusion.
- AES-CTR and ChaCha20 remain the correct cryptographic baselines; the redesigned chaotic candidates are research prototypes for image-domain transformation tradeoffs.

## Best Full Schemes
| cipher | image_size | MBps | total_ms |
|---|---|---|---|
| chacha20 | 512x512 | 1162.61 | 0.6451 |
| aes_ctr | 1024x1024 | 587.629 | 5.10526 |
| chaotic_seed_blake3_xor | 768x512 | 351.374 | 3.20171 |
| logistic_xor | 512x768 | 146.166 | 7.69672 |
| arnold_xor | 512x768 | 132.805 | 8.4711 |
| tiled_arnold_xor | 512x768 | 104.319 | 10.7842 |
| tent_block_xor | 768x512 | 87.1063 | 12.9153 |
| coupled_lattice_xor | 512x512 | 76.6474 | 9.78506 |

## Aggregated Full-Scheme Statistics
| cipher | image_size | n | MBps_mean | MBps_ci95 | MBps_mean_speedup_vs_baseline |
|---|---|---|---|---|---|
| chacha20 | 512x512 | 30 | 945.506 | 46.968 | 7.127 |
| chacha20 | 512x768 | 60 | 857.771 | 19.283 | 6.460 |
| chacha20 | 768x512 | 180 | 845.183 | 12.099 | 6.316 |
| chacha20 | 1024x1024 | 30 | 768.588 | 25.216 | 5.969 |
| aes_ctr | 1024x1024 | 30 | 533.687 | 15.948 | 4.145 |
| aes_ctr | 512x768 | 60 | 338.504 | 4.689 | 2.549 |
| aes_ctr | 768x512 | 180 | 337.636 | 3.266 | 2.523 |
| chaotic_seed_blake3_xor | 512x768 | 60 | 300.865 | 6.443 | 2.266 |

## Best Stage Primitives
| category | stage | image_size | MBps | ms |
|---|---|---|---|---|
| permutation | block_permutation | 512x768 | 544.276 | 2.06697 |
| diffusion | multilane_chain_avx2 | 512x512 | 537.423 | 1.39555 |
| diffusion | tree_xor_avx2 | 768x512 | 527.636 | 2.13215 |
| diffusion | arx_block_diffusion | 512x512 | 461.291 | 1.62587 |
| diffusion | prefix_xor_avx2 | 512x768 | 391.845 | 2.87103 |
| diffusion | block_local_chain | 768x512 | 324.369 | 3.46828 |
| keystream | cellular_automata | 512x768 | 313.028 | 3.59392 |
| diffusion | parallel_prefix | 768x512 | 310.848 | 3.61913 |
| diffusion | bitplane_diffusion | 512x768 | 309.734 | 3.63215 |
| diffusion | reverse_prefix_xor_avx2 | 768x512 | 301.241 | 3.73455 |

## Aggregated Stage Statistics
| category | stage | image_size | n | MBps_mean | MBps_ci95 | MBps_mean_speedup_vs_baseline |
|---|---|---|---|---|---|---|
| permutation | block_permutation | 512x512 | 30 | 488.197 | 10.410 | 105.859 |
| permutation | block_permutation | 512x768 | 60 | 484.649 | 5.764 | 111.033 |
| diffusion | multilane_chain_avx2 | 768x512 | 180 | 483.497 | 2.725 | 2.125 |
| permutation | block_permutation | 768x512 | 180 | 480.218 | 3.135 | 110.670 |
| diffusion | multilane_chain_avx2 | 512x512 | 30 | 479.517 | 9.062 | 2.130 |
| diffusion | multilane_chain_avx2 | 512x768 | 60 | 478.641 | 5.010 | 2.085 |
| permutation | block_permutation | 1024x1024 | 30 | 465.646 | 8.393 | 134.984 |
| diffusion | tree_xor_avx2 | 512x768 | 60 | 456.079 | 4.227 | 1.987 |
| diffusion | tree_xor_avx2 | 768x512 | 180 | 454.466 | 2.819 | 1.997 |
| diffusion | tree_xor_avx2 | 512x512 | 30 | 452.561 | 8.089 | 2.010 |

## Best Redesigned Candidates
| scheme | image_size | MBps | keygen_ms | permutation_ms | diffusion_ms |
|---|---|---|---|---|---|
| Checkerboard-CA-ARX | 512x768 | 291.875 | 2.0951 | 0.744992 | 1.01149 |
| Checkerboard-CA-MultilaneTree | 512x768 | 255.128 | 2.04412 | 0.83066 | 1.53189 |
| CA-Feistel-ARX | 512x512 | 84.432 | 1.81811 | 6.17542 | 0.886048 |
| Affine-CA-PrefixTree | 768x512 | 51.9139 | 2.21974 | 14.0833 | 5.36412 |
| CML-Feistel-Stencil | 512x512 | 37.3873 | 11.0965 | 5.92112 | 3.03895 |
| Affine-CML-Bitplane | 768x512 | 33.9701 | 16.8071 | 14.0928 | 2.21483 |
| Hamiltonian-Block-Stencil | 512x512 | 10.7496 | 61.1936 | 5.6854 | 2.88849 |

## Aggregated Candidate Statistics
| scheme | image_size | n | MBps_mean | MBps_ci95 | MBps_mean_speedup_vs_baseline |
|---|---|---|---|---|---|
| Checkerboard-CA-ARX | 512x768 | 60 | 251.141 | 5.071 | 8.793 |
| Checkerboard-CA-ARX | 512x512 | 30 | 249.045 | 6.148 | 7.117 |
| Checkerboard-CA-ARX | 768x512 | 180 | 245.658 | 3.054 | 8.665 |
| Checkerboard-CA-MultilaneTree | 512x512 | 30 | 226.727 | 4.922 | 6.479 |
| Checkerboard-CA-MultilaneTree | 512x768 | 60 | 224.509 | 4.140 | 7.861 |
| Checkerboard-CA-MultilaneTree | 768x512 | 180 | 220.599 | 2.585 | 7.781 |
| Checkerboard-CA-ARX | 1024x1024 | 30 | 211.652 | 5.127 | 6.229 |
| Checkerboard-CA-ARX | 2048x2048 | 30 | 196.312 | 3.182 | 7.099 |

## Security Caveats

- Entropy, histogram, NPCR, UACI, and correlation are image-statistical diagnostics, not cryptographic proofs.
- Deterministic stream-XOR variants remain weak under key/nonce reuse and should be reported as negative controls.
- `chaotic_seed_blake3_xor` uses official BLAKE3 as a keyed XOF keystream generator.
- A Q2/Q3-ready manuscript should present the proposed candidates as SIMD-native redesigns with measured tradeoffs, not as standardized secure ciphers.

## Plot Status

Matplotlib unavailable; generated dependency-free PNG bar charts instead (No module named 'matplotlib').
