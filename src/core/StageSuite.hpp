#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <functional>
#include <numeric>
#include <string>
#include <vector>

#if defined(__AVX2__)
#include <immintrin.h>
#endif

#include "core/IImageCipher.hpp"
#include "core/Utils.hpp"
#include "scalar/ChaoticCiphers.hpp"
#include "simd/avx2/Xor.hpp"

namespace bench {

struct StageRow {
    std::string category;
    std::string stage;
    std::string expected_speed;
    std::string research_value;
    size_t bytes = 0;
    double ms = 0.0;
    double mbps = 0.0;
    uint64_t checksum = 0;
};

struct CandidateRow {
    std::string scheme;
    std::string keystream;
    std::string permutation;
    std::string diffusion;
    size_t bytes = 0;
    double keygen_ms = 0.0;
    double permutation_ms = 0.0;
    double diffusion_ms = 0.0;
    double total_ms = 0.0;
    double mbps = 0.0;
    uint64_t checksum = 0;
};

inline uint64_t checksum64(const std::vector<uint8_t>& data) {
    uint64_t h = 0xCBF29CE484222325ULL;
    for (uint8_t b : data) {
        h ^= b;
        h *= 0x100000001B3ULL;
    }
    return h;
}

inline double mbps_for(size_t bytes, double ms) {
    if (ms <= 0.0) return 0.0;
    return (static_cast<double>(bytes) / (1024.0 * 1024.0)) / (ms / 1000.0);
}

inline std::vector<uint8_t> fixed_point_tent_bytes(size_t n, const CipherContext& ctx) {
    std::vector<uint8_t> out(n);
    uint32_t x = static_cast<uint32_t>(seed_from_key(ctx, 0x54454E54ULL));
    if (x == 0) x = 1;
    for (size_t i = 0; i < n; ++i) {
        x = (x < 0x80000000U) ? (x << 1U) : ((~x) << 1U);
        x ^= x >> 13U;
        out[i] = static_cast<uint8_t>(x >> 24U);
    }
    return out;
}

inline std::vector<uint8_t> cellular_automata_bytes(size_t n, const CipherContext& ctx) {
    std::vector<uint8_t> out(n);
    uint64_t s = seed_from_key(ctx, 0x4341554C45534554ULL);
    uint64_t state = splitmix64_next(s);
    for (size_t i = 0; i < n; ++i) {
        const uint64_t left = (state << 1U) | (state >> 63U);
        const uint64_t right = (state >> 1U) | (state << 63U);
        state = left ^ (state | right);  // rule-30-like update over 64 cells.
        out[i] = static_cast<uint8_t>(state >> ((i & 7U) * 8U));
    }
    return out;
}

inline std::vector<uint8_t> cml_bytes(size_t n, const CipherContext& ctx) {
    std::vector<uint8_t> out(n);
    constexpr size_t lanes = 16;
    double x[lanes];
    uint64_t seed = seed_from_key(ctx, 0xC011EDULL);
    for (double& v : x) {
        v = 0.001 + (static_cast<double>(splitmix64_next(seed) & 0xFFFFFU) / 0xFFFFFU) * 0.998;
    }
    for (size_t i = 0; i < n; ++i) {
        const size_t lane = i & (lanes - 1U);
        const double prev = x[(lane + lanes - 1U) & (lanes - 1U)];
        const double next = x[(lane + 1U) & (lanes - 1U)];
        const double local = 3.91 * x[lane] * (1.0 - x[lane]);
        x[lane] = std::fabs(local + 0.04 * (prev + next - 2.0 * x[lane]));
        x[lane] -= std::floor(x[lane]);
        out[i] = static_cast<uint8_t>(static_cast<int>(x[lane] * 256.0) & 0xFF);
    }
    return out;
}

inline std::vector<uint8_t> reaction_diffusion_bytes(size_t n, const CipherContext& ctx) {
    std::vector<uint8_t> out(n);
    constexpr size_t lanes = 64;
    float a[lanes], b[lanes];
    uint64_t seed = seed_from_key(ctx, 0xD1FF0510ULL);
    for (size_t i = 0; i < lanes; ++i) {
        a[i] = 0.5f + static_cast<float>(splitmix64_next(seed) & 0xFFU) / 512.0f;
        b[i] = static_cast<float>(splitmix64_next(seed) & 0xFFU) / 512.0f;
    }
    for (size_t i = 0; i < n; ++i) {
        const size_t j = i & (lanes - 1U);
        const size_t l = (j + lanes - 1U) & (lanes - 1U);
        const size_t r = (j + 1U) & (lanes - 1U);
        const float lap_a = a[l] + a[r] - 2.0f * a[j];
        const float lap_b = b[l] + b[r] - 2.0f * b[j];
        const float reaction = a[j] * b[j] * b[j];
        a[j] += 0.16f * lap_a - reaction + 0.035f * (1.0f - a[j]);
        b[j] += 0.08f * lap_b + reaction - 0.061f * b[j];
        a[j] = std::fabs(a[j] - std::floor(a[j]));
        b[j] = std::fabs(b[j] - std::floor(b[j]));
        out[i] = static_cast<uint8_t>((a[j] + b[j]) * 127.5f);
    }
    return out;
}

inline size_t gcd_size(size_t a, size_t b) {
    while (b != 0) {
        const size_t t = a % b;
        a = b;
        b = t;
    }
    return a;
}

inline std::vector<uint8_t> perm_chaotic_sort(const std::vector<uint8_t>& in, const CipherContext& ctx) {
    std::vector<double> scores(in.size());
    uint64_t seed = seed_from_key(ctx, 0x50524D31ULL);
    double x = 0.731;
    for (double& v : scores) {
        x = 3.99 * x * (1.0 - x);
        v = x + static_cast<double>(splitmix64_next(seed) & 0xFFU) * 1e-12;
    }
    std::vector<size_t> idx(in.size());
    std::iota(idx.begin(), idx.end(), 0U);
    std::stable_sort(idx.begin(), idx.end(), [&](size_t a, size_t b) { return scores[a] < scores[b]; });
    std::vector<uint8_t> out(in.size());
    for (size_t i = 0; i < idx.size(); ++i) out[idx[i]] = in[i];
    return out;
}

inline std::vector<uint8_t> perm_random_walk(const std::vector<uint8_t>& in, const CipherContext& ctx) {
    std::vector<uint8_t> out = in;
    uint64_t s = seed_from_key(ctx, 0x57414C4BULL);
    size_t pos = splitmix64_next(s) % out.size();
    for (size_t i = 0; i < out.size(); ++i) {
        pos = (pos + 1U + (splitmix64_next(s) % 31U)) % out.size();
        std::swap(out[i], out[pos]);
    }
    return out;
}

inline std::vector<uint8_t> perm_affine(const std::vector<uint8_t>& in) {
    std::vector<uint8_t> out(in.size());
    size_t a = 65537U;
    while (gcd_size(a, in.size()) != 1U) a += 2U;
    const size_t b = 17U;
    for (size_t i = 0; i < in.size(); ++i) out[(a * i + b) % in.size()] = in[i];
    return out;
}

inline std::vector<uint8_t> perm_feistel(const std::vector<uint8_t>& in, const CipherContext& ctx) {
    std::vector<uint8_t> out(in.size());
    uint64_t domain = 1;
    while (domain < in.size()) domain <<= 1U;
    const uint64_t mask = domain - 1U;
    const uint64_t seed = seed_from_key(ctx, 0xFE157E1ULL);
    const uint64_t a1 = (seed | 1ULL) & mask;
    const uint64_t a2 = ((seed >> 17U) | 1ULL) & mask;
    const uint64_t b1 = (seed >> 7U) & mask;
    const uint64_t b2 = (seed >> 29U) & mask;
    for (size_t i = 0; i < in.size(); ++i) {
        uint64_t p = i;
        do {
            p = (p * a1 + b1) & mask;
            p ^= (p >> 3U) ^ (seed & mask);
            p = (p * a2 + b2) & mask;
        } while (p >= in.size());
        out[p] = in[i];
    }
    return out;
}

inline std::vector<uint8_t> perm_block(const std::vector<uint8_t>& in) {
    constexpr size_t block = 256;
    std::vector<uint8_t> out(in.size());
    const size_t blocks = (in.size() + block - 1U) / block;
    for (size_t b = 0; b < blocks; ++b) {
        const size_t src = b * block;
        const size_t dst_block = (blocks - 1U - b);
        const size_t dst = dst_block * block;
        const size_t count = std::min(block, in.size() - src);
        std::copy_n(in.begin() + src, count, out.begin() + std::min(dst, in.size() - count));
    }
    return out;
}

inline std::vector<uint8_t> perm_checkerboard(const std::vector<uint8_t>& in) {
    std::vector<uint8_t> out = in;
    for (int round = 0; round < 4; ++round) {
        const size_t start = static_cast<size_t>(round & 1);
        for (size_t i = start; i + 1 < out.size(); i += 2) std::swap(out[i], out[i + 1]);
    }
    return out;
}

inline std::vector<uint8_t> perm_arnold_linear(const std::vector<uint8_t>& in, int width, int height, int channels) {
    std::vector<uint8_t> out(in.size());
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            const int nx = (x + y) % width;
            const int ny = (x + 2 * y) % height;
            const size_t src = (static_cast<size_t>(y) * width + x) * channels;
            const size_t dst = (static_cast<size_t>(ny) * width + nx) * channels;
            std::copy_n(in.begin() + src, channels, out.begin() + dst);
        }
    }
    return out;
}

inline std::vector<uint8_t> diffuse_global_chain(const std::vector<uint8_t>& in, const std::vector<uint8_t>& ks) {
    std::vector<uint8_t> out(in.size());
    uint8_t chain = 0;
    for (size_t i = 0; i < in.size(); ++i) {
        chain = static_cast<uint8_t>(in[i] ^ ks[i] ^ chain);
        out[i] = chain;
    }
    return out;
}

inline std::vector<uint8_t> diffuse_block_chain(const std::vector<uint8_t>& in, const std::vector<uint8_t>& ks) {
    std::vector<uint8_t> out(in.size());
    constexpr size_t block = 256;
    for (size_t start = 0; start < in.size(); start += block) {
        uint8_t chain = 0;
        const size_t end = std::min(in.size(), start + block);
        for (size_t i = start; i < end; ++i) {
            chain = static_cast<uint8_t>(in[i] ^ ks[i] ^ chain);
            out[i] = chain;
        }
    }
    return out;
}

inline std::vector<uint8_t> diffuse_parallel_prefix(const std::vector<uint8_t>& in, const std::vector<uint8_t>& ks) {
    std::vector<uint8_t> x;
    xor_bytes(in, ks, x, true);
    std::vector<uint8_t> out(x.size());
    constexpr size_t block = 1024;
    for (size_t start = 0; start < x.size(); start += block) {
        uint8_t acc = 0;
        const size_t end = std::min(x.size(), start + block);
        for (size_t i = start; i < end; ++i) {
            acc ^= x[i];
            out[i] = acc;
        }
    }
    return out;
}

#if defined(__AVX2__)
inline __m256i prefix_xor_32(__m256i v) {
    alignas(32) uint8_t bytes[32];
    _mm256_store_si256(reinterpret_cast<__m256i*>(bytes), v);
    for (int i = 1; i < 32; ++i) bytes[i] ^= bytes[i - 1];
    return _mm256_load_si256(reinterpret_cast<const __m256i*>(bytes));
}

inline uint8_t last_byte_32(__m256i v) {
    alignas(32) uint8_t bytes[32];
    _mm256_store_si256(reinterpret_cast<__m256i*>(bytes), v);
    return bytes[31];
}
#endif

inline std::vector<uint8_t> diffuse_prefix_xor_avx2(const std::vector<uint8_t>& in,
                                                    const std::vector<uint8_t>& ks,
                                                    bool reverse = false) {
    std::vector<uint8_t> out(in.size());
#if defined(__AVX2__)
    uint8_t carry = 0;
    if (!reverse) {
        size_t i = 0;
        for (; i + 32 <= in.size(); i += 32) {
            __m256i a = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(in.data() + i));
            __m256i b = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(ks.data() + i));
            __m256i x = _mm256_xor_si256(a, b);
            __m256i y = prefix_xor_32(x);
            y = _mm256_xor_si256(y, _mm256_set1_epi8(static_cast<char>(carry)));
            _mm256_storeu_si256(reinterpret_cast<__m256i*>(out.data() + i), y);
            carry = last_byte_32(y);
        }
        for (; i < in.size(); ++i) {
            carry = static_cast<uint8_t>(in[i] ^ ks[i] ^ carry);
            out[i] = carry;
        }
    } else {
        size_t remaining = in.size();
        while (remaining >= 32) {
            remaining -= 32;
            alignas(32) uint8_t ib[32], kb[32], rb[32];
            std::memcpy(ib, in.data() + remaining, 32);
            std::memcpy(kb, ks.data() + remaining, 32);
            std::reverse(ib, ib + 32);
            std::reverse(kb, kb + 32);
            __m256i a = _mm256_load_si256(reinterpret_cast<const __m256i*>(ib));
            __m256i b = _mm256_load_si256(reinterpret_cast<const __m256i*>(kb));
            __m256i y = prefix_xor_32(_mm256_xor_si256(a, b));
            y = _mm256_xor_si256(y, _mm256_set1_epi8(static_cast<char>(carry)));
            carry = last_byte_32(y);
            _mm256_store_si256(reinterpret_cast<__m256i*>(rb), y);
            std::reverse(rb, rb + 32);
            std::memcpy(out.data() + remaining, rb, 32);
        }
        for (size_t j = remaining; j > 0; --j) {
            const size_t i = j - 1;
            carry = static_cast<uint8_t>(in[i] ^ ks[i] ^ carry);
            out[i] = carry;
        }
    }
#else
    uint8_t carry = 0;
    if (!reverse) {
        for (size_t i = 0; i < in.size(); ++i) {
            carry = static_cast<uint8_t>(in[i] ^ ks[i] ^ carry);
            out[i] = carry;
        }
    } else {
        for (size_t j = in.size(); j > 0; --j) {
            const size_t i = j - 1;
            carry = static_cast<uint8_t>(in[i] ^ ks[i] ^ carry);
            out[i] = carry;
        }
    }
#endif
    return out;
}

inline std::vector<uint8_t> diffuse_multilane_chain_avx2(const std::vector<uint8_t>& in,
                                                         const std::vector<uint8_t>& ks) {
    std::vector<uint8_t> out(in.size());
#if defined(__AVX2__)
    __m256i state = _mm256_set1_epi8(0);
    size_t i = 0;
    for (; i + 32 <= in.size(); i += 32) {
        __m256i a = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(in.data() + i));
        __m256i b = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(ks.data() + i));
        state = _mm256_xor_si256(_mm256_xor_si256(a, b), state);
        _mm256_storeu_si256(reinterpret_cast<__m256i*>(out.data() + i), state);
    }
    alignas(32) uint8_t tail_state[32];
    _mm256_store_si256(reinterpret_cast<__m256i*>(tail_state), state);
    for (; i < in.size(); ++i) {
        const size_t lane = i & 31U;
        tail_state[lane] = static_cast<uint8_t>(in[i] ^ ks[i] ^ tail_state[lane]);
        out[i] = tail_state[lane];
    }
#else
    constexpr size_t width = 32;
    uint8_t state[width] = {};
    for (size_t i = 0; i < in.size(); ++i) {
        const size_t lane = i & (width - 1U);
        state[lane] = static_cast<uint8_t>(in[i] ^ ks[i] ^ state[lane]);
        out[i] = state[lane];
    }
#endif
    return out;
}

inline std::vector<uint8_t> diffuse_tree_xor_avx2(const std::vector<uint8_t>& in,
                                                 const std::vector<uint8_t>& ks) {
    std::vector<uint8_t> out(in.size());
    xor_bytes(in, ks, out, true);
#if defined(__AVX2__)
    size_t i = 0;
    for (; i + 32 <= out.size(); i += 32) {
        __m256i x = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(out.data() + i));
        x = _mm256_xor_si256(x, _mm256_slli_si256(x, 1));
        x = _mm256_xor_si256(x, _mm256_srli_si256(x, 1));
        x = _mm256_xor_si256(x, _mm256_slli_si256(x, 2));
        x = _mm256_xor_si256(x, _mm256_srli_si256(x, 2));
        x = _mm256_xor_si256(x, _mm256_slli_si256(x, 4));
        x = _mm256_xor_si256(x, _mm256_srli_si256(x, 4));
        x = _mm256_xor_si256(x, _mm256_permute2x128_si256(x, x, 1));
        _mm256_storeu_si256(reinterpret_cast<__m256i*>(out.data() + i), x);
    }
    for (; i < out.size(); ++i) out[i] ^= static_cast<uint8_t>(i * 131U);
#else
    for (size_t stride = 1; stride < 32; stride <<= 1U) {
        for (size_t i = 0; i < out.size(); ++i) out[i] ^= out[i ^ std::min(stride, out.size() - 1U)];
    }
#endif
    return out;
}

inline std::vector<uint8_t> diffuse_prefix_tree_reverse(const std::vector<uint8_t>& in,
                                                       const std::vector<uint8_t>& ks) {
    std::vector<uint8_t> a = diffuse_prefix_xor_avx2(in, ks, false);
    std::vector<uint8_t> b = diffuse_tree_xor_avx2(a, ks);
    return diffuse_prefix_xor_avx2(b, ks, true);
}

inline std::vector<uint8_t> diffuse_arx_prefix_mod256(const std::vector<uint8_t>& in,
                                                     const std::vector<uint8_t>& ks) {
    std::vector<uint8_t> out(in.size());
    uint8_t acc = 0;
    for (size_t i = 0; i < in.size(); ++i) {
        acc = static_cast<uint8_t>(acc + static_cast<uint8_t>(in[i] + ks[i]));
        const uint8_t r = static_cast<uint8_t>(ks[i] & 7U);
        out[i] = static_cast<uint8_t>(((acc << r) | (acc >> ((8U - r) & 7U))) ^ ks[i]);
    }
    return out;
}

inline std::vector<uint8_t> diffuse_stencil(const std::vector<uint8_t>& in,
                                            const std::vector<uint8_t>& ks,
                                            int width,
                                            int height,
                                            int channels) {
    std::vector<uint8_t> out(in.size());
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            for (int c = 0; c < channels; ++c) {
                const size_t i = (static_cast<size_t>(y) * width + x) * channels + c;
                const size_t l = (static_cast<size_t>(y) * width + ((x + width - 1) % width)) * channels + c;
                const size_t u = (static_cast<size_t>((y + height - 1) % height) * width + x) * channels + c;
                out[i] = static_cast<uint8_t>(in[i] ^ ks[i] ^ ((in[l] + in[u]) & 0xFFU) ^
                                              static_cast<uint8_t>(x + y + c));
            }
        }
    }
    return out;
}

inline uint32_t rotl32(uint32_t x, int r) {
    return (x << r) | (x >> (32 - r));
}

inline std::vector<uint8_t> diffuse_arx(const std::vector<uint8_t>& in, const std::vector<uint8_t>& ks) {
    std::vector<uint8_t> out(in.size());
    size_t i = 0;
    for (; i + 4 <= in.size(); i += 4) {
        uint32_t a = 0, b = 0;
        std::memcpy(&a, in.data() + i, 4);
        std::memcpy(&b, ks.data() + i, 4);
        uint32_t v = rotl32(a + b, 7) ^ (b + 0x9E3779B9U);
        std::memcpy(out.data() + i, &v, 4);
    }
    for (; i < in.size(); ++i) out[i] = in[i] ^ ks[i] ^ static_cast<uint8_t>(i);
    return out;
}

inline std::vector<uint8_t> diffuse_bitplane(const std::vector<uint8_t>& in, const std::vector<uint8_t>& ks) {
    std::vector<uint8_t> out(in.size());
    for (size_t i = 0; i < in.size(); ++i) {
        const uint8_t v = static_cast<uint8_t>((in[i] << 3U) | (in[i] >> 5U));
        out[i] = static_cast<uint8_t>(v ^ ks[i] ^ 0xA5U);
    }
    return out;
}

inline void add_stage_row(std::vector<StageRow>& rows,
                          const std::string& category,
                          const std::string& stage,
                          const std::string& expected,
                          const std::string& value,
                          size_t bytes,
                          double ms,
                          uint64_t checksum) {
    rows.push_back({category, stage, expected, value, bytes, ms, mbps_for(bytes, ms), checksum});
}

inline std::vector<StageRow> run_replaceable_stage_bench(const cv::Mat& img,
                                                         const CipherContext& ctx,
                                                         bool include_slow = true) {
    const std::vector<uint8_t> input = mat_to_bytes(img);
    const size_t n = input.size();
    std::vector<StageRow> rows;

    auto bench_ks = [&](const std::string& name, const std::string& expected, const std::string& value,
                        const std::function<std::vector<uint8_t>()>& fn) {
        Timer t;
        std::vector<uint8_t> out = fn();
        add_stage_row(rows, "keystream", name, expected, value, n, t.elapsed_ms(), checksum64(out));
    };
    if (include_slow) {
        bench_ks("logistic_double", "slow", "baseline", [&] { return make_keystream(n, ctx, ChaosFlavor::Logistic); });
        bench_ks("sine_map", "very slow", "bad baseline", [&] { return make_keystream(n, ctx, ChaosFlavor::Sine); });
    }
    bench_ks("fixed_point_tent", "high", "useful", [&] { return fixed_point_tent_bytes(n, ctx); });
    bench_ks("cml", "medium-high", "chaos-native SIMD", [&] { return cml_bytes(n, ctx); });
    bench_ks("cellular_automata", "very high", "fastest", [&] { return cellular_automata_bytes(n, ctx); });
    if (include_slow) {
        bench_ks("hamiltonian", "medium", "physics novelty", [&] { return make_keystream(n, ctx, ChaosFlavor::HamiltonianLattice); });
        bench_ks("reaction_diffusion", "low-medium", "physics novelty", [&] { return reaction_diffusion_bytes(n, ctx); });
    }

    auto bench_perm = [&](const std::string& name, const std::string& expected, const std::string& value,
                          const std::function<std::vector<uint8_t>()>& fn) {
        Timer t;
        std::vector<uint8_t> out = fn();
        add_stage_row(rows, "permutation", name, expected, value, n, t.elapsed_ms(), checksum64(out));
    };
    if (include_slow) {
        bench_perm("chaotic_sort", "O(N log N)", "poor", [&] { return perm_chaotic_sort(input, ctx); });
        bench_perm("random_walk", "O(N) sequential", "poor", [&] { return perm_random_walk(input, ctx); });
    }
    bench_perm("arnold_cat_map", "O(N)", "good", [&] { return perm_arnold_linear(input, img.cols, img.rows, img.channels()); });
    bench_perm("affine_modular", "O(N)", "excellent", [&] { return perm_affine(input); });
    bench_perm("feistel_index", "O(rN)", "excellent", [&] { return perm_feistel(input, ctx); });
    bench_perm("block_permutation", "O(N)", "excellent", [&] { return perm_block(input); });
    bench_perm("checkerboard_swaps", "O(rN)", "excellent", [&] { return perm_checkerboard(input); });

    std::vector<uint8_t> ks = cellular_automata_bytes(n, ctx);
    auto bench_diff = [&](const std::string& name, const std::string& expected, const std::string& value,
                          const std::function<std::vector<uint8_t>()>& fn) {
        Timer t;
        std::vector<uint8_t> out = fn();
        add_stage_row(rows, "diffusion", name, expected, value, n, t.elapsed_ms(), checksum64(out));
    };
    bench_diff("global_chain", "sequential", "poor", [&] { return diffuse_global_chain(input, ks); });
    bench_diff("block_local_chain", "block-level", "good", [&] { return diffuse_block_chain(input, ks); });
    bench_diff("parallel_prefix", "reduced", "medium", [&] { return diffuse_parallel_prefix(input, ks); });
    bench_diff("stencil_diffusion", "none per round", "high", [&] { return diffuse_stencil(input, ks, img.cols, img.rows, img.channels()); });
    bench_diff("arx_block_diffusion", "none", "excellent", [&] { return diffuse_arx(input, ks); });
    bench_diff("bitplane_diffusion", "none", "excellent", [&] { return diffuse_bitplane(input, ks); });
    bench_diff("prefix_xor_avx2", "associative scan", "excellent", [&] { return diffuse_prefix_xor_avx2(input, ks, false); });
    bench_diff("reverse_prefix_xor_avx2", "associative scan", "excellent", [&] { return diffuse_prefix_xor_avx2(input, ks, true); });
    bench_diff("multilane_chain_avx2", "lane-local", "excellent", [&] { return diffuse_multilane_chain_avx2(input, ks); });
    bench_diff("tree_xor_avx2", "logarithmic", "excellent", [&] { return diffuse_tree_xor_avx2(input, ks); });
    bench_diff("prefix_tree_reverse", "multi-pass associative", "excellent", [&] { return diffuse_prefix_tree_reverse(input, ks); });
    bench_diff("arx_prefix_mod256", "prefix sum mod 256", "medium-high", [&] { return diffuse_arx_prefix_mod256(input, ks); });
    return rows;
}

inline CandidateRow run_candidate(const std::string& scheme,
                                  const std::string& keygen,
                                  const std::string& permutation,
                                  const std::string& diffusion,
                                  const cv::Mat& img,
                                  const CipherContext& ctx) {
    const std::vector<uint8_t> input = mat_to_bytes(img);
    CandidateRow row;
    row.scheme = scheme;
    row.keystream = keygen;
    row.permutation = permutation;
    row.diffusion = diffusion;
    row.bytes = input.size();

    Timer total;
    Timer key_timer;
    std::vector<uint8_t> ks;
    if (keygen == "cellular_automata") ks = cellular_automata_bytes(input.size(), ctx);
    else if (keygen == "cml") ks = cml_bytes(input.size(), ctx);
    else if (keygen == "hamiltonian") ks = make_keystream(input.size(), ctx, ChaosFlavor::HamiltonianLattice);
    else ks = reaction_diffusion_bytes(input.size(), ctx);
    row.keygen_ms = key_timer.elapsed_ms();

    Timer perm_timer;
    std::vector<uint8_t> permuted;
    if (permutation == "feistel_index" || permutation == "block_feistel") permuted = perm_feistel(input, ctx);
    else if (permutation == "affine_bitplane") permuted = perm_affine(input);
    else if (permutation == "block_permutation") permuted = perm_block(input);
    else permuted = perm_checkerboard(input);
    row.permutation_ms = perm_timer.elapsed_ms();

    Timer diff_timer;
    std::vector<uint8_t> out;
    if (diffusion == "arx_block") out = diffuse_arx(permuted, ks);
    else if (diffusion == "stencil") out = diffuse_stencil(permuted, ks, img.cols, img.rows, img.channels());
    else if (diffusion == "prefix_tree_reverse") out = diffuse_prefix_tree_reverse(permuted, ks);
    else if (diffusion == "multilane_tree") {
        std::vector<uint8_t> a = diffuse_multilane_chain_avx2(permuted, ks);
        out = diffuse_tree_xor_avx2(a, ks);
    }
    else if (diffusion == "prefix_xor") out = diffuse_prefix_xor_avx2(permuted, ks, false);
    else out = diffuse_bitplane(permuted, ks);
    row.diffusion_ms = diff_timer.elapsed_ms();

    row.total_ms = total.elapsed_ms();
    row.mbps = mbps_for(row.bytes, row.total_ms);
    row.checksum = checksum64(out);
    return row;
}

inline std::vector<CandidateRow> run_candidate_stage_schemes(const cv::Mat& img,
                                                             const CipherContext& ctx,
                                                             bool include_slow = true) {
    std::vector<CandidateRow> rows = {
        run_candidate("CA-Feistel-ARX", "cellular_automata", "feistel_index", "arx_block", img, ctx),
        run_candidate("Checkerboard-CA-ARX", "cellular_automata", "checkerboard_swaps", "arx_block", img, ctx),
        run_candidate("Affine-CA-PrefixTree", "cellular_automata", "affine_bitplane", "prefix_tree_reverse", img, ctx),
        run_candidate("Checkerboard-CA-MultilaneTree", "cellular_automata", "checkerboard_swaps", "multilane_tree", img, ctx),
    };
    if (include_slow) {
        rows.push_back(run_candidate("CML-Feistel-Stencil", "cml", "feistel_index", "stencil", img, ctx));
        rows.push_back(run_candidate("Hamiltonian-Block-Stencil", "hamiltonian", "block_feistel", "stencil", img, ctx));
        rows.push_back(run_candidate("Affine-CML-Bitplane", "cml", "affine_bitplane", "bitplane", img, ctx));
    }
    return rows;
}

}  // namespace bench
