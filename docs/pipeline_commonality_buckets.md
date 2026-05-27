# Common Pipeline And Buckets

## Common pipeline across most schemes

1. **Key material setup**
- Hash or direct key-to-parameter mapping.
- Outputs chaotic initial states and control parameters.

2. **Chaotic sequence generation**
- Logistic/sine/tent/hyperchaotic maps.
- Generates floating sequence, integer keystream, or both.

3. **Permutation (confusion)**
- Map-based index transform (Arnold/Baker/affine).
- Or sort-based index permutation from chaotic values.

4. **Diffusion (value mixing)**
- Usually chained recurrence (`out[i] = in[i] xor/add k[i] xor/add out[i-1]`).
- Sometimes multi-pass, channel-wise, or bit-plane-wise.

5. **Optional symbolic layer**
- DNA encoding, CA transitions, substitution table/rule switching.

6. **Inverse pipeline for decryption**
- Reverse diffusion then inverse permutation (or exact inverse ordering of stages).

## Buckets for implementation

### Bucket A: Key generation
- A1: static-key to seeds (fast baseline)
- A2: plaintext-dependent seeds (extra coupling, harder reproducibility)

### Bucket B: Permutation
- B1: map-based modular permutation (`O(N)`)
- B2: sort-based permutation (`O(N log N)`)
- B3: bit-plane/block permutation

### Bucket C: Diffusion
- C1: scalar chain recurrence
- C2: exact prefix-scan equivalent (SIMD-friendly)
- C3: block/tile multi-pass recurrence

### Bucket D: Symbolic operations
- D1: DNA coding rules
- D2: cellular automata symbol transitions

## SIMD/AVX feasibility by bucket

- **A (keygen)**: moderate. Map generation may be partially vectorized if independent lanes are used.
- **B1 (map permutation)**: moderate. Good for pointer arithmetic + gather/scatter aware loops.
- **B2 (sort permutation)**: low-to-moderate. Sort often memory-bound; radix/parallel sort can help.
- **C (diffusion)**: high. Convert recurrence to exact scan form and vectorize chunk operations.
- **D (symbolic)**: low-to-moderate. Branch-heavy logic often limits SIMD efficiency.

## Suggested benchmark matrix for publication

1. **Algorithms**: one representative from B1, B2, B3+D.
2. **Kernels**: scalar baseline, SSE2, AVX2, optional AVX-512.
3. **Parallelism**: single-thread first, then OpenMP scaling.
4. **Image sizes**: 256, 512, 1024, 2048, 4096.
5. **Metrics**: MB/s, cycles/byte, cache miss rate, entropy/NPCR/UACI, lossless recovery.
6. **Reproducibility**: fixed seeds, pinned cores, fixed compiler flags, CSV artifacts.
