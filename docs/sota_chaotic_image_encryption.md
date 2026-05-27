# State Of The Art Chaotic Image Encryption (2026 Snapshot)

## Scope
This is a practical research shortlist for building a SIMD/AVX reference implementation. It targets algorithm *families* that recur across recent papers, not an exhaustive bibliography.

## Important caveat
Most chaos-based image ciphers are evaluated with statistical metrics (entropy, NPCR, UACI, correlation) and not always with modern cryptographic proofs or strong adversarial models. For production security, AES/ChaCha remain the default baseline.

## Representative scheme families

1. **Fridrich permutation-diffusion baseline (historical anchor)**
- Core pattern: repeated permutation + chained diffusion.
- Why included: foundational architecture used by many later variants.
- Sources:
  - https://www.mdpi.com/1099-4300/20/4/282
  - https://arxiv.org/abs/1503.06638

2. **Bit-level permutation + logistic/Arnold families**
- Core pattern: bit-plane or bit-level permutation, then XOR/add diffusion.
- Why included: common in "fast" designs and easy to benchmark.
- Sources:
  - https://www.sciencedirect.com/science/article/pii/S0020025510005542
  - https://en.wikipedia.org/wiki/Arnold%27s_cat_map

3. **Sort-based chaotic permutation families (YeHuang/LSCM-like)**
- Core pattern: generate chaotic sequence, argsort indices, permute, then chain diffusion.
- Why included: strong diffusion/permutation behavior but expensive sort stage.
- Sources:
  - https://arxiv.org/abs/1503.06638
  - https://www.mdpi.com/1099-4300/20/4/282

4. **DNA / symbolic-operation hybrids**
- Core pattern: permutation-diffusion plus DNA coding/mutation/substitution layers.
- Why included: still common in recent publications and hard to optimize due to branching rules.
- Sources:
  - https://www.frontiersin.org/articles/10.3389/fphy.2022.911156/full
  - https://www.sciencedirect.com/science/article/pii/S0016003223006567

5. **Parallel-aware bit-plane cross permutation-diffusion**
- Core pattern: cross bit-plane permutation and staged diffusion, explicitly parallelized.
- Why included: best candidate for SIMD + multicore speedups.
- Source:
  - https://www.sciencedirect.com/science/article/abs/pii/S0045790625005129

6. **Higher-dimensional chaos + cellular automata hybrids (2024-2025 trend)**
- Core pattern: 4D hyperchaotic keystreams with symbolic/CA-assisted diffusion stages.
- Why included: current trend in 2024-2025 papers.
- Sources:
  - https://www.nature.com/articles/s41598-025-95511-y
  - https://www.nature.com/articles/s41598-024-57756-x

7. **Latest surveys and trend aggregation**
- Useful for paper landscape and terminology normalization.
- Sources:
  - https://www.mdpi.com/2571-5577/5/3/57
  - https://link.springer.com/article/10.1007/s11042-026-21425-0

## What we will treat as implementation reference set
For a publishable optimization study, use one representative per bucket:
- Fridrich-style map permutation + chain diffusion
- Sort-based permutation + chain diffusion
- Bit-plane/DNA-enhanced variant
- Parallel bit-plane cross permutation-diffusion variant

This gives architectural diversity without unbounded implementation scope.
