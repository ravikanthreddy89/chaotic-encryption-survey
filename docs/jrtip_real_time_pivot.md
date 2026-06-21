# JRTIP Real-Time Pivot Plan

Target journal: Journal of Real-Time Image Processing.

The submission should be positioned as a real-time implementation study for
chaos-based image-encryption pipelines, not as a new secure chaotic cipher.
The paper must treat real-time behavior as a first-class issue: latency
budgets, frame-rate feasibility, stage bottlenecks, and cross-architecture
stability.

## Top three modifications

### 1. Real-time framing in the manuscript

Revise the title, abstract, introduction, research questions, and conclusion
so that the core contribution is:

> A reproducible real-time evaluation of chaos-based image-encryption pipeline
> stages under frame-rate latency budgets.

Required language:

- 30 FPS and 60 FPS frame budgets.
- End-to-end latency, not only throughput.
- Stage-level bottleneck attribution.
- SIMD/vectorization and CPU architecture effects.
- Conservative security wording.

Avoid language that presents the candidate pipelines as deployment-ready
ciphers.

### 2. Dataset and metric protocol

Keep the existing Kodak and synthetic controls, then add a video-frame workload.
The minimum JRTIP-ready dataset package is:

- Kodak PhotoCD, all 24 images.
- Synthetic controls: gradient, noise, texture at 512, 1024, 2048, and 4096.
- Public video-frame workload: 100-300 frames sampled at 720p and 1080p.
- One image-encryption comparison set, such as USC-SIPI excluding Lenna/Lena,
  only for compatibility with prior image-encryption literature.

Primary real-time metrics:

- Mean `ms/frame` and 95% confidence interval.
- p95 `ms/frame` for video-frame workloads.
- FPS equivalent, computed as `1000 / ms_per_frame`.
- Pass/fail against 30 FPS and 60 FPS budgets.
- MiB/s throughput.
- Stage share: key generation, permutation, diffusion, and total.
- Speedup against scalar variants and AES-CTR/ChaCha20 baselines.

### 3. Cross-architecture evidence

Minimum:

- x86-64 with AVX2 or AVX-512.
- ARM64 with NEON.

Stronger target:

- Existing older x86 AVX2 host.
- One modern x86 AVX2/AVX-512 host.
- One ARM64 NEON host.

Report CPU model, core/vCPU count, memory, OS, compiler, compiler flags, SIMD
support, repetitions, warmups, and confidence intervals. Do not generalize to
GPU, FPGA, or embedded deployment unless those measurements are added.

## Submission gate

Submit to JRTIP only after the manuscript includes a table that directly maps
each candidate and baseline to 30 FPS and 60 FPS feasibility at 720p and 1080p
on at least x86-64 and ARM64.
