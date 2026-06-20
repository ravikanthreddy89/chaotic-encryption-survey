# Critical Literature Matrix, 2022-2026

This is the working evidence table for the revised paper. Rows below are
starter entries that support manuscript positioning and claim boundary
drafting. Before journal submission, verify venue/indexing status, DOI, and
final bibliographic metadata against the publisher page.

| ID | Year | DOI / arXiv | Venue | Verified Q/category | Family | Dataset | Hardware | Runtime unit | AES/ChaCha baseline | Code/data | Stage timing | KPA/CPA | Finite precision | Notes |
|---|---:|---|---|---|---|---|---|---|---|---|---|---|---|---|
| jiang2023 | 2023 | arXiv:2302.07411 | arXiv preprint | not yet verified | Parallel confusion-diffusion | Video frames | Intel Core i5/i7/Xeon | FPS / throughput | not reported | not reported | partial | not reported | not reported | Video-oriented, useful for parallelization framing |
| sanaboina2024 | 2024 | arXiv:2406.07560 | arXiv preprint | not yet verified | Chaos-based permutation-diffusion | Diagnostic imagery | not reported | not reported | not reported | not reported | not reported | not reported | not reported | Medical-image application, mostly statistical evaluation |
| ali2025 | 2025 | arXiv:2503.09939 | arXiv preprint | not yet verified | Geometric block permutation + dynamic substitution | not reported | not reported | not reported | not reported | not reported | not reported | not reported | not reported | Very close to the block/permutation family used here |
| li2026 | 2026 | arXiv:2605.21118 | arXiv preprint | not yet verified | Data-identified discrete chaotic maps | Hénon / Lozi / 3D logistic | not reported | not reported | not reported | not reported | not reported | not reported | partial | Strong candidate for a later proxy or sensitivity study |

## Inclusion Rules

1. Include at least 25 peer-reviewed papers published from 2022 through 2026
   before treating this as submission-ready.
2. Cover DNA/symbolic, hyperchaotic, dynamic S-box, bit-plane, block/
   geometric, cellular-automata, and parallel families.
3. Record `not reported` rather than inferring an omitted experiment.
4. Normalize runtime only when image dimensions, channels, hardware, and timed
   scope are available.
5. Do not call a proxy an exact reproduction without matching equations,
   parameters, rounds, key schedule, and preprocessing.

## Quantities to Report

- Number and percentage providing runnable source code.
- Number reporting a standard-cipher baseline.
- Number reporting stage-level timing or ablation.
- Number including known- or chosen-plaintext analysis.
- Number discussing finite-precision state collapse or effective key space.

## Working Interpretation

- The current paper is strongest when framed as a benchmark and cryptanalytic
  sanity-check study.
- The most defensible comparison message is not "our cipher is stronger" but
  "our stage measurements reveal which recent design families are
  implementation-hostile and which are SIMD-friendly."
- The matrix above is intentionally sparse. It is a starting point for the
  verified 25-paper journal appendix, not a final literature survey.
