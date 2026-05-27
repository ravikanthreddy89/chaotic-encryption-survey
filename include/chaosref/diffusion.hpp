#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <vector>

#if (defined(__x86_64__) || defined(__i386__) || defined(_M_X64) || defined(_M_IX86))
#define CHAOSREF_X86 1
#endif

#if defined(CHAOSREF_X86) && defined(__SSE2__)
#include <emmintrin.h>
#endif

#if defined(CHAOSREF_X86) && defined(CHAOSREF_ENABLE_AVX2) && defined(__AVX2__)
#include <immintrin.h>
#endif

namespace chaosref {

enum class DiffusionKernel {
    ScalarChain,
    ScanExact
};

inline const char* diffusion_kernel_name(DiffusionKernel k) {
    return (k == DiffusionKernel::ScalarChain) ? "scalar_chain" : "scan_exact";
}

inline const char* simd_backend_name() {
#if defined(CHAOSREF_X86) && defined(CHAOSREF_ENABLE_AVX2) && defined(__AVX2__)
    return "avx2";
#elif defined(CHAOSREF_X86) && defined(__SSE2__)
    return "sse2";
#else
    return "scalar";
#endif
}

inline void xor_bytes(uint8_t* dst, const uint8_t* a, const uint8_t* b, size_t n) {
#if defined(CHAOSREF_X86) && defined(CHAOSREF_ENABLE_AVX2) && defined(__AVX2__)
    size_t i = 0;
    for (; i + 32 <= n; i += 32) {
        __m256i va = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(a + i));
        __m256i vb = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(b + i));
        __m256i vx = _mm256_xor_si256(va, vb);
        _mm256_storeu_si256(reinterpret_cast<__m256i*>(dst + i), vx);
    }
    for (; i < n; ++i) dst[i] = a[i] ^ b[i];
#elif defined(CHAOSREF_X86) && defined(__SSE2__)
    size_t i = 0;
    for (; i + 16 <= n; i += 16) {
        __m128i va = _mm_loadu_si128(reinterpret_cast<const __m128i*>(a + i));
        __m128i vb = _mm_loadu_si128(reinterpret_cast<const __m128i*>(b + i));
        __m128i vx = _mm_xor_si128(va, vb);
        _mm_storeu_si128(reinterpret_cast<__m128i*>(dst + i), vx);
    }
    for (; i < n; ++i) dst[i] = a[i] ^ b[i];
#else
    for (size_t i = 0; i < n; ++i) dst[i] = a[i] ^ b[i];
#endif
}

#if defined(CHAOSREF_X86) && defined(__SSE2__)
inline __m128i prefix_xor_16(__m128i v) {
    v = _mm_xor_si128(v, _mm_slli_si128(v, 1));
    v = _mm_xor_si128(v, _mm_slli_si128(v, 2));
    v = _mm_xor_si128(v, _mm_slli_si128(v, 4));
    v = _mm_xor_si128(v, _mm_slli_si128(v, 8));
    return v;
}

inline uint8_t last_byte_16(__m128i v) {
    __m128i hi = _mm_srli_si128(v, 15);
    return static_cast<uint8_t>(_mm_cvtsi128_si32(hi) & 0xFF);
}
#endif

inline void diffuse_scalar_chain(const uint8_t* in,
                                 const uint8_t* ks,
                                 uint8_t* out,
                                 size_t n,
                                 uint8_t init_chain = 0) {
    uint8_t chain = init_chain;
    for (size_t i = 0; i < n; ++i) {
        uint8_t e = in[i] ^ ks[i] ^ chain;
        out[i] = e;
        chain = e;
    }
}

inline void undiffuse_scalar_chain(const uint8_t* in,
                                   const uint8_t* ks,
                                   uint8_t* out,
                                   size_t n,
                                   uint8_t init_chain = 0) {
    uint8_t chain = init_chain;
    for (size_t i = 0; i < n; ++i) {
        uint8_t e = in[i];
        out[i] = e ^ ks[i] ^ chain;
        chain = e;
    }
}

inline void diffuse_scan_exact(const uint8_t* in,
                               const uint8_t* ks,
                               uint8_t* out,
                               size_t n,
                               uint8_t init_chain = 0,
                               size_t tile_bytes = 16384) {
    if (tile_bytes == 0) tile_bytes = 1;
    const size_t blocks = (n + tile_bytes - 1) / tile_bytes;

    std::vector<uint8_t> block_tail(blocks, 0);
    std::vector<uint8_t> block_seed(blocks, 0);
    std::vector<uint8_t> scratch;

    for (size_t b = 0; b < blocks; ++b) {
        const size_t start = b * tile_bytes;
        const size_t end = std::min(n, start + tile_bytes);
        const size_t chunk = end - start;
        scratch.resize(chunk);
        xor_bytes(scratch.data(), in + start, ks + start, chunk);

        uint8_t local = 0;
#if defined(CHAOSREF_X86) && defined(__SSE2__)
        size_t i = 0;
        for (; i + 16 <= chunk; i += 16) {
            __m128i vx = _mm_loadu_si128(reinterpret_cast<const __m128i*>(scratch.data() + i));
            __m128i vp = prefix_xor_16(vx);
            if (local != 0) {
                __m128i vc = _mm_set1_epi8(static_cast<char>(local));
                vp = _mm_xor_si128(vp, vc);
            }
            _mm_storeu_si128(reinterpret_cast<__m128i*>(out + start + i), vp);
            local = last_byte_16(vp);
        }
        for (; i < chunk; ++i) {
            local ^= scratch[i];
            out[start + i] = local;
        }
#else
        for (size_t i = 0; i < chunk; ++i) {
            local ^= scratch[i];
            out[start + i] = local;
        }
#endif
        block_tail[b] = local;
    }

    uint8_t carry = init_chain;
    for (size_t b = 0; b < blocks; ++b) {
        block_seed[b] = carry;
        carry ^= block_tail[b];
    }

    for (size_t b = 0; b < blocks; ++b) {
        const size_t start = b * tile_bytes;
        const size_t end = std::min(n, start + tile_bytes);
        const uint8_t seed = block_seed[b];
        for (size_t i = start; i < end; ++i) out[i] ^= seed;
    }
}

inline void undiffuse_scan_exact(const uint8_t* in,
                                 const uint8_t* ks,
                                 uint8_t* out,
                                 size_t n,
                                 uint8_t init_chain = 0,
                                 size_t tile_bytes = 16384) {
    (void)tile_bytes;
    std::vector<uint8_t> s(n);
    uint8_t prev = init_chain;
    for (size_t i = 0; i < n; ++i) {
        const uint8_t c = in[i];
        s[i] = c ^ prev;
        prev = c;
    }
    xor_bytes(out, s.data(), ks, n);
}

inline void diffuse(const uint8_t* in,
                    const uint8_t* ks,
                    uint8_t* out,
                    size_t n,
                    DiffusionKernel kernel,
                    uint8_t init_chain = 0) {
    if (kernel == DiffusionKernel::ScalarChain) {
        diffuse_scalar_chain(in, ks, out, n, init_chain);
    } else {
        diffuse_scan_exact(in, ks, out, n, init_chain);
    }
}

inline void undiffuse(const uint8_t* in,
                      const uint8_t* ks,
                      uint8_t* out,
                      size_t n,
                      DiffusionKernel kernel,
                      uint8_t init_chain = 0) {
    if (kernel == DiffusionKernel::ScalarChain) {
        undiffuse_scalar_chain(in, ks, out, n, init_chain);
    } else {
        undiffuse_scan_exact(in, ks, out, n, init_chain);
    }
}

}  // namespace chaosref
