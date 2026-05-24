#pragma once

#include <array>
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <string>
#include <unordered_map>
#include <vector>

#include <openssl/sha.h>

#include "../bench_common.h"

#if defined(__SSE2__) && (defined(__x86_64__) || defined(__i386__))
#include <emmintrin.h>
#endif

namespace Ciphers {

#ifndef CHAOS_DIFFUSE_KERNEL
#define CHAOS_DIFFUSE_KERNEL 0
#endif

#ifndef CHAOS_DIFFUSE_TILE_BYTES
#define CHAOS_DIFFUSE_TILE_BYTES 16384
#endif

static inline double yeh_sin(double v) {
#if defined(YEH_USE_FAST_SIN) && YEH_USE_FAST_SIN == 1
    return static_cast<double>(std::sin(static_cast<float>(v)));
#else
    return std::sin(v);
#endif
}

static inline void derive_ic(const uint8_t key[32], double& x0, double& y0, double& r) {
    uint64_t xi = 0, yi = 0;
    std::memcpy(&xi, key, 8);
    std::memcpy(&yi, key + 8, 8);
    x0 = (double)(xi & 0x000FFFFFFFFFFFFFULL) / (double)0x000FFFFFFFFFFFFFULL;
    y0 = (double)(yi & 0x000FFFFFFFFFFFFFULL) / (double)0x000FFFFFFFFFFFFFULL;
    if (x0 < 0.001) x0 = 0.001;
    if (y0 < 0.001) y0 = 0.001;
    uint32_t ri = 0;
    std::memcpy(&ri, key + 16, 4);
    r = 3.9 + 0.1 * ((double)(ri & 0xFFFFFF) / (double)0xFFFFFF);
}

struct Logistic {
    double x, r;
    Logistic(double x0, double r_, int warmup = 500) : x(x0), r(r_) {
        for (int i = 0; i < warmup; ++i) step();
    }
    inline double next() { return step(); }
    inline uint8_t byte() { return (uint8_t)((int)(step() * 256.0) & 0xFF); }
private:
    inline double step() { x = r * x * (1.0 - x); return x; }
};

inline const char* diffusion_kernel_name() {
#if CHAOS_DIFFUSE_KERNEL == 1
    return "tiled_simd_prexor";
#else
    return "scalar_baseline";
#endif
}

inline void xor_bytes_simd_or_scalar(
        uint8_t* dst,
        const uint8_t* a,
        const uint8_t* b,
        size_t n) {
#if CHAOS_DIFFUSE_KERNEL == 1 && defined(__SSE2__) && (defined(__x86_64__) || defined(__i386__))
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

inline void diffuse_cbc3(
        const uint8_t* in,
        const uint8_t* ks,
        uint8_t* out,
        size_t total_bytes,
        uint8_t prev[3],
        std::vector<uint8_t>& scratch) {
#if CHAOS_DIFFUSE_KERNEL == 1
    size_t tile = static_cast<size_t>(CHAOS_DIFFUSE_TILE_BYTES);
    if (tile < 3) tile = 3;
    tile -= tile % 3;
    if (tile == 0) tile = 3;

    for (size_t start = 0; start < total_bytes; start += tile) {
        size_t chunk = std::min(tile, total_bytes - start);
        chunk -= chunk % 3;
        if (chunk == 0) chunk = total_bytes - start;
        scratch.resize(chunk);
        xor_bytes_simd_or_scalar(scratch.data(), in + start, ks + start, chunk);
        for (size_t i = 0; i < chunk; i += 3) {
            uint8_t er = scratch[i + 0] ^ prev[0];
            uint8_t eg = scratch[i + 1] ^ prev[1];
            uint8_t eb = scratch[i + 2] ^ prev[2];
            out[start + i + 0] = er;
            out[start + i + 1] = eg;
            out[start + i + 2] = eb;
            prev[0] = er; prev[1] = eg; prev[2] = eb;
        }
    }
#else
    for (size_t i = 0; i < total_bytes; i += 3) {
        uint8_t er = in[i + 0] ^ ks[i + 0] ^ prev[0];
        uint8_t eg = in[i + 1] ^ ks[i + 1] ^ prev[1];
        uint8_t eb = in[i + 2] ^ ks[i + 2] ^ prev[2];
        out[i + 0] = er; out[i + 1] = eg; out[i + 2] = eb;
        prev[0] = er; prev[1] = eg; prev[2] = eb;
    }
#endif
}

inline void undiffuse_cbc3(
        const uint8_t* in,
        const uint8_t* ks,
        uint8_t* out,
        size_t total_bytes,
        uint8_t prev[3],
        std::vector<uint8_t>& scratch) {
#if CHAOS_DIFFUSE_KERNEL == 1
    size_t tile = static_cast<size_t>(CHAOS_DIFFUSE_TILE_BYTES);
    if (tile < 3) tile = 3;
    tile -= tile % 3;
    if (tile == 0) tile = 3;

    for (size_t start = 0; start < total_bytes; start += tile) {
        size_t chunk = std::min(tile, total_bytes - start);
        chunk -= chunk % 3;
        if (chunk == 0) chunk = total_bytes - start;
        scratch.resize(chunk);
        xor_bytes_simd_or_scalar(scratch.data(), in + start, ks + start, chunk);
        for (size_t i = 0; i < chunk; i += 3) {
            uint8_t er = in[start + i + 0];
            uint8_t eg = in[start + i + 1];
            uint8_t eb = in[start + i + 2];
            out[start + i + 0] = scratch[i + 0] ^ prev[0];
            out[start + i + 1] = scratch[i + 1] ^ prev[1];
            out[start + i + 2] = scratch[i + 2] ^ prev[2];
            prev[0] = er; prev[1] = eg; prev[2] = eb;
        }
    }
#else
    for (size_t i = 0; i < total_bytes; i += 3) {
        uint8_t er = in[i + 0];
        uint8_t eg = in[i + 1];
        uint8_t eb = in[i + 2];
        out[i + 0] = er ^ ks[i + 0] ^ prev[0];
        out[i + 1] = eg ^ ks[i + 1] ^ prev[1];
        out[i + 2] = eb ^ ks[i + 2] ^ prev[2];
        prev[0] = er; prev[1] = eg; prev[2] = eb;
    }
#endif
}

inline void diffuse_chain(
        const uint8_t* in,
        const uint8_t* ks,
        uint8_t* out,
        size_t total_bytes,
        uint8_t& chain,
        std::vector<uint8_t>& scratch) {
#if CHAOS_DIFFUSE_KERNEL == 1
    size_t tile = static_cast<size_t>(CHAOS_DIFFUSE_TILE_BYTES);
    if (tile == 0) tile = 1;
    for (size_t start = 0; start < total_bytes; start += tile) {
        size_t chunk = std::min(tile, total_bytes - start);
        scratch.resize(chunk);
        xor_bytes_simd_or_scalar(scratch.data(), in + start, ks + start, chunk);
        for (size_t i = 0; i < chunk; ++i) {
            uint8_t e = scratch[i] ^ chain;
            out[start + i] = e;
            chain = e;
        }
    }
#else
    for (size_t i = 0; i < total_bytes; ++i) {
        uint8_t e = in[i] ^ ks[i] ^ chain;
        out[i] = e;
        chain = e;
    }
#endif
}

inline void undiffuse_chain(
        const uint8_t* in,
        const uint8_t* ks,
        uint8_t* out,
        size_t total_bytes,
        uint8_t& chain,
        std::vector<uint8_t>& scratch) {
#if CHAOS_DIFFUSE_KERNEL == 1
    size_t tile = static_cast<size_t>(CHAOS_DIFFUSE_TILE_BYTES);
    if (tile == 0) tile = 1;
    for (size_t start = 0; start < total_bytes; start += tile) {
        size_t chunk = std::min(tile, total_bytes - start);
        scratch.resize(chunk);
        xor_bytes_simd_or_scalar(scratch.data(), in + start, ks + start, chunk);
        for (size_t i = 0; i < chunk; ++i) {
            uint8_t e = in[start + i];
            out[start + i] = scratch[i] ^ chain;
            chain = e;
        }
    }
#else
    for (size_t i = 0; i < total_bytes; ++i) {
        uint8_t e = in[i];
        out[i] = e ^ ks[i] ^ chain;
        chain = e;
    }
#endif
}

namespace ChaosProfile {

enum class Stage {
    KeyDerive,
    SeqGen,
    SortPerm,
    PermuteApply,
    Diffuse,
    Reverse,
};

struct StageStats {
    uint64_t ns = 0;
    uint64_t calls = 0;
};

inline const char* stage_name(Stage s) {
    switch (s) {
        case Stage::KeyDerive:   return "K_key_derive";
        case Stage::SeqGen:      return "S_seq_gen";
        case Stage::SortPerm:    return "P_sort_perm_idx";
        case Stage::PermuteApply:return "M_permute_apply";
        case Stage::Diffuse:     return "D_diffuse";
        case Stage::Reverse:     return "R_reverse";
    }
    return "unknown";
}

inline std::unordered_map<std::string, std::array<StageStats, 6>>& db() {
    static std::unordered_map<std::string, std::array<StageStats, 6>> s;
    return s;
}

struct Scope {
    const char* cipher;
    Stage stage;
    uint64_t t0;
    Scope(const char* c, Stage s) : cipher(c), stage(s), t0(Bench::now_ns()) {}
    ~Scope() {
        auto dt = Bench::now_ns() - t0;
        auto& bucket = db()[cipher][(size_t)stage];
        bucket.ns += dt;
        bucket.calls += 1;
    }
};

inline void dump_csv(const std::string& path) {
    std::ofstream f(path);
    if (!f) return;
    f << "diffusion_kernel,cipher,stage,total_ns,calls,avg_ns\n";
    for (const auto& kv : db()) {
        const auto& cipher = kv.first;
        const auto& arr = kv.second;
        for (size_t i = 0; i < arr.size(); ++i) {
            const auto& s = arr[i];
            if (s.calls == 0) continue;
            f << diffusion_kernel_name() << ',' << cipher << ',' << stage_name((Stage)i) << ',' << s.ns << ',' << s.calls << ','
              << (double)s.ns / (double)s.calls << "\n";
        }
    }
}

} // namespace ChaosProfile

} // namespace Ciphers
