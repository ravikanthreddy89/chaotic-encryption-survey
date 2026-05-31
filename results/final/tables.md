# Paper Tables

## Fastest Full Schemes
| cipher | image_size | MBps | keygen_ms | permutation_ms | diffusion_ms | total_ms |
|---|---|---|---|---|---|---|
| chacha20 | 512x512 | 1162.61 | 0 | 0 | 0.479461 | 0.6451 |
| aes_ctr | 1024x1024 | 587.629 | 0 | 0 | 3.57967 | 5.10526 |
| chaotic_seed_blake3_xor | 768x512 | 351.374 | 2.28284 | 0.000499 | 0.576819 | 3.20171 |
| logistic_xor | 512x768 | 146.166 | 4.32254 | 0.000489 | 1.67265 | 7.69672 |
| arnold_xor | 512x768 | 132.805 | 3.36757 | 4.32759 | 0.283378 | 8.4711 |
| tiled_arnold_xor | 512x768 | 104.319 | 3.45471 | 5.13995 | 1.14986 | 10.7842 |
| tent_block_xor | 768x512 | 87.1063 | 10.1903 | 0.000545 | 1.58506 | 12.9153 |
| coupled_lattice_xor | 512x512 | 76.6474 | 9.21374 | 0.000592 | 0.380747 | 9.78506 |
| sine_xor | 768x512 | 25.7743 | 42.5818 | 0.00072 | 0.695851 | 43.6481 |
| logistic_permute_xor | 512x512 | 15.5012 | 2.40489 | 44.3716 | 0.820031 | 48.3834 |
| hamiltonian_lattice_xor | 512x768 | 12.3513 | 89.9461 | 0.000606 | 0.75429 | 91.0835 |

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
| chaotic_seed_blake3_xor | 768x512 | 180 | 299.394 | 3.426 | 2.237 |
| chaotic_seed_blake3_xor | 512x512 | 30 | 296.591 | 9.270 | 2.236 |
| chaotic_seed_blake3_xor | 1024x1024 | 30 | 267.421 | 8.154 | 2.077 |
| aes_ctr | 512x512 | 30 | 251.274 | 4.809 | 1.894 |

## Fastest Replaceable Stages
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
| diffusion | arx_prefix_mod256 | 768x512 | 258.35 | 4.35455 |
| diffusion | global_chain | 512x768 | 252.854 | 4.44921 |

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
| diffusion | multilane_chain_avx2 | 1024x1024 | 30 | 447.583 | 11.638 | 1.977 |
| diffusion | tree_xor_avx2 | 1024x1024 | 30 | 426.760 | 9.649 | 1.885 |
| diffusion | arx_block_diffusion | 512x768 | 60 | 416.576 | 3.652 | 1.815 |
| permutation | block_permutation | 2048x2048 | 30 | 415.410 | 6.720 |  |
| diffusion | arx_block_diffusion | 512x512 | 30 | 411.493 | 9.046 | 1.828 |
| diffusion | arx_block_diffusion | 768x512 | 180 | 411.095 | 2.810 | 1.807 |

## Fastest Candidate Pipelines
| scheme | image_size | MBps | keygen_ms | permutation_ms | diffusion_ms | total_ms |
|---|---|---|---|---|---|---|
| Checkerboard-CA-ARX | 512x768 | 291.875 | 2.0951 | 0.744992 | 1.01149 | 3.85439 |
| Checkerboard-CA-MultilaneTree | 512x768 | 255.128 | 2.04412 | 0.83066 | 1.53189 | 4.40954 |
| CA-Feistel-ARX | 512x512 | 84.432 | 1.81811 | 6.17542 | 0.886048 | 8.88289 |
| Affine-CA-PrefixTree | 768x512 | 51.9139 | 2.21974 | 14.0833 | 5.36412 | 21.6705 |
| CML-Feistel-Stencil | 512x512 | 37.3873 | 11.0965 | 5.92112 | 3.03895 | 20.0603 |
| Affine-CML-Bitplane | 768x512 | 33.9701 | 16.8071 | 14.0928 | 2.21483 | 33.1173 |
| Hamiltonian-Block-Stencil | 512x512 | 10.7496 | 61.1936 | 5.6854 | 2.88849 | 69.7703 |

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
| Checkerboard-CA-ARX | 4096x4096 | 30 | 188.949 | 2.567 | 4.788 |
| Checkerboard-CA-MultilaneTree | 1024x1024 | 30 | 183.371 | 3.251 | 5.397 |
| Checkerboard-CA-MultilaneTree | 2048x2048 | 30 | 156.743 | 3.187 | 5.668 |
| Checkerboard-CA-MultilaneTree | 4096x4096 | 30 | 150.678 | 2.260 | 3.818 |
