# Claim Boundary for the Revised Benchmark Paper

## Supported after the full experiment matrix

- Scalar and exact-scan implementations produce identical ciphertext for fixed inputs and parameters.
- Stage costs and end-to-end throughput differ across the measured scalar, AVX2, and NEON builds.
- Sort-based permutation can remain the dominant cost after exact diffusion vectorization.
- Results generalize across the named datasets and the two measured machines only.
- The manuscript is best framed as a benchmark and cryptanalytic sanity-check study, not as a new secure cipher.

## Q2 framing

- Keep the 2022-2026 comparison matrix central.
- Keep the negative-control table central.
- Keep the claim boundary explicit in the abstract, introduction, and conclusion.
- Treat finite-precision results as a lower bound or stress test, not a proof of randomness or security.
- Treat any new family additions as benchmark proxies unless they are fully key-derived, decryptable, and cryptanalytically validated.

## Not supported

- The proxy families reproduce a cited scheme unless every algorithmic detail has been matched and documented.
- Image-domain statistics establish semantic security, integrity, or resistance to cryptanalysis.
- A faster transform is a stronger cipher.
- Results from the Xeon or cloud ARM host predict GPU, embedded, or other CPU performance.
- Deliberate nonce reuse constitutes a break of AES-CTR, ChaCha20, or their underlying primitives.
- The current finite-precision probe establishes an exact orbit bound for all parameterizations.

## Required terminology

- Use **family proxy** for a documented representative implementation.
- Use **exact optimization** only when scalar and vector outputs are byte-identical.
- Use **algorithmic replacement** when transforms have different semantics.
- Use **diagnostic** for entropy, correlation, NPCR, UACI, and histogram measurements.
- Use **misuse test** for repeated-key/nonce experiments.
- Do not use **secure cipher**, **robust against attacks**, or **cryptographic proof** for experimental pipelines.
