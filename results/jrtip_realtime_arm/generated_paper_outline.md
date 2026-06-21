# Stage-Level SIMD Redesign and Benchmarking of Chaotic Image Encryption Pipelines

## Abstract

Chaotic image encryption papers frequently report image-domain statistical metrics while giving limited attention to implementation bottlenecks. This work decomposes representative chaotic image encryption pipelines into keystream generation, permutation, and diffusion stages, then benchmarks both traditional and SIMD-native alternatives. The results show that sort-based permutation, floating-point scalar maps, and global feedback diffusion are the main performance barriers. Cellular automata, coupled-map lattices, affine/checkerboard/block permutations, and prefix/tree/lane-local diffusion reduce this gap while retaining useful image-domain statistical behavior. AES-CTR and ChaCha20 are included as cryptographic baselines and remain the appropriate reference for general-purpose confidentiality.

## Contribution

1. A C++17/OpenCV/OpenSSL benchmark suite with common cipher interfaces and stage-level timing.
2. A reproducible matrix over synthetic controls, the Kodak PhotoCD real-image set, image sizes, repetitions, legacy chaotic schemes, redesigned candidates, and AES/ChaCha baselines.
3. An applied analysis showing which chaotic-image stages are implementation-hostile and which replacements are SIMD-friendly.
4. An explicit security discussion separating image-statistical metrics from cryptographic security claims.

## Experimental Methodology

The benchmark separates full ciphers from replaceable stages. Full schemes measure key generation, permutation, diffusion, and total runtime. Stage experiments isolate keystream generation, permutation, and diffusion primitives so bottlenecks can be attributed directly. The final matrix includes deterministic synthetic images for controlled scaling and the Kodak PhotoCD image set for real natural-image coverage. Results are aggregated with mean, median, standard deviation, minimum, maximum, and 95% confidence intervals.

## Datasets

- Synthetic controls: gradient, texture, and noise images generated deterministically at 512, 1024, 2048, and 4096 square resolutions.
- Real images: 24 Kodak PhotoCD natural images downloaded from https://r0k.us/graphics/kodak/.

## Baselines

AES-CTR and ChaCha20 are included through OpenSSL EVP as cryptographic baselines. They are not expected to be beaten as general-purpose ciphers. Their purpose is to keep the security and performance discussion anchored to standard cryptographic practice.

## Proposed Direction

The strongest proposed candidates are Checkerboard-CA-ARX and Checkerboard-CA-MultilaneTree. They replace sort-based permutation and serial diffusion with parallel checkerboard swaps, cellular automata keystreams, ARX block diffusion, multi-lane chaining, and tree diffusion.

## Security Position

The image metrics in this work are diagnostic rather than proof of cryptographic security. Entropy, histogram uniformity, NPCR, UACI, and adjacent-pixel correlation are useful for image-domain behavior, but known-plaintext and chosen-plaintext tests remain necessary negative controls. Deterministic stream-XOR variants are explicitly treated as weak under key/nonce reuse.

## Results

See `results/final/tables.md`, `results/final/performance_summary.md`, `results/final/real_time_metrics.csv`, and `results/final/figures/`.

## Limitations

The proposed candidates are research prototypes. They require deeper cryptanalysis before any security deployment. The strongest defensible claim is performance-oriented stage redesign, not superiority to AES or ChaCha.
