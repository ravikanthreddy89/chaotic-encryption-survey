# Paper Tables

## Fastest Full Schemes
| cipher | image_size | MBps | keygen_ms | permutation_ms | diffusion_ms | total_ms |
|---|---|---|---|---|---|---|
| aes_256_gcm | 1920x1080 | 1486.99 | 0 | 0 | 3.10759 | 3.98969 |
| aes_ctr | 1920x1080 | 1178.04 | 0 | 0 | 4.14436 | 5.03599 |
| chacha20 | 1920x1080 | 866.806 | 0 | 0 | 5.96137 | 6.84422 |
| chacha20_poly1305 | 1024x1024 | 631.327 | 0 | 0 | 4.33343 | 4.7519 |
| chaotic_seed_blake3_xor | 768x512 | 448.455 | 2.15981 | 6.6e-05 | 0.202063 | 2.50861 |
| logistic_xor | 1920x1080 | 229.187 | 20.9657 | 6.6e-05 | 2.63876 | 25.8854 |
| arnold_xor | 1024x1024 | 161.133 | 10.2265 | 7.38039 | 0.589189 | 18.6181 |
| tent_block_xor | 768x512 | 158.131 | 6.76484 | 4.1e-05 | 0.203934 | 7.11434 |
| tiled_arnold_xor | 512x512 | 143.153 | 2.56627 | 2.4304 | 0.129047 | 5.23916 |
| coupled_lattice_xor | 768x512 | 98.9596 | 11.0109 | 0.000172 | 0.208505 | 11.3683 |
| sine_xor | 1920x1080 | 34.0276 | 172.347 | 9e-05 | 1.19406 | 174.347 |
| logistic_permute_xor | 512x512 | 19.9085 | 2.57852 | 34.7802 | 0.126438 | 37.6723 |

## Aggregated Full-Scheme Statistics
| cipher | image_size | n | MBps_mean | MBps_ci95 | MBps_mean_speedup_vs_baseline |
|---|---|---|---|---|---|
| aes_256_gcm | 1920x1080 | 36 | 1463.159 | 5.774 | 6.460 |
| aes_256_gcm | 1024x1024 | 9 | 1439.100 | 10.827 | 6.934 |
| aes_256_gcm | 1280x720 | 36 | 1404.510 | 23.720 | 7.117 |
| aes_256_gcm | 512x512 | 9 | 1396.403 | 25.941 | 7.727 |
| aes_256_gcm | 512x768 | 18 | 1273.024 | 30.406 | 7.159 |
| aes_256_gcm | 768x512 | 54 | 1260.310 | 28.623 | 7.033 |
| aes_ctr | 1920x1080 | 36 | 1139.297 | 11.552 | 5.030 |
| chacha20 | 1024x1024 | 9 | 849.917 | 4.126 | 4.095 |
| chacha20 | 1920x1080 | 36 | 846.786 | 3.664 | 3.739 |
| chacha20 | 1280x720 | 36 | 829.128 | 10.894 | 4.202 |
| chacha20 | 512x512 | 9 | 829.024 | 15.508 | 4.588 |
| aes_ctr | 1024x1024 | 9 | 809.055 | 14.985 | 3.898 |

## Fastest Replaceable Stages
| category | stage | image_size | MBps | ms |
|---|---|---|---|---|
| permutation | block_permutation | 512x512 | 9437.17 | 0.079473 |
| diffusion | arx_block_diffusion | 768x512 | 2373.13 | 0.474057 |
| diffusion | parallel_prefix | 512x512 | 1032.15 | 0.726642 |
| keystream | cellular_automata | 768x512 | 803.721 | 1.39974 |
| diffusion | reverse_prefix_xor_avx2 | 1280x720 | 727.754 | 3.62309 |
| diffusion | block_local_chain | 1920x1080 | 704.157 | 8.42514 |
| diffusion | global_chain | 768x512 | 589.218 | 1.90931 |
| diffusion | prefix_xor_avx2 | 768x512 | 586.381 | 1.91855 |
| diffusion | bitplane_diffusion | 512x512 | 564.057 | 1.32965 |
| diffusion | multilane_chain_avx2 | 512x512 | 529.507 | 1.41641 |
| permutation | arnold_cat_map | 768x512 | 462.589 | 2.43196 |
| diffusion | arx_prefix_mod256 | 768x512 | 409.417 | 2.74781 |

## Aggregated Stage Statistics
| category | stage | image_size | n | MBps_mean | MBps_ci95 | MBps_mean_speedup_vs_baseline |
|---|---|---|---|---|---|---|
| permutation | block_permutation | 512x512 | 9 | 8406.279 | 608.325 | 1443.457 |
| permutation | block_permutation | 512x768 | 18 | 5270.047 | 349.630 | 982.919 |
| permutation | block_permutation | 768x512 | 54 | 5269.671 | 206.311 | 985.021 |
| permutation | block_permutation | 1280x720 | 36 | 4221.517 | 62.243 | 884.863 |
| permutation | block_permutation | 1024x1024 | 9 | 4171.163 | 55.655 | 880.681 |
| permutation | block_permutation | 2048x2048 | 9 | 3871.874 | 92.134 |  |
| permutation | block_permutation | 1920x1080 | 36 | 3803.921 | 35.743 | 889.619 |
| permutation | block_permutation | 4096x4096 | 9 | 3187.870 | 14.384 |  |
| diffusion | arx_block_diffusion | 512x512 | 9 | 2237.302 | 66.527 | 3.941 |
| diffusion | arx_block_diffusion | 1024x1024 | 9 | 2201.810 | 67.477 | 4.090 |
| diffusion | arx_block_diffusion | 2048x2048 | 9 | 2199.331 | 86.327 | 4.132 |
| diffusion | arx_block_diffusion | 512x768 | 18 | 2156.550 | 73.462 | 3.836 |
| diffusion | arx_block_diffusion | 1280x720 | 36 | 2156.226 | 51.563 | 4.027 |
| diffusion | arx_block_diffusion | 768x512 | 54 | 2155.942 | 47.172 | 3.893 |
| diffusion | arx_block_diffusion | 1920x1080 | 36 | 2150.003 | 44.009 | 3.994 |
| diffusion | arx_block_diffusion | 4096x4096 | 9 | 1948.536 | 129.001 | 3.552 |

## Fastest Candidate Pipelines
| scheme | image_size | MBps | keygen_ms | permutation_ms | diffusion_ms | total_ms |
|---|---|---|---|---|---|---|
| Checkerboard-CA-ARX | 768x512 | 509.908 | 1.4223 | 0.447671 | 0.336042 | 2.20628 |
| CA-Feistel-ARX | 2048x2048 | 118.164 | 16.1581 | 80.4521 | 4.9432 | 101.554 |
| Checkerboard-CA-MultilaneTree | 768x512 | 92.4212 | 1.3585 | 0.444758 | 10.3688 | 12.1725 |
| Affine-CA-PrefixTree | 768x512 | 71.3298 | 1.40748 | 4.48344 | 9.88072 | 15.7718 |
| Affine-CML-Bitplane | 768x512 | 69.0237 | 11.6906 | 4.4713 | 0.136326 | 16.2987 |
| CML-Feistel-Stencil | 1920x1080 | 51.1118 | 61.8101 | 39.0061 | 15.2549 | 116.071 |
| Hamiltonian-Block-Stencil | 1920x1080 | 13.1259 | 397.47 | 39.2048 | 15.3021 | 451.978 |

## Aggregated Candidate Statistics
| scheme | image_size | n | MBps_mean | MBps_ci95 | MBps_mean_speedup_vs_baseline |
|---|---|---|---|---|---|
| Checkerboard-CA-ARX | 1920x1080 | 36 | 494.220 | 4.295 | 9.739 |
| Checkerboard-CA-ARX | 2048x2048 | 9 | 491.921 | 7.189 | 9.857 |
| Checkerboard-CA-ARX | 512x768 | 18 | 491.419 | 6.025 | 12.647 |
| Checkerboard-CA-ARX | 512x512 | 9 | 490.270 | 6.706 | 9.708 |
| Checkerboard-CA-ARX | 1024x1024 | 9 | 485.711 | 12.290 | 9.657 |
| Checkerboard-CA-ARX | 1280x720 | 36 | 485.522 | 6.182 | 10.636 |
| Checkerboard-CA-ARX | 768x512 | 54 | 478.123 | 7.916 | 12.446 |
| Checkerboard-CA-ARX | 4096x4096 | 9 | 473.277 | 9.995 | 4.140 |
| CA-Feistel-ARX | 2048x2048 | 9 | 116.702 | 0.704 | 2.338 |
| CA-Feistel-ARX | 1920x1080 | 36 | 114.557 | 0.919 | 2.257 |
| CA-Feistel-ARX | 4096x4096 | 9 | 114.312 | 0.772 | 1.000 |
| CA-Feistel-ARX | 1024x1024 | 9 | 111.017 | 1.532 | 2.207 |

## Real-Time Feasibility Summary
| family | name | dataset | image_size | n | ms_mean | ms_p95 | fps_eq_mean | meets_30fps | meets_60fps |
|---|---|---|---|---|---|---|---|---|---|
| full_scheme | aes_256_gcm | kodak_photocd | 512x768 | 18 | 0.886 | 0.932 | 1128.787 | yes | yes |
| full_scheme | chacha20 | kodak_photocd | 512x768 | 18 | 1.430 | 1.487 | 699.339 | yes | yes |
| full_scheme | chacha20_poly1305 | kodak_photocd | 512x768 | 18 | 1.928 | 2.003 | 518.743 | yes | yes |
| candidate | Checkerboard-CA-ARX | kodak_photocd | 512x768 | 18 | 2.291 | 2.383 | 436.524 | yes | yes |
| full_scheme | aes_ctr | kodak_photocd | 512x768 | 18 | 2.463 | 2.554 | 406.077 | yes | yes |
| full_scheme | chaotic_seed_blake3_xor | kodak_photocd | 512x768 | 18 | 2.684 | 2.765 | 372.613 | yes | yes |
| full_scheme | logistic_xor | kodak_photocd | 512x768 | 18 | 6.342 | 7.114 | 157.687 | yes | yes |
| full_scheme | arnold_xor | kodak_photocd | 512x768 | 18 | 7.244 | 7.424 | 138.051 | yes | yes |
| full_scheme | tent_block_xor | kodak_photocd | 512x768 | 18 | 7.410 | 7.733 | 134.957 | yes | yes |
| full_scheme | tiled_arnold_xor | kodak_photocd | 512x768 | 18 | 8.134 | 8.405 | 122.937 | yes | yes |
| full_scheme | coupled_lattice_xor | kodak_photocd | 512x768 | 18 | 11.688 | 12.011 | 85.555 | yes | yes |
| candidate | Checkerboard-CA-MultilaneTree | kodak_photocd | 512x768 | 18 | 12.473 | 12.742 | 80.172 | yes | yes |
| candidate | Affine-CA-PrefixTree | kodak_photocd | 512x768 | 18 | 15.959 | 16.228 | 62.659 | yes | yes |
| candidate | Affine-CML-Bitplane | kodak_photocd | 512x768 | 18 | 16.405 | 16.544 | 60.959 | yes | yes |
| candidate | CA-Feistel-ARX | kodak_photocd | 512x768 | 18 | 17.130 | 17.527 | 58.378 | yes | no |
| candidate | CML-Feistel-Stencil | kodak_photocd | 512x768 | 18 | 28.956 | 29.544 | 34.536 | yes | no |
| full_scheme | sine_xor | kodak_photocd | 512x768 | 18 | 33.633 | 34.225 | 29.733 | no | no |
| full_scheme | logistic_permute_xor | kodak_photocd | 512x768 | 18 | 62.865 | 71.197 | 15.907 | no | no |
| full_scheme | hamiltonian_lattice_xor | kodak_photocd | 512x768 | 18 | 76.694 | 77.820 | 13.039 | no | no |
| candidate | Hamiltonian-Block-Stencil | kodak_photocd | 512x768 | 18 | 92.867 | 93.445 | 10.768 | no | no |
