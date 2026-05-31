#pragma once

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <numeric>
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

struct StageBenchResult {
    std::string name;
    std::string simd_backend;
    std::string shape;
    size_t bytes = 0;
    int warmup = 0;
    int iters = 0;
    std::string stage;
    double ms = 0.0;
    double mb_s = 0.0;
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

inline uint64_t elapsed_ns(std::chrono::steady_clock::time_point start,
                           std::chrono::steady_clock::time_point end) {
    return static_cast<uint64_t>(
        std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count());
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

inline void append_stage(std::vector<StageBenchResult>& out,
                         const BenchCase& bc,
                         const Image& plain,
                         int warmup,
                         int iters,
                         const std::string& stage,
                         uint64_t ns,
                         bool correct) {
    StageBenchResult r;
    r.name = bc.name;
    r.simd_backend = simd_backend_name();
    r.shape = shape_string(plain);
    r.bytes = plain.bytes();
    r.warmup = warmup;
    r.iters = iters;
    r.stage = stage;
    r.ms = ns_to_ms(ns);
    r.mb_s = throughput_mb_s(plain.bytes(), iters, r.ms);
    r.correct = correct;
    out.push_back(std::move(r));
}

inline std::vector<StageBenchResult> run_stage_bench_case(const Image& plain,
                                                          const BenchCase& bc,
                                                          int warmup,
                                                          int iters) {
    for (int i = 0; i < warmup; ++i) {
        Image c = encrypt_image(plain, bc.cfg);
        Image r = decrypt_image(c, bc.cfg);
        (void)r;
    }

    uint64_t enc_total_ns = 0;
    uint64_t dec_total_ns = 0;
    uint64_t enc_permute_map_ns = 0;
    uint64_t enc_sort_scores_ns = 0;
    uint64_t enc_sort_indices_ns = 0;
    uint64_t enc_permute_apply_ns = 0;
    uint64_t enc_keystream_ns = 0;
    uint64_t enc_diffuse_ns = 0;
    uint64_t dec_keystream_ns = 0;
    uint64_t dec_undiffuse_ns = 0;
    uint64_t dec_inverse_map_ns = 0;
    uint64_t dec_sort_scores_ns = 0;
    uint64_t dec_sort_indices_ns = 0;
    uint64_t dec_inverse_apply_ns = 0;

    Image last_cipher;
    Image last_recovered;

    for (int iter = 0; iter < iters; ++iter) {
        auto enc_start = std::chrono::steady_clock::now();

        Image permuted;
        std::vector<uint32_t> enc_idx;

        if (bc.cfg.perm_kind == PermutationKind::MapFridrich) {
            auto t0 = std::chrono::steady_clock::now();
            permuted = permute_map_fridrich(plain, bc.cfg.map_rounds);
            auto t1 = std::chrono::steady_clock::now();
            enc_permute_map_ns += elapsed_ns(t0, t1);
        } else {
            std::vector<double> scores(static_cast<size_t>(plain.pixels()));
            auto t0 = std::chrono::steady_clock::now();
            fill_logistic_scores(scores, bc.cfg.perm_seed);
            auto t1 = std::chrono::steady_clock::now();
            enc_sort_scores_ns += elapsed_ns(t0, t1);

            enc_idx.resize(static_cast<size_t>(plain.pixels()));
            auto t2 = std::chrono::steady_clock::now();
            std::iota(enc_idx.begin(), enc_idx.end(), 0U);
            std::stable_sort(enc_idx.begin(), enc_idx.end(), [&](uint32_t a, uint32_t b) {
                return scores[a] < scores[b];
            });
            auto t3 = std::chrono::steady_clock::now();
            enc_sort_indices_ns += elapsed_ns(t2, t3);

            auto t4 = std::chrono::steady_clock::now();
            permuted = permute_sort(plain, enc_idx);
            auto t5 = std::chrono::steady_clock::now();
            enc_permute_apply_ns += elapsed_ns(t4, t5);
        }

        std::vector<uint8_t> ks(permuted.bytes());
        auto t6 = std::chrono::steady_clock::now();
        fill_logistic_bytes(ks, bc.cfg.diff_seed);
        auto t7 = std::chrono::steady_clock::now();
        enc_keystream_ns += elapsed_ns(t6, t7);

        Image cipher = permuted;
        auto t8 = std::chrono::steady_clock::now();
        diffuse(permuted.data.data(), ks.data(), cipher.data.data(), cipher.bytes(), bc.cfg.diff_kernel, 0);
        auto t9 = std::chrono::steady_clock::now();
        enc_diffuse_ns += elapsed_ns(t8, t9);

        auto enc_end = std::chrono::steady_clock::now();
        enc_total_ns += elapsed_ns(enc_start, enc_end);

        auto dec_start = std::chrono::steady_clock::now();

        Image undiff = cipher;
        std::vector<uint8_t> dks(cipher.bytes());
        auto t10 = std::chrono::steady_clock::now();
        fill_logistic_bytes(dks, bc.cfg.diff_seed);
        auto t11 = std::chrono::steady_clock::now();
        dec_keystream_ns += elapsed_ns(t10, t11);

        auto t12 = std::chrono::steady_clock::now();
        undiffuse(cipher.data.data(), dks.data(), undiff.data.data(), undiff.bytes(), bc.cfg.diff_kernel, 0);
        auto t13 = std::chrono::steady_clock::now();
        dec_undiffuse_ns += elapsed_ns(t12, t13);

        Image recovered;
        if (bc.cfg.perm_kind == PermutationKind::MapFridrich) {
            auto t14 = std::chrono::steady_clock::now();
            recovered = invert_permute_map_fridrich(undiff, bc.cfg.map_rounds);
            auto t15 = std::chrono::steady_clock::now();
            dec_inverse_map_ns += elapsed_ns(t14, t15);
        } else {
            std::vector<double> scores(static_cast<size_t>(cipher.pixels()));
            auto t14 = std::chrono::steady_clock::now();
            fill_logistic_scores(scores, bc.cfg.perm_seed);
            auto t15 = std::chrono::steady_clock::now();
            dec_sort_scores_ns += elapsed_ns(t14, t15);

            std::vector<uint32_t> dec_idx(static_cast<size_t>(cipher.pixels()));
            auto t16 = std::chrono::steady_clock::now();
            std::iota(dec_idx.begin(), dec_idx.end(), 0U);
            std::stable_sort(dec_idx.begin(), dec_idx.end(), [&](uint32_t a, uint32_t b) {
                return scores[a] < scores[b];
            });
            auto t17 = std::chrono::steady_clock::now();
            dec_sort_indices_ns += elapsed_ns(t16, t17);

            auto t18 = std::chrono::steady_clock::now();
            recovered = invert_permute_sort(undiff, dec_idx);
            auto t19 = std::chrono::steady_clock::now();
            dec_inverse_apply_ns += elapsed_ns(t18, t19);
        }

        auto dec_end = std::chrono::steady_clock::now();
        dec_total_ns += elapsed_ns(dec_start, dec_end);

        if (iter + 1 == iters) {
            last_cipher = std::move(cipher);
            last_recovered = std::move(recovered);
        }
    }

    const bool correct = equal_image(plain, last_recovered) && !last_cipher.empty();
    std::vector<StageBenchResult> out;
    append_stage(out, bc, plain, warmup, iters, "encrypt_total", enc_total_ns, correct);
    append_stage(out, bc, plain, warmup, iters, "decrypt_total", dec_total_ns, correct);
    if (bc.cfg.perm_kind == PermutationKind::MapFridrich) {
        append_stage(out, bc, plain, warmup, iters, "encrypt_permute_map", enc_permute_map_ns, correct);
        append_stage(out, bc, plain, warmup, iters, "decrypt_inverse_map", dec_inverse_map_ns, correct);
    } else {
        append_stage(out, bc, plain, warmup, iters, "encrypt_sort_scores", enc_sort_scores_ns, correct);
        append_stage(out, bc, plain, warmup, iters, "encrypt_sort_indices", enc_sort_indices_ns, correct);
        append_stage(out, bc, plain, warmup, iters, "encrypt_permute_apply", enc_permute_apply_ns, correct);
        append_stage(out, bc, plain, warmup, iters, "decrypt_sort_scores", dec_sort_scores_ns, correct);
        append_stage(out, bc, plain, warmup, iters, "decrypt_sort_indices", dec_sort_indices_ns, correct);
        append_stage(out, bc, plain, warmup, iters, "decrypt_inverse_apply", dec_inverse_apply_ns, correct);
    }
    append_stage(out, bc, plain, warmup, iters, "encrypt_keystream_logistic", enc_keystream_ns, correct);
    append_stage(out, bc, plain, warmup, iters, "encrypt_diffuse", enc_diffuse_ns, correct);
    append_stage(out, bc, plain, warmup, iters, "decrypt_keystream_logistic", dec_keystream_ns, correct);
    append_stage(out, bc, plain, warmup, iters, "decrypt_undiffuse", dec_undiffuse_ns, correct);
    return out;
}

inline std::vector<StageBenchResult> run_microbench_cases(const Image& plain,
                                                          int warmup,
                                                          int iters,
                                                          uint64_t seed = 0xBADC0FFEE0DDF00DULL) {
    std::vector<StageBenchResult> out;

    std::vector<uint8_t> in(plain.bytes());
    std::vector<uint8_t> ks(plain.bytes());
    std::vector<uint8_t> tmp(plain.bytes());
    std::vector<uint8_t> outbuf(plain.bytes());
    fill_counter_mix_bytes(in, seed ^ 0x1111111111111111ULL);
    fill_counter_mix_bytes(ks, seed ^ 0x2222222222222222ULL);

    auto append_micro = [&](const std::string& name,
                            const std::string& stage,
                            uint64_t ns,
                            bool correct) {
        BenchCase bc;
        bc.name = name;
        append_stage(out, bc, plain, warmup, iters, stage, ns, correct);
    };

    auto bench_diff = [&](const std::string& name, DiffusionKernel kernel) {
        for (int i = 0; i < warmup; ++i) {
            diffuse(in.data(), ks.data(), tmp.data(), tmp.size(), kernel, 0);
            undiffuse(tmp.data(), ks.data(), outbuf.data(), outbuf.size(), kernel, 0);
        }

        uint64_t diff_ns = 0;
        uint64_t undiff_ns = 0;
        for (int i = 0; i < iters; ++i) {
            auto t0 = std::chrono::steady_clock::now();
            diffuse(in.data(), ks.data(), tmp.data(), tmp.size(), kernel, 0);
            auto t1 = std::chrono::steady_clock::now();
            undiffuse(tmp.data(), ks.data(), outbuf.data(), outbuf.size(), kernel, 0);
            auto t2 = std::chrono::steady_clock::now();
            diff_ns += elapsed_ns(t0, t1);
            undiff_ns += elapsed_ns(t1, t2);
        }
        const bool correct = (in == outbuf);
        append_micro(name, "diffuse_only", diff_ns, correct);
        append_micro(name, "undiffuse_only", undiff_ns, correct);
    };

    bench_diff("micro_diffusion+scalar_chain", DiffusionKernel::ScalarChain);
    bench_diff("micro_diffusion+scan_exact", DiffusionKernel::ScanExact);

    uint64_t logistic_ns = 0;
    uint64_t counter_ns = 0;
    std::vector<uint8_t> logistic(plain.bytes());
    std::vector<uint8_t> counter(plain.bytes());

    for (int i = 0; i < warmup; ++i) {
        fill_logistic_bytes(logistic, seed);
        fill_counter_mix_bytes(counter, seed);
    }

    for (int i = 0; i < iters; ++i) {
        auto t0 = std::chrono::steady_clock::now();
        fill_logistic_bytes(logistic, seed);
        auto t1 = std::chrono::steady_clock::now();
        fill_counter_mix_bytes(counter, seed);
        auto t2 = std::chrono::steady_clock::now();
        logistic_ns += elapsed_ns(t0, t1);
        counter_ns += elapsed_ns(t1, t2);
    }

    append_micro("micro_keygen+logistic", "keystream_only", logistic_ns, !logistic.empty());
    append_micro("micro_keygen+counter_mix", "keystream_only", counter_ns, !counter.empty());
    return out;
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

inline std::string stage_csv_header() {
    return "case,simd_backend,shape,bytes,warmup,iters,stage,ms,MBps,correct\n";
}

inline std::string to_csv(const StageBenchResult& r) {
    return r.name + "," +
           r.simd_backend + "," +
           r.shape + "," +
           std::to_string(r.bytes) + "," +
           std::to_string(r.warmup) + "," +
           std::to_string(r.iters) + "," +
           r.stage + "," +
           std::to_string(r.ms) + "," +
           std::to_string(r.mb_s) + "," +
           (r.correct ? "1" : "0") + "\n";
}

}  // namespace chaosref
