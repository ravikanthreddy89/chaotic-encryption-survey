# Q2/Q3 Applied Performance Summary

This experiment package is positioned as an applied performance study, not as a claim that chaotic schemes beat AES or ChaCha.

## Main Empirical Story

- Traditional chaotic schemes are dominated by implementation-hostile stages: floating-point keystreams, sort-based permutation, and serial diffusion chains.
- SIMD-friendly redesigns move the useful contribution to replaceable stages: cellular automata keystreams, checkerboard/block/Feistel-style permutations, and lane-local/tree/prefix diffusion.
- AES-CTR and ChaCha20 remain the correct cryptographic baselines; the redesigned chaotic candidates are research prototypes for image-domain transformation tradeoffs.

## Best Full Schemes
| cipher | image_size | MBps | total_ms |
|---|---|---|---|
| chacha20 | 64x64 | 524.212 | 0.022355 |
| chaotic_seed_mix_xor | 64x64 | 200.064 | 0.058575 |
| arnold_xor | 64x64 | 101.451 | 0.115511 |
| logistic_xor | 64x64 | 101.037 | 0.115985 |
| tent_block_xor | 64x64 | 90.675 | 0.129239 |
| tiled_arnold_xor | 64x64 | 62.3073 | 0.18808 |
| coupled_lattice_xor | 64x64 | 59.3721 | 0.197378 |
| logistic_permute_xor | 64x64 | 25.0734 | 0.467377 |

## Best Stage Primitives
| category | stage | image_size | MBps | ms |
|---|---|---|---|---|
| diffusion | multilane_chain_avx2 | 64x64 | 545.642 | 0.021477 |
| permutation | block_permutation | 64x64 | 532.767 | 0.021996 |
| diffusion | tree_xor_avx2 | 64x64 | 511.803 | 0.022897 |
| diffusion | arx_block_diffusion | 64x64 | 454.391 | 0.02579 |
| diffusion | prefix_xor_avx2 | 64x64 | 391.042 | 0.029968 |
| diffusion | parallel_prefix | 64x64 | 388.927 | 0.030131 |
| diffusion | block_local_chain | 64x64 | 324.484 | 0.036115 |
| keystream | cellular_automata | 64x64 | 324.054 | 0.036163 |
| diffusion | reverse_prefix_xor_avx2 | 64x64 | 314.251 | 0.037291 |
| diffusion | bitplane_diffusion | 64x64 | 306.87 | 0.038188 |

## Best Redesigned Candidates
| scheme | image_size | MBps | keygen_ms | permutation_ms | diffusion_ms |
|---|---|---|---|---|---|
| Checkerboard-CA-MultilaneTree | 64x64 | 442.835 | 0.017084 | 0.002442 | 0.005193 |
| Checkerboard-CA-ARX | 64x64 | 306.613 | 0.023611 | 0.003829 | 0.008519 |
| CA-Feistel-ARX | 64x64 | 92.8387 | 0.023731 | 0.086891 | 0.012774 |
| Affine-CA-PrefixTree | 64x64 | 54.9109 | 0.017026 | 0.143513 | 0.051066 |
| CML-Feistel-Stencil | 64x64 | 39.3142 | 0.168082 | 0.075513 | 0.052502 |
| Affine-CML-Bitplane | 64x64 | 33.0032 | 0.169091 | 0.15713 | 0.02644 |
| Hamiltonian-Block-Stencil | 64x64 | 5.88822 | 1.79007 | 0.136078 | 0.061194 |

## Security Caveats

- Entropy, histogram, NPCR, UACI, and correlation are image-statistical diagnostics, not cryptographic proofs.
- Deterministic stream-XOR variants remain weak under key/nonce reuse and should be reported as negative controls.
- The current `chaotic_seed_mix_xor` label intentionally avoids claiming official BLAKE3.
- A Q2/Q3-ready manuscript should present the proposed candidates as SIMD-native redesigns with measured tradeoffs, not as standardized secure ciphers.

## Plot Status

Matplotlib unavailable; skipped generated paper plots (No module named 'matplotlib').
