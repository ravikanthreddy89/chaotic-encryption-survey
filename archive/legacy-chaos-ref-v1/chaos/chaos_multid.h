#pragma once

#include <array>
#include <numeric>
#include <opencv2/opencv.hpp>

#include "chaos_common.h"

namespace Ciphers {

struct YeHuang2018 {
    static constexpr const char* NAME = "YeHuang-2018";
    static constexpr int LOC = 128;

    struct LogSine {
        double x, mu;
        LogSine(double x0, double mu_, int wu = 500) : x(x0), mu(mu_) {
            for (int i = 0; i < wu; ++i) step();
        }
        inline double next() { return step(); }
        inline uint8_t byte() { return (uint8_t)((int)(step() * 256.0) & 0xFF); }
    private:
        inline double step() {
            x = yeh_sin(M_PI * mu * (x * (1.0 - x) + (4.0 - mu) * x * yeh_sin(M_PI * x) / 4.0));
            if (x < 0) x = -x;
            if (x >= 1.0) x = 1.0 - 1e-10;
            return x;
        }
    };

    static void derive_ye(double& x0, double& x1, double& mu0, double& mu1) {
        uint8_t digest[32];
        {
            ChaosProfile::Scope s(NAME, ChaosProfile::Stage::KeyDerive);
            SHA256(Bench::FIXED_KEY.data(), 32, digest);
        }
        uint64_t a, b;
        std::memcpy(&a, digest, 8);
        std::memcpy(&b, digest + 8, 8);
        x0 = (double)(a & 0x000FFFFFFFFFFFFFULL) / (double)0x000FFFFFFFFFFFFFULL;
        x1 = (double)(b & 0x000FFFFFFFFFFFFFULL) / (double)0x000FFFFFFFFFFFFFULL;
        if (x0 < 0.001) x0 = 0.001;
        if (x1 < 0.001) x1 = 0.001;
        uint32_t m0, m1;
        std::memcpy(&m0, digest + 16, 4);
        std::memcpy(&m1, digest + 20, 4);
        mu0 = 3.5 + 0.49 * ((double)(m0 & 0xFFFFFF) / (double)0xFFFFFF);
        mu1 = 3.5 + 0.49 * ((double)(m1 & 0xFFFFFF) / (double)0xFFFFFF);
    }

    static cv::Mat encrypt_image(const cv::Mat& src) {
        double x0, x1, mu0, mu1;
        derive_ye(x0, x1, mu0, mu1);

        const int H = src.rows, W = src.cols, C = src.channels();
        const int N = H * W;

        LogSine ls0(x0, mu0);
        std::vector<double> seq(N);
        {
            ChaosProfile::Scope s(NAME, ChaosProfile::Stage::SeqGen);
            for (auto& v : seq) v = ls0.next();
        }

        std::vector<int> idx(N);
        std::iota(idx.begin(), idx.end(), 0);
        {
            ChaosProfile::Scope s(NAME, ChaosProfile::Stage::SortPerm);
            std::sort(idx.begin(), idx.end(), [&](int a, int b) { return seq[a] < seq[b]; });
        }

        cv::Mat permuted(H, W, src.type());
        {
            ChaosProfile::Scope s(NAME, ChaosProfile::Stage::PermuteApply);
            for (int i = 0; i < N; ++i)
                for (int c = 0; c < C; ++c)
                    permuted.data[idx[i] * C + c] = src.data[i * C + c];
        }

        LogSine ls1(x1, mu1);
        cv::Mat out(H, W, src.type());
        uint8_t prev_r = 0x5A, prev_g = 0xA5, prev_b = 0xF0;
        {
            ChaosProfile::Scope s(NAME, ChaosProfile::Stage::Diffuse);
#if CHAOS_DIFFUSE_KERNEL != 0
            if (C == 3) {
                std::vector<uint8_t> ks_local(N * C);
                for (auto& b : ks_local) b = ls1.byte();
                std::vector<uint8_t> scratch;
                uint8_t prev[3] = {prev_r, prev_g, prev_b};
                diffuse_cbc3(permuted.data, ks_local.data(), out.data, static_cast<size_t>(N * C), prev, scratch);
                prev_r = prev[0]; prev_g = prev[1]; prev_b = prev[2];
            } else {
                for (int i = 0; i < N; ++i) {
                    uint8_t kr = ls1.byte(), kg = ls1.byte(), kb = ls1.byte();
                    uint8_t er = permuted.data[i * C + 0] ^ kr ^ prev_r;
                    uint8_t eg = permuted.data[i * C + 1] ^ kg ^ prev_g;
                    uint8_t eb = permuted.data[i * C + 2] ^ kb ^ prev_b;
                    out.data[i * C + 0] = er; out.data[i * C + 1] = eg; out.data[i * C + 2] = eb;
                    prev_r = er; prev_g = eg; prev_b = eb;
                }
            }
#else
            for (int i = 0; i < N; ++i) {
                uint8_t kr = ls1.byte(), kg = ls1.byte(), kb = ls1.byte();
                uint8_t er = permuted.data[i * C + 0] ^ kr ^ prev_r;
                uint8_t eg = permuted.data[i * C + 1] ^ kg ^ prev_g;
                uint8_t eb = permuted.data[i * C + 2] ^ kb ^ prev_b;
                out.data[i * C + 0] = er; out.data[i * C + 1] = eg; out.data[i * C + 2] = eb;
                prev_r = er; prev_g = eg; prev_b = eb;
            }
#endif
        }
        return out;
    }

    static cv::Mat decrypt_image(const cv::Mat& src, int rows, int cols, int type) {
        double x0, x1, mu0, mu1;
        derive_ye(x0, x1, mu0, mu1);

        const int H = rows, W = cols, C = src.channels(), N = H * W;

        LogSine ls1(x1, mu1);
        std::vector<uint8_t> ks(N * C);
        {
            ChaosProfile::Scope s(NAME, ChaosProfile::Stage::SeqGen);
            for (auto& b : ks) b = ls1.byte();
        }

        cv::Mat undiff(H, W, type);
        uint8_t pr = 0x5A, pg = 0xA5, pb = 0xF0;
        {
            ChaosProfile::Scope s(NAME, ChaosProfile::Stage::Reverse);
#if CHAOS_DIFFUSE_KERNEL != 0
            if (C == 3) {
                std::vector<uint8_t> scratch;
                uint8_t prev[3] = {pr, pg, pb};
                undiffuse_cbc3(src.data, ks.data(), undiff.data, static_cast<size_t>(N * C), prev, scratch);
                pr = prev[0]; pg = prev[1]; pb = prev[2];
            } else {
                for (int i = 0; i < N; ++i) {
                    uint8_t er = src.data[i * C + 0], eg = src.data[i * C + 1], eb = src.data[i * C + 2];
                    undiff.data[i * C + 0] = er ^ ks[i * C + 0] ^ pr;
                    undiff.data[i * C + 1] = eg ^ ks[i * C + 1] ^ pg;
                    undiff.data[i * C + 2] = eb ^ ks[i * C + 2] ^ pb;
                    pr = er; pg = eg; pb = eb;
                }
            }
#else
            for (int i = 0; i < N; ++i) {
                uint8_t er = src.data[i * C + 0], eg = src.data[i * C + 1], eb = src.data[i * C + 2];
                undiff.data[i * C + 0] = er ^ ks[i * C + 0] ^ pr;
                undiff.data[i * C + 1] = eg ^ ks[i * C + 1] ^ pg;
                undiff.data[i * C + 2] = eb ^ ks[i * C + 2] ^ pb;
                pr = er; pg = eg; pb = eb;
            }
#endif
        }

        LogSine ls0(x0, mu0);
        std::vector<double> seq(N);
        for (auto& v : seq) v = ls0.next();
        std::vector<int> idx(N);
        std::iota(idx.begin(), idx.end(), 0);
        {
            ChaosProfile::Scope s(NAME, ChaosProfile::Stage::SortPerm);
            std::sort(idx.begin(), idx.end(), [&](int a, int b) { return seq[a] < seq[b]; });
        }

        cv::Mat out(H, W, type);
        {
            ChaosProfile::Scope s(NAME, ChaosProfile::Stage::PermuteApply);
            for (int i = 0; i < N; ++i)
                for (int c = 0; c < C; ++c)
                    out.data[i * C + c] = undiff.data[idx[i] * C + c];
        }
        return out;
    }
};

struct YeHuang2018Optimized {
    static constexpr const char* NAME = "YeHuang-2018-Optimized";
    static constexpr int LOC = 128;

    struct SortPair { double val; int idx; };
    struct WorkBuffers {
        std::vector<double> seq;
        std::vector<SortPair> pairs;
        std::vector<uint8_t> keystream;
        std::vector<uint8_t> scratch;
    };
    static WorkBuffers& buffers() { thread_local WorkBuffers wb; return wb; }
    static inline void ensure_sizes(WorkBuffers& wb, int N, int total_bytes) {
        wb.seq.resize(N); wb.pairs.resize(N); wb.keystream.resize(total_bytes);
    }

    struct LogSine {
        double x, mu;
        LogSine(double x0, double mu_, int wu = 500) : x(x0), mu(mu_) {
            for (int i = 0; i < wu; ++i) step();
        }
        inline void generate_keystream(uint8_t* dest, size_t count) {
            for (size_t i = 0; i < count; ++i) {
                step();
                dest[i] = static_cast<uint8_t>(static_cast<int>(x * 256.0) & 0xFF);
            }
        }
        inline void generate_doubles(double* dest, size_t count) {
            for (size_t i = 0; i < count; ++i) dest[i] = step();
        }
    private:
        inline double step() {
            x = yeh_sin(M_PI * mu * (x * (1.0 - x) + (4.0 - mu) * x * yeh_sin(M_PI * x) / 4.0));
            if (x < 0) x = -x;
            if (x >= 1.0) x = 1.0 - 1e-10;
            return x;
        }
    };

    static void derive_ye(double& x0, double& x1, double& mu0, double& mu1) {
        uint8_t digest[32];
        {
            ChaosProfile::Scope s(NAME, ChaosProfile::Stage::KeyDerive);
            SHA256(Bench::FIXED_KEY.data(), 32, digest);
        }
        uint64_t a, b;
        std::memcpy(&a, digest, 8);
        std::memcpy(&b, digest + 8, 8);
        x0 = (double)(a & 0x000FFFFFFFFFFFFFULL) / (double)0x000FFFFFFFFFFFFFULL;
        x1 = (double)(b & 0x000FFFFFFFFFFFFFULL) / (double)0x000FFFFFFFFFFFFFULL;
        if (x0 < 0.001) x0 = 0.001;
        if (x1 < 0.001) x1 = 0.001;
        uint32_t m0, m1;
        std::memcpy(&m0, digest + 16, 4);
        std::memcpy(&m1, digest + 20, 4);
        mu0 = 3.5 + 0.49 * ((double)(m0 & 0xFFFFFF) / (double)0xFFFFFF);
        mu1 = 3.5 + 0.49 * ((double)(m1 & 0xFFFFFF) / (double)0xFFFFFF);
    }

    static cv::Mat encrypt_image(const cv::Mat& src) {
        double x0, x1, mu0, mu1;
        derive_ye(x0, x1, mu0, mu1);

        const int H = src.rows, W = src.cols, C = src.channels();
        const int N = H * W;
        const int total_bytes = N * C;
        WorkBuffers& wb = buffers();
        ensure_sizes(wb, N, total_bytes);

        LogSine ls0(x0, mu0);
        {
            ChaosProfile::Scope s(NAME, ChaosProfile::Stage::SeqGen);
            ls0.generate_doubles(wb.seq.data(), N);
        }
        for (int i = 0; i < N; ++i) wb.pairs[i] = {wb.seq[i], i};
        {
            ChaosProfile::Scope s(NAME, ChaosProfile::Stage::SortPerm);
            std::sort(wb.pairs.begin(), wb.pairs.end(), [](const SortPair& a, const SortPair& b) { return a.val < b.val; });
        }

        cv::Mat permuted(H, W, src.type());
        uint8_t* src_ptr = src.data;
        uint8_t* perm_ptr = permuted.data;
        {
            ChaosProfile::Scope s(NAME, ChaosProfile::Stage::PermuteApply);
            if (C == 3) {
                for (int i = 0; i < N; ++i) {
                    int src_offset = i * 3;
                    int dest_offset = wb.pairs[i].idx * 3;
                    perm_ptr[dest_offset] = src_ptr[src_offset];
                    perm_ptr[dest_offset + 1] = src_ptr[src_offset + 1];
                    perm_ptr[dest_offset + 2] = src_ptr[src_offset + 2];
                }
            } else {
                for (int i = 0; i < N; ++i)
                    std::memcpy(perm_ptr + (wb.pairs[i].idx * C), src_ptr + (i * C), C);
            }
        }

        LogSine ls1(x1, mu1);
        {
            ChaosProfile::Scope s(NAME, ChaosProfile::Stage::SeqGen);
            ls1.generate_keystream(wb.keystream.data(), total_bytes);
        }

        cv::Mat out(H, W, src.type());
        uint8_t* out_ptr = out.data;
        uint8_t prev[3] = {0x5A, 0xA5, 0xF0};
        {
            ChaosProfile::Scope s(NAME, ChaosProfile::Stage::Diffuse);
            if (C == 3) {
                diffuse_cbc3(perm_ptr, wb.keystream.data(), out_ptr, static_cast<size_t>(total_bytes), prev, wb.scratch);
            } else {
                for (int i = 0; i < total_bytes; i += C) {
                    for (int c = 0; c < C; ++c) {
                        uint8_t er = perm_ptr[i + c] ^ wb.keystream[i + c] ^ prev[c];
                        out_ptr[i + c] = er;
                        prev[c] = er;
                    }
                }
            }
        }
        return out;
    }

    static cv::Mat decrypt_image(const cv::Mat& src, int rows, int cols, int type) {
        double x0, x1, mu0, mu1;
        derive_ye(x0, x1, mu0, mu1);

        const int H = rows, W = cols, C = src.channels(), N = H * W;
        const int total_bytes = N * C;
        WorkBuffers& wb = buffers();
        ensure_sizes(wb, N, total_bytes);

        LogSine ls1(x1, mu1);
        {
            ChaosProfile::Scope s(NAME, ChaosProfile::Stage::SeqGen);
            ls1.generate_keystream(wb.keystream.data(), total_bytes);
        }

        cv::Mat undiff(H, W, type);
        const uint8_t* src_ptr = src.data;
        uint8_t* undiff_ptr = undiff.data;
        uint8_t prev[3] = {0x5A, 0xA5, 0xF0};
        {
            ChaosProfile::Scope s(NAME, ChaosProfile::Stage::Reverse);
            if (C == 3) {
                undiffuse_cbc3(src_ptr, wb.keystream.data(), undiff_ptr, static_cast<size_t>(total_bytes), prev, wb.scratch);
            } else {
                for (int i = 0; i < total_bytes; i += C) {
                    for (int c = 0; c < C; ++c) {
                        uint8_t er = src_ptr[i + c];
                        undiff_ptr[i + c] = er ^ wb.keystream[i + c] ^ prev[c];
                        prev[c] = er;
                    }
                }
            }
        }

        LogSine ls0(x0, mu0);
        {
            ChaosProfile::Scope s(NAME, ChaosProfile::Stage::SeqGen);
            ls0.generate_doubles(wb.seq.data(), N);
        }
        for (int i = 0; i < N; ++i) wb.pairs[i] = {wb.seq[i], i};
        {
            ChaosProfile::Scope s(NAME, ChaosProfile::Stage::SortPerm);
            std::sort(wb.pairs.begin(), wb.pairs.end(), [](const SortPair& a, const SortPair& b) { return a.val < b.val; });
        }

        cv::Mat out(H, W, type);
        uint8_t* out_ptr = out.data;
        {
            ChaosProfile::Scope s(NAME, ChaosProfile::Stage::PermuteApply);
            if (C == 3) {
                for (int i = 0; i < N; ++i) {
                    int src_offset = wb.pairs[i].idx * 3;
                    int dest_offset = i * 3;
                    out_ptr[dest_offset] = undiff_ptr[src_offset];
                    out_ptr[dest_offset + 1] = undiff_ptr[src_offset + 1];
                    out_ptr[dest_offset + 2] = undiff_ptr[src_offset + 2];
                }
            } else {
                for (int i = 0; i < N; ++i)
                    std::memcpy(out_ptr + (i * C), undiff_ptr + (wb.pairs[i].idx * C), C);
            }
        }
        return out;
    }
};

struct LSCM2020 {
    static constexpr const char* NAME = "LSCM-2020";
    static constexpr int LOC = 112;

    struct WorkBuffers {
        std::vector<double> sp;
        std::vector<int> perm;
        std::vector<uint8_t> ks;
        std::vector<int> inv;
        std::vector<uint8_t> scratch;
    };
    static WorkBuffers& buffers() { thread_local WorkBuffers wb; return wb; }
    static inline void ensure_sizes(WorkBuffers& wb, int N, int total_bytes) {
        wb.sp.resize(N); wb.perm.resize(N); wb.ks.resize(total_bytes); wb.inv.resize(N);
    }

    struct LSCM {
        double x, mu;
        LSCM(double x0, double mu_, int wu = 500) : x(x0), mu(mu_) { for (int i = 0; i < wu; ++i) step(); }
        inline double next() { return step(); }
        inline uint8_t byte() { return (uint8_t)((int)(step() * 256.0) & 0xFF); }
    private:
        inline double step() {
            x = (4.0 - mu) * yeh_sin(M_PI * x) / 4.0 + mu * x * (1.0 - x);
            if (x <= 0.0) x = 1e-10;
            if (x >= 1.0) x = 1.0 - 1e-10;
            return x;
        }
    };

    static cv::Mat encrypt_image(const cv::Mat& src) {
        double x0, y0, r;
        {
            ChaosProfile::Scope s(NAME, ChaosProfile::Stage::KeyDerive);
            derive_ic(Bench::FIXED_KEY.data(), x0, y0, r);
        }
        double mu = 3.5 + (r - 3.9) * 4.0;

        const int H = src.rows, W = src.cols, C = src.channels(), N = H * W;
        const int total_bytes = N * C;
        WorkBuffers& wb = buffers();
        ensure_sizes(wb, N, total_bytes);

        LSCM lscm_p(x0, mu);
        LSCM lscm_d(y0, mu);

        {
            ChaosProfile::Scope s(NAME, ChaosProfile::Stage::SeqGen);
            for (auto& v : wb.sp) v = lscm_p.next();
        }
        std::iota(wb.perm.begin(), wb.perm.end(), 0);
        {
            ChaosProfile::Scope s(NAME, ChaosProfile::Stage::SortPerm);
            std::sort(wb.perm.begin(), wb.perm.end(), [&](int a, int b) { return wb.sp[a] < wb.sp[b]; });
        }

        {
            ChaosProfile::Scope s(NAME, ChaosProfile::Stage::SeqGen);
            for (auto& b : wb.ks) b = lscm_d.byte();
        }

        cv::Mat out(H, W, src.type());
        uint8_t chain = 0;
        {
            ChaosProfile::Scope s(NAME, ChaosProfile::Stage::Diffuse);
#if CHAOS_DIFFUSE_KERNEL != 0
            std::vector<uint8_t> permuted(total_bytes);
            for (int i = 0; i < N; ++i) {
                int src_i = wb.perm[i];
                for (int c = 0; c < C; ++c) {
                    permuted[i * C + c] = src.data[src_i * C + c];
                }
            }
            diffuse_chain(permuted.data(), wb.ks.data(), out.data, static_cast<size_t>(total_bytes), chain, wb.scratch);
#else
            for (int i = 0; i < N; ++i) {
                int src_i = wb.perm[i];
                for (int c = 0; c < C; ++c) {
                    uint8_t p = src.data[src_i * C + c];
                    uint8_t k = wb.ks[i * C + c];
                    uint8_t e = p ^ k ^ chain;
                    out.data[i * C + c] = e;
                    chain = e;
                }
            }
#endif
        }
        return out;
    }

    static cv::Mat decrypt_image(const cv::Mat& src, int rows, int cols, int type) {
        double x0, y0, r;
        {
            ChaosProfile::Scope s(NAME, ChaosProfile::Stage::KeyDerive);
            derive_ic(Bench::FIXED_KEY.data(), x0, y0, r);
        }
        double mu = 3.5 + (r - 3.9) * 4.0;

        const int H = rows, W = cols, C = src.channels(), N = H * W;
        const int total_bytes = N * C;
        WorkBuffers& wb = buffers();
        ensure_sizes(wb, N, total_bytes);

        LSCM lscm_p(x0, mu);
        LSCM lscm_d(y0, mu);

        {
            ChaosProfile::Scope s(NAME, ChaosProfile::Stage::SeqGen);
            for (auto& v : wb.sp) v = lscm_p.next();
        }
        std::iota(wb.perm.begin(), wb.perm.end(), 0);
        {
            ChaosProfile::Scope s(NAME, ChaosProfile::Stage::SortPerm);
            std::sort(wb.perm.begin(), wb.perm.end(), [&](int a, int b) { return wb.sp[a] < wb.sp[b]; });
        }

        {
            ChaosProfile::Scope s(NAME, ChaosProfile::Stage::SeqGen);
            for (auto& b : wb.ks) b = lscm_d.byte();
        }

        for (int i = 0; i < N; ++i) wb.inv[wb.perm[i]] = i;

        cv::Mat undiff(H, W, type);
        uint8_t chain = 0;
        {
            ChaosProfile::Scope s(NAME, ChaosProfile::Stage::Reverse);
#if CHAOS_DIFFUSE_KERNEL != 0
            undiffuse_chain(src.data, wb.ks.data(), undiff.data, static_cast<size_t>(total_bytes), chain, wb.scratch);
#else
            for (int i = 0; i < N; ++i) {
                for (int c = 0; c < C; ++c) {
                    uint8_t e = src.data[i * C + c];
                    uint8_t k = wb.ks[i * C + c];
                    undiff.data[i * C + c] = e ^ k ^ chain;
                    chain = e;
                }
            }
#endif
        }

        cv::Mat out(H, W, type);
        {
            ChaosProfile::Scope s(NAME, ChaosProfile::Stage::PermuteApply);
            for (int i = 0; i < N; ++i)
                for (int c = 0; c < C; ++c)
                    out.data[wb.inv[i] * C + c] = undiff.data[i * C + c];
        }
        return out;
    }
};

} // namespace Ciphers
