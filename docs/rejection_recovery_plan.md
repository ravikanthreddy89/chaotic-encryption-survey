# Rejection Recovery Plan

## Editor decision in practical terms

The rejection is not mainly about formatting. It says the manuscript is not
yet convincing as a publishable research contribution. The critical concerns
are:

1. **Incremental novelty:** the manuscript does not make a strong enough case
   that stage-level SIMD benchmarking is a new contribution relative to recent
   chaotic image-encryption work.
2. **Weak critical analysis:** the related-work section mostly gives
   background, not a structured critique of what recent papers do, omit, and
   measure.
3. **Insufficient state-of-the-art comparison:** the experimental section does
   not compare against enough SCI/WoS or IEEE-field papers from 2022--2026.
4. **Naive experimental framing:** the manuscript compares internal prototype
   stages and standard ciphers, but it does not reproduce or fairly proxy
   enough recent image-encryption schemes.
5. **Contribution mismatch:** the current candidate pipelines are explicitly
   not deployable ciphers. That honesty is good, but it also weakens the paper
   if the target journal expects a new encryption algorithm with full security
   validation.

## Recommended strategic pivot

Do **not** resubmit the current paper with only more citations. The current
paper is strongest as a **benchmarking and cryptanalysis methodology paper**,
not as a new chaotic image cipher. The revised submission should make one of
two clear claims:

### Option A: Benchmarking paper

Title direction:

> A Reproducible Stage-Level Benchmark and Cryptanalytic Sanity Check for
> Recent Chaotic Image Encryption Schemes

Core claim:

> Recent chaotic image-encryption papers often report image-domain statistics
> without isolating stage costs or testing simple adversarial misuse cases. This
> work reproduces representative 2022--2026 design families under a common
> benchmark and shows which components dominate runtime and which security
> claims are unsupported.

This is the better fit for the current repo.

### Option B: New cipher paper

Only choose this if the codebase is extended to include a complete decryptable,
key-derived, nonce-respecting, authenticated or authentication-compatible image
encryption design. The candidate pipelines currently in the manuscript are not
enough because the paper itself says they are performance templates, not secure
ciphers.

## Required manuscript changes

### 1. Rewrite the introduction around a sharper gap

Current gap:

> Stage-level timing is useful and image metrics are not proof.

Stronger gap:

> From 2022--2026, many chaotic image-encryption papers introduce new maps,
> DNA/symbolic operations, hyperchaotic systems, S-boxes, block permutations,
> and neural compression layers. However, they are rarely evaluated under a
> common implementation harness, rarely compare against optimized AES/ChaCha
> baselines, and often lack adversarial tests such as known-plaintext,
> chosen-plaintext, nonce-reuse, finite-precision, and ablation studies.

### 2. Add a critical related-work matrix

Create a table with at least 20 recent papers, preferably 2022--2026, and
include:

- year
- venue and indexing status, verified manually
- algorithm family
- dataset used
- entropy, NPCR, UACI, correlation, PSNR if reported
- runtime or throughput, including hardware
- whether AES/ChaCha or other standard baseline is compared
- whether code/data are available
- whether known/chosen-plaintext analysis is included
- whether finite-precision or key-space discretization is discussed
- whether ablation or stage timing is included

The table should support a sentence like:

> Among N recent studies, only X report standard-cipher baselines, Y provide
> runnable source code, Z isolate stage-level cost, and W include explicit
> known/chosen-plaintext analysis.

That quantified critique directly answers the editor's "no critical analyses"
comment.

### 3. Add recent representative baselines

The current baseline set is too internally defined. Add reproducible proxies
for recent families, even if exact author code is unavailable:

- DNA/dynamic symbolic operation family.
- Hyperchaotic or 4D/5D chaotic map family.
- S-box/dynamic substitution family.
- Block/geometric permutation family.
- Bit-plane permutation/diffusion family.
- Parallel-aware or GPU/SIMD-friendly family, if available.
- Neural/compression-assisted image-encryption family, if kept in scope.

Each proxy must be justified as implementing the dominant computational
structure of a cited family, not as a bit-for-bit reproduction.

### 4. Add deeper experiments

Minimum additions:

- stage ablation for each representative family
- throughput in MiB/s and cycles/byte if possible
- memory allocation excluded/included variants
- hardware counters if feasible: cache misses, branch misses, instructions
- finite-precision sensitivity for floating-point maps
- statistical tests on keystreams, at least NIST STS or PractRand if feasible
- known-plaintext and chosen-plaintext probes for every reproduced family
- nonce/key reuse tests clearly separated from normal-mode tests
- cross-image generality: Kodak plus USC-SIPI or BOSSBase/medical/satellite if
  the target venue expects domain-specific image security

### 5. Change the claims

Avoid claiming:

- "proposed cipher"
- "robust against attacks"
- "high security" from entropy/NPCR/UACI
- "SIMD-native image encryption" if the candidate is not a full cipher

Claim instead:

- "stage-level benchmark"
- "reproducible comparative evaluation"
- "implementation bottleneck analysis"
- "negative controls for common security claims"
- "design guidelines for future chaos-based image-encryption papers"

## Literature collection targets

Use these as starting points only; verify exact venue, indexing, publication
year, and final citation metadata before adding them to the manuscript.

| Family | Example starting point | Why it matters |
|---|---|---|
| Dynamic DNA coding | Patidar and Kaur, 2022, dynamic DNA coding with conservative chaotic standard map | Represents DNA/symbolic operation trend and extensive image metrics. |
| Compression plus DNA/chaos | Ahmed et al., 2022, convolutional autoencoder plus DNA/chaos | Represents neural/compression-assisted encryption. |
| Parallel S-box/PRNG | Gabr et al., 2023, base-n PRNGs and parallel base-n S-boxes, IEEE Access | Directly relevant to parallel substitution and IEEE-field comparison. |
| Hyperchaos/SVD/RC5/S-box | Youssef et al., 2024, satellite image security via hyperchaos, SVD, RC5, dynamic S-box, IEEE Access | Useful recent IEEE Access baseline with stronger engineering framing. |
| Geometric block permutation | Ali et al., 2025, geometric block permutation and dynamic substitution | Relevant to the repo's block/checkerboard permutation experiments. |
| X-cross / dual block permutation | Ahsan et al., 2025, dual-layer block permutation and dynamic substitution | Relevant to modern block and bit-level permutation designs. |
| Hyperchaotic/CA hybrids | 2024--2025 Scientific Reports hyperchaotic image-encryption papers | Represents current high-dimensional chaos trend. |
| Survey/trend paper | 2022--2026 survey in Applied Sciences, Multimedia Tools and Applications, or similar | Use to justify taxonomy, not as an experimental baseline. |

## Repo work items

1. Add `docs/sota_matrix_2022_2026.md` with the literature matrix and links.
2. Add `results/literature_metrics.csv` for metrics extracted from papers.
3. Extend `src/scalar/ChaoticCiphers.hpp` with representative family proxies.
4. Extend `src/core/StageSuite.hpp` so every reproduced family has stage-level
   timing.
5. Extend `scripts/make_paper_outputs.py` to generate:
   - SOTA comparison table
   - ablation table
   - adversarial-test table
   - runtime-normalized comparison table
6. Add a `docs/claim_boundary.md` file separating:
   - what the experiments prove
   - what they do not prove
   - what is only a design guideline
7. Rewrite `paper/access/access_paper.tex` after the new comparison data exist.

## Likely paper outline after revision

1. Introduction
2. Critical review of 2022--2026 chaotic image encryption
3. Threat model and evaluation pitfalls
4. Common implementation harness
5. Reproduced/proxy schemes and standard baselines
6. Stage-level performance comparison
7. Security diagnostics and adversarial probes
8. Ablation and design lessons
9. Threats to validity
10. Conclusion

## Submission guidance

If targeting IEEE Access or a similar broad engineering journal, the paper
needs a much larger recent comparison set and a table showing exactly how the
work differs from 2022--2026 studies. If targeting a cryptography venue, the
paper should reduce image-encryption novelty claims and emphasize empirical
cryptanalysis, misuse tests, and reproducibility.

The current manuscript should not be resubmitted as-is. It needs either a real
state-of-the-art comparative suite or a complete shift to a shorter benchmark
artifact paper.
