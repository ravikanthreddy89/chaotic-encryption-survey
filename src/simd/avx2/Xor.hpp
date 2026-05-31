#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#if defined(__AVX2__)
#include <immintrin.h>
#endif

namespace bench {

inline bool avx2_compiled() {
#if defined(__AVX2__)
    return true;
#else
    return false;
#endif
}

inline void xor_bytes_scalar(const uint8_t* a, const uint8_t* b, uint8_t* out, size_t n) {
    for (size_t i = 0; i < n; ++i) out[i] = a[i] ^ b[i];
}

inline void xor_bytes_avx2(const uint8_t* a, const uint8_t* b, uint8_t* out, size_t n) {
#if defined(__AVX2__)
    size_t i = 0;
    for (; i + 32 <= n; i += 32) {
        __m256i va = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(a + i));
        __m256i vb = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(b + i));
        _mm256_storeu_si256(reinterpret_cast<__m256i*>(out + i), _mm256_xor_si256(va, vb));
    }
    for (; i < n; ++i) out[i] = a[i] ^ b[i];
#else
    xor_bytes_scalar(a, b, out, n);
#endif
}

inline void xor_bytes(const std::vector<uint8_t>& a,
                      const std::vector<uint8_t>& b,
                      std::vector<uint8_t>& out,
                      bool use_avx2) {
    out.resize(a.size());
    if (use_avx2 && avx2_compiled()) {
        xor_bytes_avx2(a.data(), b.data(), out.data(), a.size());
    } else {
        xor_bytes_scalar(a.data(), b.data(), out.data(), a.size());
    }
}

}  // namespace bench
