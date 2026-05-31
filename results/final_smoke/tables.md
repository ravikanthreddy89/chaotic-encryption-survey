# Paper Tables

## Fastest Full Schemes
| cipher | image_size | MBps | keygen_ms | permutation_ms | diffusion_ms | total_ms |
|---|---|---|---|---|---|---|
| chacha20 | 64x64 | 524.212 | 0 | 0 | 0.018382 | 0.022355 |
| chaotic_seed_mix_xor | 64x64 | 200.064 | 0.04835 | 0.000485 | 0.003339 | 0.058575 |
| arnold_xor | 64x64 | 101.451 | 0.05372 | 0.054127 | 0.002573 | 0.115511 |
| logistic_xor | 64x64 | 101.037 | 0.066899 | 0.000463 | 0.029569 | 0.115985 |
| tent_block_xor | 64x64 | 90.675 | 0.120918 | 0.000529 | 0.002689 | 0.129239 |
| tiled_arnold_xor | 64x64 | 62.3073 | 0.120729 | 0.057113 | 0.004105 | 0.18808 |
| coupled_lattice_xor | 64x64 | 59.3721 | 0.188377 | 0.000485 | 0.003286 | 0.197378 |
| logistic_permute_xor | 64x64 | 25.0734 | 0.06627 | 0.392291 | 0.002332 | 0.467377 |
| sine_xor | 64x64 | 20.6855 | 0.552172 | 0.000488 | 0.006928 | 0.566519 |
| hamiltonian_lattice_xor | 64x64 | 11.4381 | 1.01459 | 0.000583 | 0.00313 | 1.02453 |
| aes_ctr | 64x64 | 5.01695 | 0 | 0 | 2.33057 | 2.33583 |

## Fastest Replaceable Stages
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
| diffusion | global_chain | 64x64 | 304.541 | 0.03848 |
| diffusion | arx_prefix_mod256 | 64x64 | 257.578 | 0.045496 |

## Fastest Candidate Pipelines
| scheme | image_size | MBps | keygen_ms | permutation_ms | diffusion_ms | total_ms |
|---|---|---|---|---|---|---|
| Checkerboard-CA-MultilaneTree | 64x64 | 442.835 | 0.017084 | 0.002442 | 0.005193 | 0.026463 |
| Checkerboard-CA-ARX | 64x64 | 306.613 | 0.023611 | 0.003829 | 0.008519 | 0.03822 |
| CA-Feistel-ARX | 64x64 | 92.8387 | 0.023731 | 0.086891 | 0.012774 | 0.126227 |
| Affine-CA-PrefixTree | 64x64 | 54.9109 | 0.017026 | 0.143513 | 0.051066 | 0.213414 |
| CML-Feistel-Stencil | 64x64 | 39.3142 | 0.168082 | 0.075513 | 0.052502 | 0.298079 |
| Affine-CML-Bitplane | 64x64 | 33.0032 | 0.169091 | 0.15713 | 0.02644 | 0.355079 |
| Hamiltonian-Block-Stencil | 64x64 | 5.88822 | 1.79007 | 0.136078 | 0.061194 | 1.9902 |
