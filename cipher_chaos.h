#pragma once
/**
 * cipher_chaos.h
 * ─────────────────────────────────────────────────────────────────────────────
 * Three representative chaos-based image ciphers from the literature:
 *
 * [1] FRIDRICH-1998  — Fridrich, J. "Symmetric ciphers based on two-dimensional
 *     chaotic maps." IJBC 1998.
 *     Arnold Cat Map permutation + Logistic Map diffusion.
 *     The canonical baseline; still widely cited.
 *     LOC: ~80
 *
 * [2] YE-HUANG-2018  — Ye, G., Huang, X. "A secure image encryption algorithm
 *     based on chaotic maps and SHA-3." CAEE 2018.
 *     SHA-3 key stretching + 2D Logistic-Sine coupling + row-column XOR
 *     diffusion.  Addresses Fridrich's chosen-plaintext vulnerability.
 *     LOC: ~130
 *
 * [3] LSCM-2020  — Liu, L., Miao, S. "A new image encryption algorithm based
 *     on logistic-sine coupling map." Scientific Reports 2020.
 *     Logistic-Sine coupled map; single-pass permutation+diffusion; high
 *     NPCR/UACI, good resistance to differential attacks.
 *     LOC: ~110
 *
 * All three use only the fixed key from bench_common.h for reproducibility.
 * Chaotic IC are derived deterministically from the key bytes so the same key
 * always produces the same ciphertext.
 */

#pragma once
#include <opencv2/opencv.hpp>
#include <openssl/sha.h>
#include <vector>
#include <array>
#include <cmath>
#include <cstring>
#include <numeric>
#include <algorithm>
#include <stdexcept>
#include "bench_common.h"

namespace Ciphers {

// ─────────────────────────────────────────────────────────────────────────────
//  Shared: derive chaotic ICs from a 32-byte key
// ─────────────────────────────────────────────────────────────────────────────
static inline void derive_ic(const uint8_t key[32],
                               double& x0, double& y0, double& r) {
    // Treat first 8 bytes as fixed-point mantissa
    uint64_t xi = 0, yi = 0;
    std::memcpy(&xi, key,    8);
    std::memcpy(&yi, key+8,  8);
    x0 = (double)(xi & 0x000FFFFFFFFFFFFFULL) / (double)0x000FFFFFFFFFFFFFULL;
    y0 = (double)(yi & 0x000FFFFFFFFFFFFFULL) / (double)0x000FFFFFFFFFFFFFULL;
    if (x0 < 0.001) x0 = 0.001;
    if (y0 < 0.001) y0 = 0.001;
    // r from bytes 16-19, scaled to [3.9, 4.0)
    uint32_t ri = 0;
    std::memcpy(&ri, key+16, 4);
    r = 3.9 + 0.1 * ((double)(ri & 0xFFFFFF) / (double)0xFFFFFF);
}

// ─────────────────────────────────────────────────────────────────────────────
//  Logistic Map iterator  (shared by all three ciphers)
// ─────────────────────────────────────────────────────────────────────────────
struct Logistic {
    double x, r;
    Logistic(double x0, double r_, int warmup=500) : x(x0), r(r_) {
        for(int i=0;i<warmup;++i) step();
    }
    inline double next() { return step(); }
    inline uint8_t byte() { return (uint8_t)((int)(step()*256.0)&0xFF); }
private:
    inline double step() { x = r*x*(1.0-x); return x; }
};

// ─────────────────────────────────────────────────────────────────────────────
//  [1]  FRIDRICH-1998
//  Permutation: Arnold Cat Map  (p rounds)
//  Diffusion:   row-by-row XOR with Logistic keystream
// ─────────────────────────────────────────────────────────────────────────────
struct Fridrich1998 {
    static constexpr const char* NAME = "Fridrich-1998";
    static constexpr int LOC = 82;

    static cv::Mat encrypt_image(const cv::Mat& src) {
        double x0, y0, r;
        derive_ic(Bench::FIXED_KEY.data(), x0, y0, r);

        // Step 1: Arnold Cat Map permutation (2 rounds)
        const int H = src.rows, W = src.cols;
        cv::Mat perm = src.clone();
        cv::Mat tmp(H, W, src.type());
        for (int round = 0; round < 2; ++round) {
            for (int y = 0; y < H; ++y)
                for (int x = 0; x < W; ++x) {
                    int nx = (x + y) % W;
                    int ny = (x + 2*y) % H;
                    for (int c = 0; c < src.channels(); ++c)
                        tmp.at<cv::Vec3b>(ny,nx)[c] = perm.at<cv::Vec3b>(y,x)[c];
                }
            std::swap(perm, tmp);
        }

        // Step 2: Logistic XOR diffusion
        Logistic lm(x0, r);
        cv::Mat out(H, W, src.type());
        const int C = src.channels();
        const int N = H * W;
        uint8_t prev = 0;
        for (int i = 0; i < N; ++i) {
            for (int c = 0; c < C; ++c) {
                uint8_t p = perm.data[i*C+c];
                uint8_t k = lm.byte();
                uint8_t e = p ^ k ^ prev;
                out.data[i*C+c] = e;
                prev = e;
            }
        }
        return out;
    }

    static cv::Mat decrypt_image(const cv::Mat& src, int rows, int cols, int type) {
        double x0, y0, r;
        derive_ic(Bench::FIXED_KEY.data(), x0, y0, r);

        // Rebuild keystream (deterministic)
        Logistic lm(x0, r);
        const int H = rows, W = cols, C = src.channels();
        const int N = H * W;
        std::vector<uint8_t> ks(N * C);
        for (auto& b : ks) b = lm.byte();

        // Reverse diffusion
        cv::Mat undiff(H, W, type);
        uint8_t prev = 0;
        for (int i = 0; i < N; ++i) {
            for (int c = 0; c < C; ++c) {
                uint8_t e = src.data[i*C+c];
                uint8_t k = ks[i*C+c];
                undiff.data[i*C+c] = e ^ k ^ prev;
                prev = e;
            }
        }

        // Reverse Arnold Cat Map (2 rounds)
        cv::Mat out = undiff.clone();
        cv::Mat tmp(H, W, type);
        for (int round = 0; round < 2; ++round) {
            for (int y = 0; y < H; ++y)
                for (int x = 0; x < W; ++x) {
                    int ox = ((2*x - y) % W + W) % W;
                    int oy = ((-x + y) % H + H) % H;
                    for (int c = 0; c < C; ++c)
                        tmp.at<cv::Vec3b>(oy,ox)[c] = out.at<cv::Vec3b>(y,x)[c];
                }
            std::swap(out, tmp);
        }
        return out;
    }
};

// ─────────────────────────────────────────────────────────────────────────────
//  [2]  YE-HUANG-2018
//  Key stretching: SHA-256 of key → chaotic ICs
//  Permutation:    index sort of Logistic sequence (confusion)
//  Diffusion:      2D Logistic-Sine coupled map XOR with CBC chaining
// ─────────────────────────────────────────────────────────────────────────────
struct YeHuang2018 {
    static constexpr const char* NAME = "YeHuang-2018";
    static constexpr int LOC = 128;

    // Logistic-Sine coupling:
    //   x_{n+1} = sin(pi * mu * (x_n*(1-x_n) + (4-mu)*x_n*sin(pi*x_n)/4 ))
    struct LogSine {
        double x, mu;
        LogSine(double x0, double mu_, int wu=500) : x(x0), mu(mu_) {
            for(int i=0;i<wu;++i) step();
        }
        inline double next() { return step(); }
        inline uint8_t byte() { return (uint8_t)((int)(step()*256.0)&0xFF); }
    private:
        inline double step() {
            x = std::sin(M_PI * mu * (x*(1.0-x) + (4.0-mu)*x*std::sin(M_PI*x)/4.0));
            if (x < 0) x = -x;
            if (x >= 1.0) x = 1.0 - 1e-10;
            return x;
        }
    };

    static void derive_ye(double& x0, double& x1, double& mu0, double& mu1) {
        // SHA-256 of the fixed key for key stretching
        uint8_t digest[32];
        SHA256(Bench::FIXED_KEY.data(), 32, digest);
        uint64_t a, b;
        std::memcpy(&a, digest,   8);
        std::memcpy(&b, digest+8, 8);
        x0  = (double)(a & 0x000FFFFFFFFFFFFFULL) / (double)0x000FFFFFFFFFFFFFULL;
        x1  = (double)(b & 0x000FFFFFFFFFFFFFULL) / (double)0x000FFFFFFFFFFFFFULL;
        if (x0 < 0.001) x0 = 0.001;
        if (x1 < 0.001) x1 = 0.001;
        uint32_t m0, m1;
        std::memcpy(&m0, digest+16, 4);
        std::memcpy(&m1, digest+20, 4);
        mu0 = 3.5 + 0.49 * ((double)(m0 & 0xFFFFFF) / (double)0xFFFFFF);
        mu1 = 3.5 + 0.49 * ((double)(m1 & 0xFFFFFF) / (double)0xFFFFFF);
    }

    static cv::Mat encrypt_image(const cv::Mat& src) {
        double x0, x1, mu0, mu1;
        derive_ye(x0, x1, mu0, mu1);

        const int H = src.rows, W = src.cols, C = src.channels();
        const int N = H * W;

        // ── Permutation: generate N doubles, argsort to get shuffle index ──
        LogSine ls0(x0, mu0);
        std::vector<double> seq(N);
        for (auto& v : seq) v = ls0.next();
        std::vector<int> idx(N);
        std::iota(idx.begin(), idx.end(), 0);
        std::sort(idx.begin(), idx.end(), [&](int a, int b){ return seq[a] < seq[b]; });

        cv::Mat permuted(H, W, src.type());
        for (int i = 0; i < N; ++i)
            for (int c = 0; c < C; ++c)
                permuted.data[idx[i]*C+c] = src.data[i*C+c];

        // ── Diffusion: LogSine XOR with CBC chaining ──────────────────────
        LogSine ls1(x1, mu1);
        cv::Mat out(H, W, src.type());
        uint8_t prev_r=0x5A, prev_g=0xA5, prev_b=0xF0;
        for (int i = 0; i < N; ++i) {
            uint8_t kr = ls1.byte(), kg = ls1.byte(), kb = ls1.byte();
            uint8_t er = permuted.data[i*C+0] ^ kr ^ prev_r;
            uint8_t eg = permuted.data[i*C+1] ^ kg ^ prev_g;
            uint8_t eb = permuted.data[i*C+2] ^ kb ^ prev_b;
            out.data[i*C+0]=er; out.data[i*C+1]=eg; out.data[i*C+2]=eb;
            prev_r=er; prev_g=eg; prev_b=eb;
        }
        return out;
    }

    static cv::Mat decrypt_image(const cv::Mat& src, int rows, int cols, int type) {
        double x0, x1, mu0, mu1;
        derive_ye(x0, x1, mu0, mu1);

        const int H=rows, W=cols, C=src.channels(), N=H*W;

        // Rebuild diffusion keystream
        LogSine ls1(x1, mu1);
        std::vector<uint8_t> ks(N*C);
        for (auto& b : ks) b = ls1.byte();

        // Reverse diffusion
        cv::Mat undiff(H, W, type);
        uint8_t pr=0x5A, pg=0xA5, pb=0xF0;
        for (int i=0; i<N; ++i) {
            uint8_t er=src.data[i*C+0], eg=src.data[i*C+1], eb=src.data[i*C+2];
            undiff.data[i*C+0] = er ^ ks[i*C+0] ^ pr;
            undiff.data[i*C+1] = eg ^ ks[i*C+1] ^ pg;
            undiff.data[i*C+2] = eb ^ ks[i*C+2] ^ pb;
            pr=er; pg=eg; pb=eb;
        }

        // Rebuild permutation index
        LogSine ls0(x0, mu0);
        std::vector<double> seq(N);
        for (auto& v : seq) v = ls0.next();
        std::vector<int> idx(N);
        std::iota(idx.begin(), idx.end(), 0);
        std::sort(idx.begin(), idx.end(), [&](int a, int b){ return seq[a] < seq[b]; });

        // Inverse permutation
        cv::Mat out(H, W, type);
        for (int i=0; i<N; ++i)
            for (int c=0; c<C; ++c)
                out.data[i*C+c] = undiff.data[idx[i]*C+c];
        return out;
    }
};

// ─────────────────────────────────────────────────────────────────────────────
//  [3]  LSCM-2020  (Logistic-Sine Coupled Map)
//  Single-pass: for each pixel, simultaneously permute AND diffuse.
//  Uses two coupled LSCM sequences: one for position, one for XOR value.
//  Reference: Liu & Miao, Scientific Reports 10, 2020.
// ─────────────────────────────────────────────────────────────────────────────
struct LSCM2020 {
    static constexpr const char* NAME = "LSCM-2020";
    static constexpr int LOC = 112;

    // Logistic-Sine Coupled Map:
    //   x_{n+1} = (4 - mu)*sin(pi*x_n)/4 + mu*x_n*(1-x_n)
    struct LSCM {
        double x, mu;
        LSCM(double x0, double mu_, int wu=500) : x(x0), mu(mu_) {
            for(int i=0;i<wu;++i) step();
        }
        inline double next() { return step(); }
        inline uint8_t byte() { return (uint8_t)((int)(step()*256.0)&0xFF); }
    private:
        inline double step() {
            x = (4.0-mu)*std::sin(M_PI*x)/4.0 + mu*x*(1.0-x);
            if (x<=0.0) x=1e-10;
            if (x>=1.0) x=1.0-1e-10;
            return x;
        }
    };

    static cv::Mat encrypt_image(const cv::Mat& src) {
        double x0, y0, r;
        derive_ic(Bench::FIXED_KEY.data(), x0, y0, r);
        double mu = 3.5 + (r - 3.9) * 4.0;  // map r → mu in [3.5, 3.9)

        const int H=src.rows, W=src.cols, C=src.channels(), N=H*W;

        // Two LSCM sequences: seq_p for permutation, seq_d for diffusion
        LSCM lscm_p(x0, mu);
        LSCM lscm_d(y0, mu);

        // Build permutation via index-sort of lscm_p
        std::vector<double> sp(N);
        for (auto& v : sp) v = lscm_p.next();
        std::vector<int> perm(N);
        std::iota(perm.begin(), perm.end(), 0);
        std::sort(perm.begin(), perm.end(), [&](int a, int b){ return sp[a]<sp[b]; });

        // Single-pass: permute then XOR
        cv::Mat out(H, W, src.type());
        std::vector<uint8_t> ks(N*C);
        for (auto& b : ks) b = lscm_d.byte();

        uint8_t chain = 0;
        for (int i=0; i<N; ++i) {
            int src_i = perm[i];
            for (int c=0; c<C; ++c) {
                uint8_t p = src.data[src_i*C+c];
                uint8_t k = ks[i*C+c];
                uint8_t e = p ^ k ^ chain;
                out.data[i*C+c] = e;
                chain = e;
            }
        }
        return out;
    }

    static cv::Mat decrypt_image(const cv::Mat& src, int rows, int cols, int type) {
        double x0, y0, r;
        derive_ic(Bench::FIXED_KEY.data(), x0, y0, r);
        double mu = 3.5 + (r - 3.9) * 4.0;

        const int H=rows, W=cols, C=src.channels(), N=H*W;

        // Rebuild both sequences
        LSCM lscm_p(x0, mu);
        LSCM lscm_d(y0, mu);

        std::vector<double> sp(N);
        for (auto& v : sp) v = lscm_p.next();
        std::vector<int> perm(N);
        std::iota(perm.begin(), perm.end(), 0);
        std::sort(perm.begin(), perm.end(), [&](int a, int b){ return sp[a]<sp[b]; });

        std::vector<uint8_t> ks(N*C);
        for (auto& b : ks) b = lscm_d.byte();

        // Inverse permutation
        std::vector<int> inv(N);
        for (int i=0; i<N; ++i) inv[perm[i]] = i;

        // Reverse diffusion then inverse permute
        cv::Mat undiff(H, W, type);
        uint8_t chain = 0;
        for (int i=0; i<N; ++i) {
            for (int c=0; c<C; ++c) {
                uint8_t e = src.data[i*C+c];
                uint8_t k = ks[i*C+c];
                undiff.data[i*C+c] = e ^ k ^ chain;
                chain = e;
            }
        }

        cv::Mat out(H, W, type);
        for (int i=0; i<N; ++i)
            for (int c=0; c<C; ++c)
                out.data[inv[i]*C+c] = undiff.data[i*C+c];
        return out;
    }
};

} // namespace Ciphers
