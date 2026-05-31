#pragma once

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <vector>

#if !defined(CHAOSREF_DISABLE_SIMD) && \
    (defined(__x86_64__) || defined(__i386__) || defined(_M_X64) || defined(_M_IX86))
#define CHAOSREF_RNG_X86 1
#endif

#if defined(CHAOSREF_RNG_X86) && defined(__SSE2__)
#include <emmintrin.h>
#endif

#if defined(CHAOSREF_RNG_X86) && defined(CHAOSREF_ENABLE_AVX2) && defined(__AVX2__)
#include <immintrin.h>
#endif

namespace chaosref {

inline uint64_t splitmix64(uint64_t& x) {
    uint64_t z = (x += 0x9E3779B97F4A7C15ULL);
    z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
    z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;
    return z ^ (z >> 31);
}

inline void derive_logistic_params(uint64_t key, double& x0, double& r) {
    uint64_t s = key;
    uint64_t a = splitmix64(s);
    uint64_t b = splitmix64(s);

    const double u = static_cast<double>(a & 0x000FFFFFFFFFFFFFULL) /
                     static_cast<double>(0x000FFFFFFFFFFFFFULL);
    const double v = static_cast<double>(b & 0x00FFFFFFULL) /
                     static_cast<double>(0x00FFFFFFULL);

    x0 = 0.001 + 0.998 * u;
    r = 3.9 + 0.1 * v;
}

struct Logistic {
    double x = 0.5;
    double r = 3.99;

    Logistic(double x0, double rr, int warmup = 500) : x(x0), r(rr) {
        for (int i = 0; i < warmup; ++i) step();
    }

    inline double step() {
        x = r * x * (1.0 - x);
        if (x <= 0.0) x = 1e-12;
        if (x >= 1.0) x = 1.0 - 1e-12;
        return x;
    }

    inline uint8_t byte() {
        return static_cast<uint8_t>(static_cast<int>(step() * 256.0) & 0xFF);
    }
};

inline void fill_logistic_bytes(std::vector<uint8_t>& out, uint64_t key_seed) {
    double x0 = 0.0, r = 0.0;
    derive_logistic_params(key_seed, x0, r);
    Logistic gen(x0, r);
    for (auto& b : out) b = gen.byte();
}

inline void fill_logistic_scores(std::vector<double>& out, uint64_t key_seed) {
    double x0 = 0.0, r = 0.0;
    derive_logistic_params(key_seed, x0, r);
    Logistic gen(x0, r);
    for (auto& v : out) v = gen.step();
}

inline uint32_t counter_mix32(uint32_t x, uint32_t seed) {
    x += seed + 0x9E3779B9U;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    x += seed ^ 0x85EBCA6BU;
    x ^= x >> 16;
    return x;
}

inline void fill_counter_mix_bytes(std::vector<uint8_t>& out, uint64_t key_seed) {
    const uint32_t seed_lo = static_cast<uint32_t>(key_seed);
    const uint32_t seed_hi = static_cast<uint32_t>(key_seed >> 32);
    const uint32_t seed = seed_lo ^ (seed_hi + 0xA5A5A5A5U);

    const size_t words = out.size() / sizeof(uint32_t);
    size_t i = 0;

#if defined(CHAOSREF_RNG_X86) && defined(CHAOSREF_ENABLE_AVX2) && defined(__AVX2__)
    const __m256i v_seed1 = _mm256_set1_epi32(static_cast<int>(seed + 0x9E3779B9U));
    const __m256i v_seed2 = _mm256_set1_epi32(static_cast<int>(seed ^ 0x85EBCA6BU));
    const __m256i v_step = _mm256_set1_epi32(8);
    __m256i v_idx = _mm256_setr_epi32(0, 1, 2, 3, 4, 5, 6, 7);
    for (; i + 8 <= words; i += 8) {
        __m256i x = _mm256_add_epi32(v_idx, v_seed1);
        x = _mm256_xor_si256(x, _mm256_slli_epi32(x, 13));
        x = _mm256_xor_si256(x, _mm256_srli_epi32(x, 17));
        x = _mm256_xor_si256(x, _mm256_slli_epi32(x, 5));
        x = _mm256_add_epi32(x, v_seed2);
        x = _mm256_xor_si256(x, _mm256_srli_epi32(x, 16));
        _mm256_storeu_si256(reinterpret_cast<__m256i*>(out.data() + i * sizeof(uint32_t)), x);
        v_idx = _mm256_add_epi32(v_idx, v_step);
    }
#elif defined(CHAOSREF_RNG_X86) && defined(__SSE2__)
    const __m128i v_seed1 = _mm_set1_epi32(static_cast<int>(seed + 0x9E3779B9U));
    const __m128i v_seed2 = _mm_set1_epi32(static_cast<int>(seed ^ 0x85EBCA6BU));
    const __m128i v_step = _mm_set1_epi32(4);
    __m128i v_idx = _mm_setr_epi32(0, 1, 2, 3);
    for (; i + 4 <= words; i += 4) {
        __m128i x = _mm_add_epi32(v_idx, v_seed1);
        x = _mm_xor_si128(x, _mm_slli_epi32(x, 13));
        x = _mm_xor_si128(x, _mm_srli_epi32(x, 17));
        x = _mm_xor_si128(x, _mm_slli_epi32(x, 5));
        x = _mm_add_epi32(x, v_seed2);
        x = _mm_xor_si128(x, _mm_srli_epi32(x, 16));
        _mm_storeu_si128(reinterpret_cast<__m128i*>(out.data() + i * sizeof(uint32_t)), x);
        v_idx = _mm_add_epi32(v_idx, v_step);
    }
#endif

    for (; i < words; ++i) {
        const uint32_t x = counter_mix32(static_cast<uint32_t>(i), seed);
        std::memcpy(out.data() + i * sizeof(uint32_t), &x, sizeof(x));
    }

    const size_t done = words * sizeof(uint32_t);
    if (done < out.size()) {
        uint32_t tail = counter_mix32(static_cast<uint32_t>(words), seed);
        std::memcpy(out.data() + done, &tail, out.size() - done);
    }
}

}  // namespace chaosref
