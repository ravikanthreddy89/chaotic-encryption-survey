# Q2/Q3 Applied Performance Summary

This experiment package is positioned as an applied performance study, not as a claim that chaotic schemes beat AES or ChaCha.

## Main Empirical Story

- Traditional chaotic schemes are dominated by implementation-hostile stages: floating-point keystreams, sort-based permutation, and serial diffusion chains.
- SIMD-friendly redesigns move the useful contribution to replaceable stages: cellular automata keystreams, checkerboard/block/Feistel-style permutations, and lane-local/tree/prefix diffusion.
- AES-CTR and ChaCha20 remain the correct cryptographic baselines; the redesigned chaotic candidates are research prototypes for image-domain transformation tradeoffs.

## Best Full Schemes
| cipher | image_size | MBps | total_ms |
|---|---|---|---|
| aes_256_gcm | 512x512 | 1237.41 | 0.606107 |
| chacha20 | 512x512 | 1035.92 | 0.723993 |
| chacha20_poly1305 | 768x512 | 865.564 | 1.29973 |
| aes_ctr | 1920x1080 | 695.916 | 8.5249 |
| chaotic_seed_blake3_xor | 768x512 | 329.345 | 3.41587 |
| logistic_xor | 1280x720 | 142.934 | 18.4471 |
| arnold_xor | 768x512 | 119.484 | 9.4155 |
| tiled_arnold_xor | 768x512 | 88.0926 | 12.7706 |

## Aggregated Full-Scheme Statistics
| cipher | image_size | n | MBps_mean | MBps_ci95 | MBps_mean_speedup_vs_baseline |
|---|---|---|---|---|---|
| aes_256_gcm | 768x512 | 54 | 1057.275 | 21.389 | 8.051 |
| aes_256_gcm | 512x768 | 18 | 1015.009 | 23.670 | 7.744 |
| aes_256_gcm | 512x512 | 9 | 983.701 | 98.605 | 7.588 |
| aes_256_gcm | 1024x1024 | 9 | 898.640 | 45.408 | 6.763 |
| chacha20 | 512x512 | 9 | 879.862 | 67.633 | 6.787 |
| aes_256_gcm | 1280x720 | 36 | 877.766 | 23.374 | 6.916 |
| chacha20 | 768x512 | 54 | 815.438 | 19.638 | 6.210 |
| chacha20 | 1024x1024 | 9 | 812.044 | 44.505 | 6.112 |

## Best Stage Primitives
| category | stage | image_size | MBps | ms |
|---|---|---|---|---|
| diffusion | multilane_chain_avx2 | 512x512 | 522.29 | 1.43599 |
| permutation | block_permutation | 512x512 | 519.857 | 1.4427 |
| diffusion | tree_xor_avx2 | 768x512 | 476.687 | 2.36004 |
| diffusion | arx_block_diffusion | 768x512 | 438.324 | 2.56659 |
| diffusion | prefix_xor_avx2 | 768x512 | 358.047 | 3.14204 |
| diffusion | block_local_chain | 768x512 | 307.469 | 3.6589 |
| keystream | cellular_automata | 512x512 | 298.931 | 2.50894 |
| diffusion | reverse_prefix_xor_avx2 | 768x512 | 295.603 | 3.80578 |
| diffusion | parallel_prefix | 768x512 | 292.052 | 3.85205 |
| diffusion | bitplane_diffusion | 512x512 | 289.656 | 2.58928 |

## Aggregated Stage Statistics
| category | stage | image_size | n | MBps_mean | MBps_ci95 | MBps_mean_speedup_vs_baseline |
|---|---|---|---|---|---|---|
| diffusion | multilane_chain_avx2 | 512x512 | 9 | 483.104 | 12.971 | 2.119 |
| permutation | block_permutation | 512x512 | 9 | 479.017 | 12.638 | 101.154 |
| permutation | block_permutation | 768x512 | 54 | 470.818 | 4.969 | 113.565 |
| permutation | block_permutation | 512x768 | 18 | 470.119 | 8.459 | 111.852 |
| diffusion | multilane_chain_avx2 | 768x512 | 54 | 467.410 | 6.703 | 2.120 |
| diffusion | multilane_chain_avx2 | 512x768 | 18 | 462.671 | 8.237 | 2.083 |
| permutation | block_permutation | 1280x720 | 36 | 462.654 | 5.180 | 132.070 |
| permutation | block_permutation | 1024x1024 | 9 | 460.703 | 7.451 | 132.753 |
| diffusion | multilane_chain_avx2 | 1024x1024 | 9 | 455.692 | 11.161 | 2.034 |
| diffusion | multilane_chain_avx2 | 1280x720 | 36 | 453.438 | 5.529 | 2.046 |

## Best Redesigned Candidates
| scheme | image_size | MBps | keygen_ms | permutation_ms | diffusion_ms |
|---|---|---|---|---|---|
| Checkerboard-CA-ARX | 512x768 | 258.812 | 2.41964 | 0.869903 | 1.0543 |
| Checkerboard-CA-MultilaneTree | 512x512 | 237.366 | 1.50316 | 0.598888 | 1.05467 |
| CA-Feistel-ARX | 512x512 | 82.1744 | 1.81594 | 6.38127 | 0.925618 |
| Affine-CA-PrefixTree | 768x512 | 49.7219 | 2.36844 | 14.4167 | 5.83752 |
| CML-Feistel-Stencil | 512x512 | 36.0439 | 11.4088 | 6.30924 | 3.0865 |
| Affine-CML-Bitplane | 768x512 | 32.8128 | 17.3477 | 14.4406 | 2.49344 |
| Hamiltonian-Block-Stencil | 512x512 | 9.93181 | 65.481 | 6.41194 | 3.61688 |

## Aggregated Candidate Statistics
| scheme | image_size | n | MBps_mean | MBps_ci95 | MBps_mean_speedup_vs_baseline |
|---|---|---|---|---|---|
| Checkerboard-CA-ARX | 512x512 | 9 | 239.167 | 9.399 | 6.913 |
| Checkerboard-CA-ARX | 768x512 | 54 | 233.952 | 4.645 | 8.463 |
| Checkerboard-CA-ARX | 512x768 | 18 | 231.028 | 12.478 | 8.389 |
| Checkerboard-CA-MultilaneTree | 512x512 | 9 | 217.930 | 8.573 | 6.299 |
| Checkerboard-CA-ARX | 1024x1024 | 9 | 213.622 | 3.264 | 6.398 |
| Checkerboard-CA-MultilaneTree | 512x768 | 18 | 212.773 | 6.115 | 7.726 |
| Checkerboard-CA-MultilaneTree | 768x512 | 54 | 212.487 | 3.245 | 7.687 |
| Checkerboard-CA-ARX | 1280x720 | 36 | 207.273 | 4.163 | 6.731 |

## Real-Time Feasibility Summary
| family | name | dataset | image_size | n | ms_mean | ms_p95 | fps_eq_mean | meets_30fps | meets_60fps |
|---|---|---|---|---|---|---|---|---|---|
| full_scheme | aes_256_gcm | kodak_photocd | 512x768 | 18 | 1.111 | 1.173 | 900.131 | yes | yes |
| full_scheme | chacha20 | kodak_photocd | 512x768 | 18 | 1.429 | 1.628 | 699.743 | yes | yes |
| full_scheme | chacha20_poly1305 | kodak_photocd | 512x768 | 18 | 1.661 | 2.142 | 602.095 | yes | yes |
| full_scheme | aes_ctr | kodak_photocd | 512x768 | 18 | 3.495 | 4.072 | 286.145 | yes | yes |
| full_scheme | chaotic_seed_blake3_xor | kodak_photocd | 512x768 | 18 | 3.907 | 4.536 | 255.976 | yes | yes |
| candidate | Checkerboard-CA-ARX | kodak_photocd | 512x768 | 18 | 4.946 | 6.604 | 202.167 | yes | yes |
| candidate | Checkerboard-CA-MultilaneTree | kodak_photocd | 512x768 | 18 | 5.308 | 5.872 | 188.387 | yes | yes |
| full_scheme | logistic_xor | kodak_photocd | 512x768 | 18 | 8.594 | 8.923 | 116.356 | yes | yes |
| full_scheme | arnold_xor | kodak_photocd | 512x768 | 18 | 10.154 | 10.666 | 98.486 | yes | yes |
| full_scheme | tiled_arnold_xor | kodak_photocd | 512x768 | 18 | 14.253 | 15.744 | 70.162 | yes | yes |
| full_scheme | tent_block_xor | kodak_photocd | 512x768 | 18 | 15.072 | 16.799 | 66.348 | yes | yes |
| full_scheme | coupled_lattice_xor | kodak_photocd | 512x768 | 18 | 16.357 | 16.802 | 61.134 | yes | yes |
| candidate | CA-Feistel-ARX | kodak_photocd | 512x768 | 18 | 21.882 | 22.383 | 45.699 | yes | no |
| candidate | Affine-CA-PrefixTree | kodak_photocd | 512x768 | 18 | 25.170 | 26.177 | 39.730 | yes | no |
| candidate | Affine-CML-Bitplane | kodak_photocd | 512x768 | 18 | 37.566 | 38.758 | 26.620 | no | no |
| candidate | CML-Feistel-Stencil | kodak_photocd | 512x768 | 18 | 40.916 | 44.901 | 24.440 | no | no |

## Security Caveats

- Entropy, histogram, NPCR, UACI, and correlation are image-statistical diagnostics, not cryptographic proofs.
- Deterministic stream-XOR variants remain weak under key/nonce reuse and should be reported as negative controls.
- `chaotic_seed_blake3_xor` uses official BLAKE3 as a keyed XOF keystream generator.
- A Q2/Q3-ready manuscript should present the proposed candidates as SIMD-native redesigns with measured tradeoffs, not as standardized secure ciphers.

## Plot Status

Generated paper plots in results/final/figures.
