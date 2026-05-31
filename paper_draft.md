# Stage-Level SIMD Redesign and Benchmarking of Chaotic Image Encryption Pipelines

## Abstract

Chaotic image encryption papers frequently report image-domain statistical metrics while giving limited attention to implementation bottlenecks. This work decomposes representative chaotic image encryption pipelines into keystream generation, permutation, and diffusion stages, then benchmarks both traditional and SIMD-native alternatives. The results show that sort-based permutation, floating-point scalar maps, and global feedback diffusion are the main performance barriers. Cellular automata, coupled-map lattices, affine/checkerboard/block permutations, and prefix/tree/lane-local diffusion reduce this gap while retaining useful image-domain statistical behavior. AES-CTR and ChaCha20 are included as cryptographic baselines and remain the appropriate reference for general-purpose confidentiality.

## Contribution

1. A C++17/OpenCV/OpenSSL benchmark suite with common cipher interfaces and stage-level timing.
2. A reproducible matrix over image types, image sizes, repetitions, legacy chaotic schemes, redesigned candidates, and AES/ChaCha baselines.
3. An applied analysis showing which chaotic-image stages are implementation-hostile and which replacements are SIMD-friendly.
4. An explicit security discussion separating image-statistical metrics from cryptographic security claims.

## Results

See `results/final/tables.md`, `results/final/performance_summary.md`, and `results/final/figures/`.

## Limitations

The proposed candidates are research prototypes. They require deeper cryptanalysis before any security deployment. The strongest defensible claim is performance-oriented stage redesign, not superiority to AES or ChaCha.
