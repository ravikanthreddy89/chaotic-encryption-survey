# Paper Tables

## Fastest Full Schemes
| cipher | image_size | MBps | keygen_ms | permutation_ms | diffusion_ms | total_ms |
|---|---|---|---|---|---|---|
| chacha20 | 512x768 | 1009.79 | 0 | 0 | 0.606162 | 1.1141 |
| aes_ctr | 768x512 | 368.742 | 0 | 0 | 2.76077 | 3.05091 |
| chaotic_seed_blake3_xor | 768x512 | 327.04 | 2.57387 | 0.000521 | 0.497473 | 3.43995 |
| logistic_xor | 512x768 | 146.126 | 4.44723 | 0.000726 | 1.7089 | 7.69882 |
| arnold_xor | 768x512 | 124.244 | 3.75197 | 4.54067 | 0.359171 | 9.05474 |
| tiled_arnold_xor | 768x512 | 91.6343 | 3.6839 | 5.72242 | 1.64416 | 12.2771 |
| tent_block_xor | 768x512 | 85.9625 | 10.2472 | 0.000572 | 1.59502 | 13.0871 |
| coupled_lattice_xor | 768x512 | 74.2402 | 14.3736 | 0.000515 | 0.438005 | 15.1535 |
| sine_xor | 768x512 | 24.8562 | 44.0946 | 0.000845 | 0.625934 | 45.2604 |
| logistic_permute_xor | 768x512 | 14.4442 | 3.68746 | 71.5635 | 1.43136 | 77.8857 |
| hamiltonian_lattice_xor | 768x512 | 11.8045 | 93.9966 | 0.000546 | 0.884359 | 95.3025 |

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
| arnold_xor | 512x768 | 6 | 116.018 | 3.580 |  |
| arnold_xor | 768x512 | 18 | 113.364 | 4.295 |  |
| tiled_arnold_xor | 768x512 | 18 | 83.608 | 2.180 |  |
| tent_block_xor | 768x512 | 18 | 78.930 | 1.721 |  |

## Fastest Replaceable Stages
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
| diffusion | global_chain | 768x512 | 237.8 | 4.73086 |
| diffusion | arx_prefix_mod256 | 768x512 | 237.374 | 4.73935 |

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
| diffusion | block_local_chain | 768x512 | 18 | 285.537 | 5.535 |  |
| diffusion | block_local_chain | 512x768 | 6 | 284.675 | 6.207 |  |
| keystream | cellular_automata | 768x512 | 18 | 278.656 | 8.127 |  |
| diffusion | bitplane_diffusion | 768x512 | 18 | 271.094 | 6.537 |  |
| diffusion | reverse_prefix_xor_avx2 | 768x512 | 18 | 270.325 | 6.340 |  |
| diffusion | reverse_prefix_xor_avx2 | 512x768 | 6 | 269.234 | 5.863 |  |

## Fastest Candidate Pipelines
| scheme | image_size | MBps | keygen_ms | permutation_ms | diffusion_ms | total_ms |
|---|---|---|---|---|---|---|
| Checkerboard-CA-ARX | 768x512 | 276.16 | 2.26926 | 0.797989 | 0.978988 | 4.07372 |
| Checkerboard-CA-MultilaneTree | 512x768 | 238.748 | 2.25696 | 0.939947 | 1.51196 | 4.71209 |
| CA-Feistel-ARX | 512x768 | 55.8308 | 2.61843 | 16.1829 | 1.34505 | 20.1502 |
| Affine-CA-PrefixTree | 768x512 | 49.579 | 2.34807 | 14.6816 | 5.65779 | 22.6911 |
| Affine-CML-Bitplane | 768x512 | 33.0773 | 17.3947 | 14.1764 | 2.43682 | 34.0112 |
| CML-Feistel-Stencil | 768x512 | 29.0625 | 17.1134 | 16.7424 | 4.85001 | 38.7097 |
| Hamiltonian-Block-Stencil | 768x512 | 9.80715 | 93.4477 | 16.4825 | 4.77785 | 114.712 |

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
| Affine-CML-Bitplane | 768x512 | 18 | 30.654 | 0.579 |  |
| Affine-CML-Bitplane | 512x768 | 6 | 29.951 | 0.288 |  |
| CML-Feistel-Stencil | 768x512 | 18 | 27.761 | 0.592 |  |
| CML-Feistel-Stencil | 512x768 | 6 | 27.653 | 0.557 |  |
