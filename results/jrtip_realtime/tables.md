# Paper Tables

## Fastest Full Schemes
| cipher | image_size | MBps | keygen_ms | permutation_ms | diffusion_ms | total_ms |
|---|---|---|---|---|---|---|
| aes_256_gcm | 512x512 | 1237.41 | 0 | 0 | 0.381768 | 0.606107 |
| chacha20 | 512x512 | 1035.92 | 0 | 0 | 0.458668 | 0.723993 |
| chacha20_poly1305 | 768x512 | 865.564 | 0 | 0 | 0.946271 | 1.29973 |
| aes_ctr | 1920x1080 | 695.916 | 0 | 0 | 5.02164 | 8.5249 |
| chaotic_seed_blake3_xor | 768x512 | 329.345 | 2.5331 | 0.000583 | 0.51336 | 3.41587 |
| logistic_xor | 1280x720 | 142.934 | 10.6695 | 0.000648 | 3.96734 | 18.4471 |
| arnold_xor | 768x512 | 119.484 | 3.80493 | 4.79223 | 0.350239 | 9.4155 |
| tiled_arnold_xor | 768x512 | 88.0926 | 3.77256 | 6.13633 | 1.53888 | 12.7706 |
| tent_block_xor | 512x768 | 83.8258 | 10.522 | 0.000833 | 1.46769 | 13.4207 |
| coupled_lattice_xor | 768x512 | 72.1276 | 14.5189 | 0.000699 | 0.653891 | 15.5974 |
| sine_xor | 768x512 | 24.1185 | 45.5353 | 0.000544 | 0.682625 | 46.6447 |
| logistic_permute_xor | 512x512 | 15.087 | 2.70869 | 45.309 | 0.920838 | 49.7116 |

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
| chacha20 | 1280x720 | 36 | 806.764 | 17.208 | 6.357 |
| chacha20 | 512x768 | 18 | 792.990 | 31.433 | 6.050 |
| aes_256_gcm | 1920x1080 | 36 | 760.835 | 14.550 | 6.200 |
| chacha20_poly1305 | 768x512 | 54 | 730.400 | 15.380 | 5.562 |

## Fastest Replaceable Stages
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
| diffusion | global_chain | 512x512 | 240.85 | 3.11397 |
| diffusion | arx_prefix_mod256 | 768x512 | 237.962 | 4.72764 |

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
| permutation | block_permutation | 1920x1080 | 36 | 449.366 | 5.917 | 144.106 |
| diffusion | tree_xor_avx2 | 512x768 | 18 | 443.893 | 5.142 | 1.998 |
| diffusion | tree_xor_avx2 | 512x512 | 9 | 442.117 | 10.211 | 1.939 |
| diffusion | tree_xor_avx2 | 768x512 | 54 | 441.778 | 4.864 | 2.004 |
| diffusion | tree_xor_avx2 | 1280x720 | 36 | 429.630 | 4.492 | 1.939 |
| diffusion | tree_xor_avx2 | 1024x1024 | 9 | 428.408 | 12.536 | 1.912 |

## Fastest Candidate Pipelines
| scheme | image_size | MBps | keygen_ms | permutation_ms | diffusion_ms | total_ms |
|---|---|---|---|---|---|---|
| Checkerboard-CA-ARX | 512x768 | 258.812 | 2.41964 | 0.869903 | 1.0543 | 4.34679 |
| Checkerboard-CA-MultilaneTree | 512x512 | 237.366 | 1.50316 | 0.598888 | 1.05467 | 3.15967 |
| CA-Feistel-ARX | 512x512 | 82.1744 | 1.81594 | 6.38127 | 0.925618 | 9.12693 |
| Affine-CA-PrefixTree | 768x512 | 49.7219 | 2.36844 | 14.4167 | 5.83752 | 22.6258 |
| CML-Feistel-Stencil | 512x512 | 36.0439 | 11.4088 | 6.30924 | 3.0865 | 20.808 |
| Affine-CML-Bitplane | 768x512 | 32.8128 | 17.3477 | 14.4406 | 2.49344 | 34.2854 |
| Hamiltonian-Block-Stencil | 512x512 | 9.93181 | 65.481 | 6.41194 | 3.61688 | 75.5149 |

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
| Checkerboard-CA-ARX | 1920x1080 | 36 | 204.994 | 2.848 | 6.261 |
| Checkerboard-CA-ARX | 2048x2048 | 9 | 199.911 | 4.955 | 6.724 |
| Checkerboard-CA-ARX | 4096x4096 | 9 | 187.085 | 7.433 | 4.658 |
| Checkerboard-CA-MultilaneTree | 1280x720 | 36 | 178.981 | 3.861 | 5.813 |

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
| full_scheme | sine_xor | kodak_photocd | 512x768 | 18 | 48.810 | 51.002 | 20.488 | no | no |
| full_scheme | logistic_permute_xor | kodak_photocd | 512x768 | 18 | 82.896 | 86.579 | 12.063 | no | no |
| full_scheme | hamiltonian_lattice_xor | kodak_photocd | 512x768 | 18 | 102.356 | 106.612 | 9.770 | no | no |
| candidate | Hamiltonian-Block-Stencil | kodak_photocd | 512x768 | 18 | 124.871 | 133.303 | 8.008 | no | no |
