# Q2/Q3 Applied Performance Summary

This experiment package is positioned as an applied performance study, not as a claim that chaotic schemes beat AES or ChaCha.

## Main Empirical Story

- Traditional chaotic schemes are dominated by implementation-hostile stages: floating-point keystreams, sort-based permutation, and serial diffusion chains.
- SIMD-friendly redesigns move the useful contribution to replaceable stages: cellular automata keystreams, checkerboard/block/Feistel-style permutations, and lane-local/tree/prefix diffusion.
- AES-CTR and ChaCha20 remain the correct cryptographic baselines; the redesigned chaotic candidates are research prototypes for image-domain transformation tradeoffs.

## Best Full Schemes
| cipher | image_size | MBps | total_ms |
|---|---|---|---|
| chacha20 | 512x512 | 1108.24 | 0.676748 |
| aes_ctr | 1024x1024 | 663.674 | 4.52029 |
| chaotic_seed_mix_xor | 512x512 | 465.985 | 1.60949 |
| logistic_xor | 512x512 | 150.72 | 4.97613 |
| arnold_xor | 512x512 | 122.629 | 6.11602 |
| tiled_arnold_xor | 512x512 | 94.8789 | 7.90482 |
| tent_block_xor | 512x512 | 90.0061 | 8.33277 |
| coupled_lattice_xor | 512x512 | 80.5024 | 9.31649 |

## Best Stage Primitives
| category | stage | image_size | MBps | ms |
|---|---|---|---|---|
| diffusion | multilane_chain_avx2 | 512x512 | 602.959 | 1.24386 |
| permutation | block_permutation | 1024x1024 | 582.6 | 5.14933 |
| diffusion | arx_block_diffusion | 512x512 | 539.426 | 1.39037 |
| diffusion | tree_xor_avx2 | 512x512 | 529.242 | 1.41712 |
| diffusion | prefix_xor_avx2 | 512x512 | 413.885 | 1.8121 |
| diffusion | block_local_chain | 512x512 | 360.326 | 2.08145 |
| keystream | cellular_automata | 512x512 | 333.047 | 2.25194 |
| diffusion | parallel_prefix | 512x512 | 322.526 | 2.3254 |
| diffusion | reverse_prefix_xor_avx2 | 512x512 | 309.054 | 2.42676 |
| diffusion | bitplane_diffusion | 1024x1024 | 307.452 | 9.75764 |

## Best Redesigned Candidates
| scheme | image_size | MBps | keygen_ms | permutation_ms | diffusion_ms |
|---|---|---|---|---|---|
| Checkerboard-CA-ARX | 512x512 | 272.737 | 1.42975 | 0.595881 | 0.721897 |
| Checkerboard-CA-MultilaneTree | 512x512 | 247.441 | 1.41137 | 0.634676 | 0.982363 |
| CA-Feistel-ARX | 512x512 | 91.5852 | 1.57605 | 5.8369 | 0.772786 |
| Affine-CA-PrefixTree | 512x512 | 48.6162 | 1.4472 | 10.2796 | 3.6955 |
| CML-Feistel-Stencil | 512x512 | 39.9035 | 10.3645 | 5.40273 | 3.02517 |
| Affine-CML-Bitplane | 512x512 | 33.4702 | 10.955 | 10.0337 | 1.41652 |
| Hamiltonian-Block-Stencil | 512x512 | 11.1153 | 58.9346 | 5.66934 | 2.86776 |

## Security Caveats

- Entropy, histogram, NPCR, UACI, and correlation are image-statistical diagnostics, not cryptographic proofs.
- Deterministic stream-XOR variants remain weak under key/nonce reuse and should be reported as negative controls.
- The current `chaotic_seed_mix_xor` label intentionally avoids claiming official BLAKE3.
- A Q2/Q3-ready manuscript should present the proposed candidates as SIMD-native redesigns with measured tradeoffs, not as standardized secure ciphers.

## Plot Status

Matplotlib unavailable; generated dependency-free PNG bar charts instead (No module named 'matplotlib').
