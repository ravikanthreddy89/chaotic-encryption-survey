# Paper Tables

## Fastest Full Schemes
| cipher | image_size | MBps | keygen_ms | permutation_ms | diffusion_ms | total_ms |
|---|---|---|---|---|---|---|
| chacha20 | 512x512 | 1108.24 | 0 | 0 | 0.453559 | 0.676748 |
| aes_ctr | 1024x1024 | 663.674 | 0 | 0 | 3.30146 | 4.52029 |
| chaotic_seed_mix_xor | 512x512 | 465.985 | 1.11082 | 0.000503 | 0.314881 | 1.60949 |
| logistic_xor | 512x512 | 150.72 | 2.87215 | 0.000587 | 1.08785 | 4.97613 |
| arnold_xor | 512x512 | 122.629 | 2.30284 | 3.3751 | 0.182183 | 6.11602 |
| tiled_arnold_xor | 512x512 | 94.8789 | 2.39818 | 3.88624 | 0.991371 | 7.90482 |
| tent_block_xor | 512x512 | 90.0061 | 6.47428 | 0.000537 | 1.07348 | 8.33277 |
| coupled_lattice_xor | 512x512 | 80.5024 | 8.86574 | 0.000569 | 0.297281 | 9.31649 |
| sine_xor | 512x512 | 26.7938 | 27.416 | 0.000614 | 0.354497 | 27.9915 |
| logistic_permute_xor | 512x512 | 15.5691 | 2.48597 | 44.0634 | 0.849632 | 48.1723 |
| hamiltonian_lattice_xor | 512x512 | 12.3845 | 59.7052 | 0.00054 | 0.571122 | 60.5594 |

## Fastest Replaceable Stages
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
| diffusion | arx_prefix_mod256 | 512x512 | 275.027 | 2.72701 |
| diffusion | global_chain | 1024x1024 | 257.189 | 11.6646 |

## Fastest Candidate Pipelines
| scheme | image_size | MBps | keygen_ms | permutation_ms | diffusion_ms | total_ms |
|---|---|---|---|---|---|---|
| Checkerboard-CA-ARX | 512x512 | 272.737 | 1.42975 | 0.595881 | 0.721897 | 2.7499 |
| Checkerboard-CA-MultilaneTree | 512x512 | 247.441 | 1.41137 | 0.634676 | 0.982363 | 3.03102 |
| CA-Feistel-ARX | 512x512 | 91.5852 | 1.57605 | 5.8369 | 0.772786 | 8.1891 |
| Affine-CA-PrefixTree | 512x512 | 48.6162 | 1.4472 | 10.2796 | 3.6955 | 15.427 |
| CML-Feistel-Stencil | 512x512 | 39.9035 | 10.3645 | 5.40273 | 3.02517 | 18.7953 |
| Affine-CML-Bitplane | 512x512 | 33.4702 | 10.955 | 10.0337 | 1.41652 | 22.408 |
| Hamiltonian-Block-Stencil | 512x512 | 11.1153 | 58.9346 | 5.66934 | 2.86776 | 67.4744 |
