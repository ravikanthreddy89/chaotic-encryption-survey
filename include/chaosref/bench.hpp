#pragma once

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include "chaosref/schemes.hpp"

namespace chaosref {

struct BenchCase {
    std::string name;
    SchemeConfig cfg;
};

struct BenchResult {
    std::string name;
    std::string simd_backend;
    std::string shape;
    size_t bytes = 0;
    int warmup = 0;
    int iters = 0;

    double enc_ms = 0.0;
    double dec_ms = 0.0;
    double enc_mb_s = 0.0;
    double dec_mb_s = 0.0;

    bool correct = false;
};

inline double ns_to_ms(uint64_t ns) {
    return static_cast<double>(ns) / 1.0e6;
}

inline double throughput_mb_s(size_t bytes, int iters, double ms) {
    if (ms <= 0.0) return 0.0;
    const double total_mb = (static_cast<double>(bytes) * iters) / (1024.0 * 1024.0);
    return total_mb / (ms / 1000.0);
}

inline BenchResult run_bench_case(const Image& plain,
                                  const BenchCase& bc,
                                  int warmup,
                                  int iters) {
    for (int i = 0; i < warmup; ++i) {
        Image c = encrypt_image(plain, bc.cfg);
        Image r = decrypt_image(c, bc.cfg);
        (void)r;
    }

    uint64_t enc_ns = 0;
    uint64_t dec_ns = 0;

    Image last_cipher;
    Image last_recovered;

    for (int i = 0; i < iters; ++i) {
        auto t0 = std::chrono::steady_clock::now();
        Image c = encrypt_image(plain, bc.cfg);
        auto t1 = std::chrono::steady_clock::now();
        Image r = decrypt_image(c, bc.cfg);
        auto t2 = std::chrono::steady_clock::now();

        enc_ns += static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::nanoseconds>(t1 - t0).count());
        dec_ns += static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1).count());

        if (i + 1 == iters) {
            last_cipher = std::move(c);
            last_recovered = std::move(r);
        }
    }

    BenchResult br;
    br.name = bc.name;
    br.simd_backend = simd_backend_name();
    br.shape = shape_string(plain);
    br.bytes = plain.bytes();
    br.warmup = warmup;
    br.iters = iters;
    br.enc_ms = ns_to_ms(enc_ns);
    br.dec_ms = ns_to_ms(dec_ns);
    br.enc_mb_s = throughput_mb_s(plain.bytes(), iters, br.enc_ms);
    br.dec_mb_s = throughput_mb_s(plain.bytes(), iters, br.dec_ms);
    br.correct = equal_image(plain, last_recovered) && !last_cipher.empty();
    return br;
}

inline std::string csv_header() {
    return "case,simd_backend,shape,bytes,warmup,iters,enc_ms,dec_ms,enc_MBps,dec_MBps,correct\n";
}

inline std::string to_csv(const BenchResult& r) {
    return r.name + "," +
           r.simd_backend + "," +
           r.shape + "," +
           std::to_string(r.bytes) + "," +
           std::to_string(r.warmup) + "," +
           std::to_string(r.iters) + "," +
           std::to_string(r.enc_ms) + "," +
           std::to_string(r.dec_ms) + "," +
           std::to_string(r.enc_mb_s) + "," +
           std::to_string(r.dec_mb_s) + "," +
           (r.correct ? "1" : "0") + "\n";
}

}  // namespace chaosref
