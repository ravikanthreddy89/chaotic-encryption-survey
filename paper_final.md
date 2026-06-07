# From Chaotic Maps to SIMD-Native Image Transforms: A Stage-Level Performance and Security Evaluation

**Ravikanth Reddy Gudipati, Senior Member, IEEE**

University at Buffalo, Buffalo, NY, USA

**Corresponding author:** Ravikanth Reddy Gudipati

## Abstract

Chaos-based image-encryption studies commonly evaluate complete schemes and
image-domain statistics, making it difficult to identify which design choices
actually determine encryption throughput and whether favorable image statistics
correspond to meaningful cryptographic resistance. This work presents a reproducible
C++17 benchmark that decomposes representative chaotic image-encryption
methods into their principal processing stages, such as keystream generation,
pixel permutation, and diffusion. Traditional components are compared with
SIMD-oriented replacements on 12 deterministic synthetic images and the
24-image Kodak PhotoCD dataset. The evaluation contains 14,730 performance
observations and 3,300 full-scheme security observations, with ten repetitions
per configuration.

The results show that implementation structure matters more than the nominal
presence of SIMD instructions. Replacing chaotic sort permutation with a
linear block permutation improves mean permutation throughput by
105.9--135.0 times at tested sizes. Replacing a global feedback chain with
AVX2 multi-lane or tree diffusion improves mean diffusion throughput by
approximately 1.9--2.1 times. The fastest benchmark-informed candidate pipeline,
Checkerboard-CA-ARX, sustains 245.7 MiB/s on Kodak images and 188.9 MiB/s at
4096x4096, while the fastest standard baseline, ChaCha20 through OpenSSL
EVP, remains substantially faster.

The security results also expose a critical limitation: favorable entropy,
histogram, correlation, and key-sensitivity values do not imply resistance to
differential or known-plaintext attacks. The evaluated deterministic XOR
schemes exhibit negligible NPCR under a one-bit plaintext change and are
recoverable under reused key/nonce conditions. The benchmark-informed candidate
pipelines are
therefore presented as performance templates rather than deployment-ready
ciphers. The central contribution is an experimentally supported design
method: chaotic-image research should benchmark replaceable stages, use
standard cryptographic baselines, and report performance and security claims
separately.

**Keywords:** chaotic image encryption; SIMD; AVX2; permutation-diffusion;
stage-level benchmarking; cellular automata; image cryptanalysis

## 1. Introduction

The permutation-diffusion architecture is a recurring structure in
chaos-based image encryption. A permutation stage rearranges pixel positions,
and a diffusion stage modifies pixel values using a key-dependent sequence.
The architecture is attractive because it maps naturally to common image
statistics: permutation reduces adjacent-pixel correlation, while diffusion
can flatten histograms and increase sensitivity to key changes.

However, two problems complicate performance and security claims in this
literature. First, complete-scheme timing hides the contribution of individual
stages. A proposed SIMD diffusion kernel may be fast while an
`O(N log N)` chaotic sort still dominates total execution time. Conversely, a
sequential chaotic generator may prevent a pipeline from benefiting from an
otherwise parallel permutation. Second, image-domain metrics such as entropy,
correlation, NPCR, and UACI are diagnostic measurements rather than proofs of
cryptographic security. A deterministic stream-XOR construction can produce a
uniform-looking ciphertext while remaining vulnerable when a key/nonce pair is
reused.

This study investigates how implementation-friendly stages affect the
performance of chaotic image-encryption methods without conflating that
engineering result with cryptographic security. We implement traditional
chaotic components, SIMD-oriented alternatives, and standard cryptographic
baselines within one benchmark suite. The experiments isolate keystream,
permutation, and diffusion execution time before composing selected components
into benchmark-informed candidate pipelines.

The study answers four research questions:

- **RQ1:** Which stages dominate representative chaotic image-encryption
  pipelines?
- **RQ2:** Which replaceable stage designs benefit most from linear-time and
  SIMD-oriented restructuring?
- **RQ3:** Do stage-level improvements remain visible in composed candidate
  pipelines and at larger image sizes?
- **RQ4:** Which security conclusions are, and are not, supported by common
  image-domain metrics?

The contributions are:

1. A reproducible C++17/OpenCV/OpenSSL benchmark with a common cipher
   interface and separate keystream, permutation, diffusion, and total timing.
2. A controlled comparison of traditional components and SIMD-oriented
   replacements across synthetic images, real natural images, and resolutions
   up to 4096x4096.
3. Evidence that linear block permutation and lane/tree diffusion remove major
   implementation bottlenecks, while scalar floating-point maps and
   sort-based permutation remain expensive.
4. A negative security result demonstrating that strong image statistics can
   coexist with differential and reused-nonce known-plaintext weaknesses.
5. An explicit boundary between fast image-domain transformation prototypes
   and cryptographically deployable encryption.

## 2. Background and Related Work

### 2.1 Permutation-Diffusion Image Encryption

Shannon identified confusion and diffusion as fundamental properties of
secret-key systems [1]. Fridrich adapted related ideas to image encryption
using two-dimensional chaotic maps and a permutation-diffusion architecture
[2]. Many later schemes use pixel-, bit-, or block-level permutation followed
by one or more diffusion rounds [3,4].

Traditional chaotic permutation often generates one chaotic score per pixel
and sorts pixel indices by those scores. The method is simple to describe but
requires `O(N log N)` comparisons and irregular memory access. Diffusion is
frequently implemented as a global recurrence:

```
C[i] = P[i] XOR K[i] XOR C[i-1],
```

where `P`, `K`, and `C` denote permuted plaintext, keystream, and ciphertext.
This construction creates an apparent avalanche path but also introduces a
loop-carried dependency that restricts vectorization.

### 2.2 Chaotic and Spatial Keystream Generators

Scalar logistic, tent, and sine maps are common keystream sources. Their
iterative dependency and floating-point operations can limit throughput.
Spatial systems such as coupled-map lattices and cellular automata expose
multiple local states and are therefore more compatible with lane-level or
data-parallel execution [5]. This work evaluates both categories, as well as
Hamiltonian-inspired and reaction-diffusion generators, to determine whether
their implementation structures produce measurable benefits.

### 2.3 SIMD-Oriented Restructuring

Single Instruction, Multiple Data (SIMD) instructions operate on multiple
values per instruction. On the evaluated x86-64 host, AVX2 provides 256-bit
integer vectors. SIMD benefits are largest when work can be expressed as
regular independent operations. Sorting, pointer chasing, and global
recurrences are less suitable.

Associative operations can sometimes be reformulated as scans. The global XOR
chain can be written as:

```
X[i] = P[i] XOR K[i]
C[i] = IV XOR X[0] XOR ... XOR X[i].
```

This permits block-wise prefix processing with a carry between blocks.
Multi-lane chaining instead maintains 32 independent byte chains per AVX2
vector:

```
C[i] = P[i] XOR K[i] XOR C[i-32].
```

Tree diffusion applies shuffle-and-XOR stages to mix bytes within a vector in
logarithmic depth. These transformations trade the dependency structure of a
global chain for regular parallel work. Parallel scan and SIMD prefix
techniques are established general-purpose primitives [6].

### 2.4 Security Evaluation

Entropy, histogram uniformity, adjacent-pixel correlation, NPCR, and UACI are
widely reported in image-encryption studies. NPCR and UACI quantify output
changes caused by a small input modification [7]. These metrics are useful,
but they do not replace cryptanalysis. Prior work has demonstrated
chosen-plaintext and other structural attacks against permutation-diffusion
image ciphers [8,9].

For this reason, the present study includes AES-256-CTR and ChaCha20 as
standardized or widely deployed cryptographic reference points [10,11].
BLAKE3 keyed extendable output is also evaluated as a high-quality keystream
source [12]. These baselines anchor the performance discussion; the proposed
stage combinations are not claimed to provide equivalent security.

## 3. Benchmark Design

### 3.1 Common Pipeline Model

Each complete scheme implements a common `IImageCipher` interface. Encryption
is represented as three timed stages:

1. **Keystream generation:** produce `N` key-dependent bytes.
2. **Permutation:** rearrange pixels, bytes, blocks, or local positions.
3. **Diffusion:** combine the permuted data and keystream.

Total execution time includes stage execution and required data conversion or
allocation overhead. Throughput, reported in mebibytes per second (MiB/s), is:

```
throughput = image_bytes / total_execution_time.
```

The suite also executes each replaceable stage independently. Checksums prevent
unused outputs from being optimized away.

### 3.2 Full-Scheme Baselines

The full-scheme suite contains the following groups:

- Scalar chaotic XOR: logistic, tent, sine, coupled-lattice, and
  Hamiltonian-lattice generators followed by XOR.
- Permutation plus XOR: logistic sort permutation, Arnold map, and tiled
  Arnold map followed by logistic keystream XOR.
- Cryptographic keystream: official BLAKE3 keyed XOF followed by XOR.
- Standard baselines: AES-256-CTR and ChaCha20 through OpenSSL EVP.

All full schemes are reversible and checked by encryption followed by
decryption. The benchmark intentionally reuses a fixed key and nonce across
security probes to expose reused-keystream behavior. This is a misuse test,
not a recommended operating mode for AES-CTR, ChaCha20, or BLAKE3.

### 3.3 Replaceable Keystream Stages

The isolated keystream experiments evaluate:

- scalar logistic double map;
- scalar sine map;
- fixed-point tent generator;
- 16-state coupled-map lattice (CML);
- 64-cell rule-30-like cellular automaton (CA);
- Hamiltonian-inspired lattice; and
- reaction-diffusion generator.

The generators are prototypes used to compare computational structures.
Passing randomness tests or providing cryptographic unpredictability is
outside the claims of this benchmark.

### 3.4 Replaceable Permutation Stages

The permutation experiments compare:

- chaotic score sort;
- sequential random walk;
- Arnold/cat map;
- affine modular index mapping;
- key-derived Feistel-like index mapping with cycle walking;
- 256-byte block permutation; and
- four rounds of disjoint checkerboard swaps.

Chaotic sort is the traditional expensive baseline. The block and checkerboard
implementations are deliberately simple performance templates. In the current
prototype, block reversal and checkerboard swaps are fixed rather than
key-derived; they therefore must not be interpreted as secure permutations.
The Feistel-like mapping is key-derived, but it has not received independent
cryptanalysis.

### 3.5 Replaceable Diffusion Stages

The diffusion experiments include:

- global XOR feedback chain;
- block-local chain;
- blocked prefix processing;
- two-dimensional stencil;
- ARX block transform;
- bit-plane-style rotate/XOR transform;
- forward and reverse prefix XOR;
- 32-lane AVX2 chaining;
- AVX2 tree XOR;
- forward-prefix/tree/reverse-prefix composition; and
- ARX prefix modulo 256.

The prefix-XOR prototype uses AVX2 loads, XORs, broadcasts, and stores, but its
within-vector prefix helper currently spills to a byte array and performs a
scalar scan. It should therefore be regarded as a transitional implementation,
not an optimal SIMD scan. The multi-lane and tree variants contain the clearest
AVX2-native data paths.

### 3.6 Candidate Pipelines

Seven candidate combinations test whether isolated improvements survive
composition:

| Candidate | Keystream | Permutation | Diffusion |
|---|---|---|---|
| CA-Feistel-ARX | cellular automata | Feistel-like index | ARX block |
| Checkerboard-CA-ARX | cellular automata | checkerboard swaps | ARX block |
| Affine-CA-PrefixTree | cellular automata | affine mapping | prefix/tree/reverse |
| Checkerboard-CA-MultilaneTree | cellular automata | checkerboard swaps | multi-lane/tree |
| CML-Feistel-Stencil | CML | Feistel-like index | stencil |
| Hamiltonian-Block-Stencil | Hamiltonian lattice | block-Feistel mapping | stencil |
| Affine-CML-Bitplane | CML | affine mapping | bit-plane transform |

These pipelines currently produce checksums for performance validation but do
not implement complete decryptors or equivalent security probes. They are
evaluated as component-composition experiments, not proposed secure ciphers.

## 4. Experimental Methodology

### 4.1 Implementation and Platform

The benchmark is implemented in C++17 and built with CMake in Release mode
using GCC 11.4.0, `-O3`, `-march=native`, and explicit AVX2 support. OpenCV is
used for image loading and output, OpenSSL EVP provides AES-256-CTR and
ChaCha20, and the official BLAKE3 C implementation provides keyed XOF output.

Experiments ran on a four-core Intel Xeon E5-2620 v4 at 2.10 GHz under
64-bit Linux 5.15. The processor supports SSE2, AVX, AVX2, and AES
instructions. The BLAKE3 library was compiled in portable mode, so the BLAKE3
result does not include its optional architecture-specific SIMD dispatch.
Candidate and stage measurements are single-process measurements; this paper
does not claim multicore scaling.

### 4.2 Datasets

Two dataset classes are used:

- **Synthetic controls:** deterministic gradient, texture, and noise images at
  512x512, 1024x1024, 2048x2048, and 4096x4096.
- **Natural images:** all 24 color images from the Kodak PhotoCD dataset [13],
  comprising 768x512 and 512x768 orientations.

Synthetic images provide controlled size and content variation. Kodak images
provide natural-image diversity while retaining a stable public benchmark.

### 4.3 Experiment Matrix

Every executed configuration is repeated ten times. Slow full schemes are
limited to images no larger than 1024x1024. Intentionally slow stage
baselines are limited to 1024x1024, while fast stage alternatives continue to
4096x4096. Slow candidate pipelines continue to 2048x2048, and fast candidates
continue to 4096x4096.

The final dataset contains:

- 3,300 full-scheme performance records;
- 3,300 full-scheme security records;
- 9,000 isolated-stage performance records; and
- 2,430 candidate-pipeline performance records.

Reported aggregate throughput values are arithmetic means. The 95% confidence
interval is calculated as:

```
CI95 = 1.96 * sample_standard_deviation / sqrt(n).
```

The benchmark records mean, median, minimum, maximum, standard deviation, and
95% confidence interval. No outlier removal is applied.

### 4.4 Security Metrics

For full schemes, the suite records:

- Shannon entropy and 256-bin chi-square statistic;
- horizontal, vertical, and diagonal adjacent-pixel correlation;
- NPCR and UACI after flipping one bit of the plaintext;
- NPCR after flipping one bit of the key;
- empirical known-plaintext and chosen-plaintext byte-recovery scores under a
  reused key/nonce; and
- exact decryption correctness.

For the byte-recovery probe, a keystream estimate is computed as
`K'[i] = P1[i] XOR C1[i]` and applied to a second ciphertext. A score of 1.0
means complete recovery under this simple reused-keystream model. The test is
expected to break stream-XOR constructions when the same key/nonce is reused.

## 5. Results

### 5.1 Full-Scheme Performance

Table 1 reports selected aggregate full-scheme results. ChaCha20 is the
fastest complete baseline on all commonly measured image dimensions.
AES-256-CTR is second among the standard baselines. BLAKE3-XOR is the fastest
non-OpenSSL full scheme, but its portable-only BLAKE3 build makes this a
conservative BLAKE3 measurement.

**Table 1. Selected aggregate full-scheme throughput.**

| Scheme | Image size | n | Mean MiB/s | 95% CI | Speedup vs logistic XOR |
|---|---:|---:|---:|---:|---:|
| ChaCha20 | 512x512 | 30 | 945.506 | 46.968 | 7.127x |
| ChaCha20 | 768x512 | 180 | 845.183 | 12.099 | 6.316x |
| ChaCha20 | 1024x1024 | 30 | 768.588 | 25.216 | 5.969x |
| AES-256-CTR | 768x512 | 180 | 337.636 | 3.266 | 2.523x |
| AES-256-CTR | 1024x1024 | 30 | 533.687 | 15.948 | 4.145x |
| BLAKE3-XOR | 768x512 | 180 | 299.394 | 3.426 | 2.237x |
| BLAKE3-XOR | 1024x1024 | 30 | 267.421 | 8.154 | 2.077x |

The results do not support a claim that chaotic-image pipelines outperform
standard ciphers. Instead, they show the cost of common chaotic components and
provide a performance target for alternative stage designs.

### 5.2 Isolated Permutation Performance

Permutation produces the largest stage-level improvement. At 512x512, block
permutation reaches 488.197 MiB/s and is 105.859 times faster than chaotic
sort. At 1024x1024 it reaches 465.646 MiB/s and is 134.984 times faster.

**Table 2. Aggregate block-permutation performance relative to chaotic sort.**

| Image size | n | Mean MiB/s | 95% CI | Speedup |
|---|---:|---:|---:|---:|
| 512x512 | 30 | 488.197 | 10.410 | 105.859x |
| 512x768 | 60 | 484.649 | 5.764 | 111.033x |
| 768x512 | 180 | 480.218 | 3.135 | 110.670x |
| 1024x1024 | 30 | 465.646 | 8.393 | 134.984x |

This result answers RQ1 and RQ2: chaotic sort is an
implementation-hostile design choice, and replacing it with a regular
linear-time transform changes the performance regime. The result is a
performance finding only. The benchmark block permutation is fixed and must be
made key-derived and cryptographically analyzed before use in a cipher.

### 5.3 Isolated Diffusion Performance

The fastest diffusion alternatives are multi-lane chaining and tree XOR.
Their mean speedups over the global chain are approximately twofold.

**Table 3. Selected aggregate diffusion performance.**

| Stage | Image size | n | Mean MiB/s | 95% CI | Speedup vs global chain |
|---|---:|---:|---:|---:|---:|
| Multi-lane chain AVX2 | 512x512 | 30 | 479.517 | 9.062 | 2.130x |
| Multi-lane chain AVX2 | 768x512 | 180 | 483.497 | 2.725 | 2.125x |
| Multi-lane chain AVX2 | 1024x1024 | 30 | 447.583 | 11.638 | 1.977x |
| Tree XOR AVX2 | 512x512 | 30 | 452.561 | 8.089 | 2.010x |
| Tree XOR AVX2 | 768x512 | 180 | 454.466 | 2.819 | 1.997x |
| Tree XOR AVX2 | 1024x1024 | 30 | 426.760 | 9.649 | 1.885x |
| ARX block diffusion | 768x512 | 180 | 411.095 | 2.810 | 1.807x |

The result demonstrates that removing a global byte-to-byte dependency creates
a measurable SIMD opportunity. It also explains why adding SIMD only to XOR
inside a traditional end-to-end pipeline may show little total improvement:
the remaining generator and permutation costs dominate.

### 5.4 Candidate Pipeline Performance

Checkerboard-CA-ARX is the fastest composed candidate. On Kodak images it
achieves 245.658 MiB/s, while Checkerboard-CA-MultilaneTree achieves
220.599 MiB/s. Both remain slower than AES-256-CTR and ChaCha20 on the same
image dimensions.

**Table 4. Aggregate candidate-pipeline throughput.**

| Candidate | Image size | n | Mean MiB/s | 95% CI |
|---|---:|---:|---:|---:|
| Checkerboard-CA-ARX | 512x512 | 30 | 249.045 | 6.148 |
| Checkerboard-CA-ARX | 768x512 | 180 | 245.658 | 3.054 |
| Checkerboard-CA-ARX | 1024x1024 | 30 | 211.652 | 5.127 |
| Checkerboard-CA-ARX | 2048x2048 | 30 | 196.312 | 3.182 |
| Checkerboard-CA-ARX | 4096x4096 | 30 | 188.949 | 2.567 |
| Checkerboard-CA-MultilaneTree | 768x512 | 180 | 220.599 | 2.585 |
| Checkerboard-CA-MultilaneTree | 4096x4096 | 30 | 150.678 | 2.260 |

The candidate stage shares explain the remaining bottlenecks:

| Candidate | Keystream share | Permutation share | Diffusion share |
|---|---:|---:|---:|
| Checkerboard-CA-ARX | 47.46% | 27.93% | 24.59% |
| Checkerboard-CA-MultilaneTree | 38.34% | 22.68% | 38.97% |
| CA-Feistel-ARX | 10.84% | 83.57% | 5.58% |
| Hamiltonian-Block-Stencil | 82.42% | 13.30% | 4.28% |

The fast checkerboard candidates distribute work across all three stages,
whereas Feistel indexing dominates CA-Feistel-ARX and Hamiltonian generation
dominates Hamiltonian-Block-Stencil. These results support RQ3: isolated-stage
choices remain visible after composition, and stage timing identifies the next
optimization target.

### 5.5 Full-Scheme Security Diagnostics

All complete schemes decrypt correctly. Several also produce near-eight-bit
entropy, low adjacent-pixel correlation, and strong key sensitivity. These
positive metrics are insufficient to establish security.

**Table 5. Mean security diagnostics across 300 observations per scheme.**

| Scheme | Entropy | Chi-square | Mean absolute correlation | Plaintext NPCR | Key sensitivity | KPA recovery |
|---|---:|---:|---:|---:|---:|---:|
| ChaCha20 | 7.999845 | 258.874 | 0.000738 | 0.000084% | 99.619% | 1.000 |
| AES-256-CTR | 7.999843 | 259.125 | 0.000424 | 0.000084% | 99.611% | 1.000 |
| BLAKE3-XOR | 7.999850 | 247.607 | 0.000759 | 0.000084% | 99.607% | 1.000 |
| Tent-XOR | 7.999828 | 282.731 | 0.001231 | 0.000084% | 99.623% | 1.000 |
| Logistic-XOR | 7.984321 | 26641.399 | 0.020388 | 0.000084% | 99.387% | 1.000 |
| Coupled-lattice-XOR | 7.971770 | 45857.479 | 0.134324 | 0.000084% | 99.410% | 1.000 |

The KPA recovery score of 1.0 for AES-CTR, ChaCha20, BLAKE3-XOR, and the
stream-XOR chaotic variants is expected because the experiment deliberately
reuses the same key/nonce. It demonstrates misuse sensitivity, not a break of
the underlying standardized primitive. The negligible plaintext NPCR is also
expected for CTR- or stream-XOR encryption: changing one plaintext bit changes
only the corresponding ciphertext bit when the keystream is unchanged.

This result answers RQ4. Entropy, correlation, and key sensitivity can look
excellent while a construction still lacks plaintext avalanche and fails
under reused keystream. Furthermore, NPCR is not an appropriate requirement
for all secure encryption modes; standard stream ciphers intentionally do not
provide plaintext avalanche. Claims must therefore be tied to the construction
and threat model rather than to universal image-metric thresholds.

The candidate pipelines were not subjected to the same security harness and
contain fixed prototype permutations. No cryptographic-security conclusion is
made for them.

## 6. Discussion

### 6.1 What SIMD Changes

The experiments show that SIMD is not a property of a pipeline label. It is a
property of the dependency graph and memory-access pattern of each stage.
Regular, independent byte or word operations benefit from AVX2. Global
recurrences, transcendental scalar maps, cycle walking, and sort-based
permutation restrict the gain.

The largest improvement does not come from a vector instruction alone. It
comes from replacing chaotic sort with a linear regular mapping. Similarly,
the diffusion improvement comes from changing one global chain into multiple
lanes or a tree. Algorithmic restructuring precedes vectorization.

### 6.2 Performance Versus Security

The fastest stage is not necessarily the strongest stage. The fixed block and
checkerboard permutations are useful controls for measuring the cost of
regular memory movement, but they provide no secret permutation by themselves.
The CA generator is fast, but its rule-30-like output has not been established
as a cryptographically secure pseudorandom stream. Tree XOR and linear
multi-lane diffusion are also structurally analyzable.

A deployment-oriented design should therefore use a standard cryptographic
primitive to derive independent subkeys and masks, make every permutation
key-derived, include unique nonces, and provide authentication. A practical
research direction is to retain image-domain transforms only as a
format-aware preprocessing or selective-encryption layer, then protect the
result with an authenticated standard cipher.

### 6.3 Implications for Future Chaotic-Image Studies

Based on the results, future evaluations should:

1. report isolated keystream, permutation, and diffusion execution time;
2. include AES and ChaCha through optimized libraries;
3. compare algorithmic complexity before attributing speedup to SIMD;
4. test real images and controlled synthetic inputs;
5. report repeated measurements with confidence intervals;
6. separate image statistics from cryptographic claims; and
7. include negative controls for nonce reuse, known plaintext, and chosen
   plaintext.

## 7. Threats to Validity and Limitations

This study has several limitations.

- Measurements were collected on one older x86-64 server CPU. Results may
  differ on newer AVX2/AVX-512 processors, ARM NEON/SVE systems, GPUs, or
  embedded devices.
- CPU frequency, virtualization noise, memory allocation, and cache state were
  not controlled through hardware performance counters or processor pinning.
- The benchmark reports wall-clock stage time and throughput, but not cycles
  per byte, energy, cache misses, branch misses, or instruction counts.
- The prefix-XOR helper is only partially vectorized because its intra-vector
  scan is scalar.
- The official BLAKE3 implementation is compiled without architecture-specific
  SIMD dispatch.
- The fixed block and checkerboard permutations are performance templates, not
  secure key-derived permutations.
- Candidate pipelines do not yet implement inverse transforms or undergo the
  full cryptanalysis suite.
- Image-domain diagnostics are reported only for full schemes. They do not
  constitute proofs of security.
- AES-CTR and ChaCha20 are confidentiality-only baselines in this benchmark;
  deployment should use an authenticated-encryption construction.

These limitations constrain the claim to stage-level performance analysis and
prototype composition. They do not invalidate the finding that sort permutation
and global dependencies are major implementation bottlenecks.

## 8. Reproducibility

The source tree includes the benchmark implementation, dataset generators,
Kodak downloader, experiment harness, raw outputs, aggregate CSV files, and
figure-generation scripts. The final experiment can be reproduced with:

```bash
REPS=10 REAL_REPS=10 SIZES="512 1024 2048 4096" \
  ./scripts/run_paper_ready_experiments.sh
```

Primary result files are:

- `results/final/bench.csv` and `bench_stats.csv`;
- `results/final/stage_bench.csv` and `stage_stats.csv`;
- `results/final/candidate_schemes.csv` and `candidate_stats.csv`;
- `results/final/analysis.csv`;
- `results/final/tables.md`;
- `results/final/performance_summary.md`; and
- `results/final/metadata.md`.

The exact source revision recorded with the experiment is
`af3901293c3c835be4433d529d31b292ca1cb061`.

## 9. Conclusion

This work decomposed chaotic image-encryption pipelines into replaceable
keystream, permutation, and diffusion stages and evaluated traditional and
SIMD-oriented alternatives. The dominant finding is architectural:
implementation-hostile components, especially chaotic sort and global
feedback dependencies, determine encryption throughput more strongly than the
mere use of SIMD instructions.

Linear block permutation improves throughput by more than two orders of
magnitude relative to chaotic sort in the tested configurations. AVX2
multi-lane and tree diffusion approximately double throughput relative to a
global chain. These improvements remain visible in composed candidate
pipelines and at resolutions up to 4096x4096. Nevertheless, optimized ChaCha20
and AES-256-CTR remain faster and have substantially stronger security
foundations.

The security experiments reinforce an equally important conclusion:
image-domain statistics must not be presented as cryptographic proof. The
current XOR-based full schemes show favorable entropy and key sensitivity but
fail differential-avalanche and reused-keystream recovery controls. The
benchmark-informed candidates are therefore performance prototypes, not secure
ciphers.

The most defensible contribution is a methodology for future work: benchmark
stages independently, select and compose alternatives only after bottlenecks
are understood, compare against standard cryptography, and keep performance
claims separate from security claims.

## Declarations

### Author Contributions

Ravikanth Reddy Gudipati conceived the study, implemented the benchmark,
conducted the experiments, analyzed the results, and prepared the manuscript.

### Data and Code Availability

The benchmark source code, experiment scripts, raw measurements, aggregate
tables, and reproduction instructions are available at
https://github.com/ravikanthreddy89/chaotic-image-encryption. The experiment
revision is identified in Section 8.

## References

[1] C. E. Shannon, "Communication Theory of Secrecy Systems," *Bell System
Technical Journal*, vol. 28, no. 4, pp. 656--715, 1949.
https://doi.org/10.1002/j.1538-7305.1949.tb00928.x

[2] J. Fridrich, "Symmetric Ciphers Based on Two-Dimensional Chaotic Maps,"
*International Journal of Bifurcation and Chaos*, vol. 8, no. 6,
pp. 1259--1284, 1998. https://doi.org/10.1142/S021812749800098X

[3] C. Fu, J.-B. Huang, N.-N. Wang, Q.-B. Hou, and W.-M. Lei, "A Symmetric
Chaos-Based Image Cipher with an Improved Bit-Level Permutation Strategy,"
*Entropy*, vol. 16, no. 2, pp. 770--788, 2014.
https://doi.org/10.3390/e16020770

[4] L. Huang, S. Cai, M. Xiao, and X. Xiong, "A Simple Chaotic Map-Based
Image Encryption System Using Both Plaintext Related Permutation and
Diffusion," *Entropy*, vol. 20, no. 7, article 535, 2018.
https://doi.org/10.3390/e20070535

[5] X. Chai et al., "A New Color Image Encryption Scheme Using CML and a
Fractional-Order Chaotic System," *PLoS ONE*, vol. 10, no. 3, e0119660,
2015. https://doi.org/10.1371/journal.pone.0119660

[6] W. Zhang, Y. Wang, and K. A. Ross, "Parallel Prefix Sum with SIMD,"
arXiv:2312.14874, 2023.
https://doi.org/10.48550/arXiv.2312.14874

[7] Y. Wu, J. P. Noonan, and S. Agaian, "NPCR and UACI Randomness Tests for
Image Encryption," *Journal of Selected Areas in Telecommunications*,
pp. 31--38, 2011.

[8] C. Li, D. Lin, B. Feng, J. Lü, and F. Hao, "Cryptanalysis of a Chaotic
Image Encryption Algorithm Based on Information Entropy," *IEEE Access*,
vol. 6, pp. 75834--75842, 2018.
https://doi.org/10.1109/ACCESS.2018.2883690

[9] Y. Liu, L. Y. Zhang, J. Wang, Y. Zhang, and K.-W. Wong,
"Chosen-Plaintext Attack of an Image Encryption Scheme Based on Modified
Permutation-Diffusion Structure," arXiv:1503.06638, 2015.
https://doi.org/10.48550/arXiv.1503.06638

[10] National Institute of Standards and Technology, "Advanced Encryption
Standard (AES)," FIPS PUB 197, updated 2023.
https://doi.org/10.6028/NIST.FIPS.197-upd1

[11] Y. Nir and A. Langley, "ChaCha20 and Poly1305 for IETF Protocols,"
RFC 8439, 2018. https://www.rfc-editor.org/rfc/rfc8439

[12] J. O'Connor, J.-P. Aumasson, S. Neves, and Z. Wilcox-O'Hearn,
"BLAKE3: One Function, Fast Everywhere," 2020.
https://github.com/BLAKE3-team/BLAKE3-specs/blob/master/blake3.pdf

[13] R. Franzen, "Kodak Lossless True Color Image Suite."
https://r0k.us/graphics/kodak/
