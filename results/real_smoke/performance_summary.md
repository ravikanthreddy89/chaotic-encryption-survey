# Q2/Q3 Applied Performance Summary

This experiment package is positioned as an applied performance study, not as a claim that chaotic schemes beat AES or ChaCha.

## Main Empirical Story

- Traditional chaotic schemes are dominated by implementation-hostile stages: floating-point keystreams, sort-based permutation, and serial diffusion chains.
- SIMD-friendly redesigns move the useful contribution to replaceable stages: cellular automata keystreams, checkerboard/block/Feistel-style permutations, and lane-local/tree/prefix diffusion.
- AES-CTR and ChaCha20 remain the correct cryptographic baselines; the redesigned chaotic candidates are research prototypes for image-domain transformation tradeoffs.

## Best Full Schemes
| cipher | image_size | MBps | total_ms |
|---|---|---|---|
| chacha20 | 512x768 | 1009.79 | 1.1141 |
| aes_ctr | 768x512 | 368.742 | 3.05091 |
| chaotic_seed_blake3_xor | 768x512 | 327.04 | 3.43995 |
| logistic_xor | 512x768 | 146.126 | 7.69882 |
| arnold_xor | 768x512 | 124.244 | 9.05474 |
| tiled_arnold_xor | 768x512 | 91.6343 | 12.2771 |
| tent_block_xor | 768x512 | 85.9625 | 13.0871 |
| coupled_lattice_xor | 768x512 | 74.2402 | 15.1535 |

## Aggregated Full-Scheme Statistics
| cipher | image_size | n | MBps_mean | MBps_ci95 | MBps_speedup_vs_baseline |
|---|---|---|---|---|---|
| chacha20 | 768x512 | 18 | 840.245 | 44.425 |  |
| chacha20 | 512x768 | 6 | 802.642 | 110.169 |  |
| aes_ctr | 768x512 | 18 | 335.627 | 11.796 |  |
| aes_ctr | 512x768 | 6 | 305.969 | 29.925 |  |
| chaotic_seed_blake3_xor | 768x512 | 18 | 298.543 | 8.555 |  |
| chaotic_seed_blake3_xor | 512x768 | 6 | 283.792 | 19.262 |  |
| logistic_xor | 512x768 | 6 | 134.702 | 5.474 |  |
| logistic_xor | 768x512 | 18 | 132.168 | 2.784 |  |

## Best Stage Primitives
| category | stage | image_size | MBps | ms |
|---|---|---|---|---|
| permutation | block_permutation | 768x512 | 517.092 | 2.17563 |
| diffusion | multilane_chain_avx2 | 768x512 | 510.164 | 2.20517 |
| diffusion | tree_xor_avx2 | 768x512 | 494.746 | 2.2739 |
| diffusion | arx_block_diffusion | 768x512 | 442.501 | 2.54237 |
| diffusion | prefix_xor_avx2 | 768x512 | 372.919 | 3.01674 |
| keystream | cellular_automata | 768x512 | 306.225 | 3.67377 |
| diffusion | block_local_chain | 768x512 | 300.177 | 3.74779 |
| diffusion | bitplane_diffusion | 768x512 | 296.607 | 3.7929 |
| diffusion | reverse_prefix_xor_avx2 | 768x512 | 293.608 | 3.83164 |
| diffusion | parallel_prefix | 768x512 | 290.933 | 3.86687 |

## Aggregated Stage Statistics
| category | stage | image_size | n | MBps_mean | MBps_ci95 | MBps_speedup_vs_baseline |
|---|---|---|---|---|---|---|
| diffusion | multilane_chain_avx2 | 768x512 | 18 | 474.161 | 9.372 |  |
| permutation | block_permutation | 768x512 | 18 | 468.131 | 12.693 |  |
| permutation | block_permutation | 512x768 | 6 | 463.373 | 12.208 |  |
| diffusion | multilane_chain_avx2 | 512x768 | 6 | 462.785 | 21.802 |  |
| diffusion | tree_xor_avx2 | 768x512 | 18 | 444.348 | 17.070 |  |
| diffusion | tree_xor_avx2 | 512x768 | 6 | 433.580 | 15.199 |  |
| diffusion | arx_block_diffusion | 512x768 | 6 | 401.927 | 10.114 |  |
| diffusion | arx_block_diffusion | 768x512 | 18 | 400.694 | 9.415 |  |
| diffusion | prefix_xor_avx2 | 768x512 | 18 | 346.685 | 5.383 |  |
| diffusion | prefix_xor_avx2 | 512x768 | 6 | 337.191 | 12.197 |  |

## Best Redesigned Candidates
| scheme | image_size | MBps | keygen_ms | permutation_ms | diffusion_ms |
|---|---|---|---|---|---|
| Checkerboard-CA-ARX | 768x512 | 276.16 | 2.26926 | 0.797989 | 0.978988 |
| Checkerboard-CA-MultilaneTree | 512x768 | 238.748 | 2.25696 | 0.939947 | 1.51196 |
| CA-Feistel-ARX | 512x768 | 55.8308 | 2.61843 | 16.1829 | 1.34505 |
| Affine-CA-PrefixTree | 768x512 | 49.579 | 2.34807 | 14.6816 | 5.65779 |
| Affine-CML-Bitplane | 768x512 | 33.0773 | 17.3947 | 14.1764 | 2.43682 |
| CML-Feistel-Stencil | 768x512 | 29.0625 | 17.1134 | 16.7424 | 4.85001 |
| Hamiltonian-Block-Stencil | 768x512 | 9.80715 | 93.4477 | 16.4825 | 4.77785 |

## Aggregated Candidate Statistics
| scheme | image_size | n | MBps_mean | MBps_ci95 | MBps_speedup_vs_baseline |
|---|---|---|---|---|---|
| Checkerboard-CA-ARX | 768x512 | 18 | 246.099 | 9.560 |  |
| Checkerboard-CA-ARX | 512x768 | 6 | 244.812 | 10.616 |  |
| Checkerboard-CA-MultilaneTree | 768x512 | 18 | 215.705 | 6.449 |  |
| Checkerboard-CA-MultilaneTree | 512x768 | 6 | 208.472 | 17.256 |  |
| CA-Feistel-ARX | 768x512 | 18 | 51.936 | 0.732 |  |
| CA-Feistel-ARX | 512x768 | 6 | 51.144 | 2.620 |  |
| Affine-CA-PrefixTree | 768x512 | 18 | 45.239 | 0.676 |  |
| Affine-CA-PrefixTree | 512x768 | 6 | 44.344 | 1.753 |  |

## Security Caveats

- Entropy, histogram, NPCR, UACI, and correlation are image-statistical diagnostics, not cryptographic proofs.
- Deterministic stream-XOR variants remain weak under key/nonce reuse and should be reported as negative controls.
- `chaotic_seed_blake3_xor` uses official BLAKE3 as a keyed XOF keystream generator.
- A Q2/Q3-ready manuscript should present the proposed candidates as SIMD-native redesigns with measured tradeoffs, not as standardized secure ciphers.

## Plot Status

Matplotlib unavailable; generated dependency-free PNG bar charts instead (No module named 'matplotlib').
