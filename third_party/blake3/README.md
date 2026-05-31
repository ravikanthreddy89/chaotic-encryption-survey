Official BLAKE3 C sources vendored from:

https://github.com/BLAKE3-team/BLAKE3/tree/master/c

This project currently builds the portable official C implementation:

- `blake3.c`
- `blake3_dispatch.c`
- `blake3_portable.c`
- `blake3.h`
- `blake3_impl.h`

The upstream SIMD sources are intentionally not wired in this first stage so
the build remains simple and portable. Add the upstream SSE2/SSE4.1/AVX2/AVX512
or NEON files later if BLAKE3 hardware acceleration itself should be measured.
