#pragma once
/**
 * bench_common.h
 * ─────────────────────────────────────────────────────────────────────────────
 * Shared types, timing primitives, measurement scaffolding, and image-quality
 * metrics for the image-encryption benchmark suite.
 *
 * Timing:
 *   CLOCK_MONOTONIC_RAW  — immune to NTP / adjtime jitter
 *   rdtsc                — cycle-accurate for latency; normalised by cpufreq
 *
 * Memory:
 *   peak RSS via /proc/self/status (Linux)
 *
 * CPU counters (PAPI):
 *   PAPI_TOT_INS, PAPI_TOT_CYC — instructions & cycles (IPC proxy)
 *
 * Reproducibility:
 *   Each cipher run re-seeds from a fixed 256-bit key & 128-bit nonce so
 *   results are byte-identical across machines (given same cipher + libs).
 */

#include <cstdint>
#include <cstddef>
#include <cstring>
#include <cmath>
#include <string>
#include <vector>
#include <array>
#include <chrono>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <cassert>
#include <algorithm>
#include <numeric>
#include <functional>
#include <stdexcept>

// POSIX / Linux
#include <time.h>
#include <sys/resource.h>

// PAPI
#include <papi.h>

// OpenCV
#include <opencv2/opencv.hpp>

namespace Bench {

// ─────────────────────────────────────────────────────────────────────────────
//  Fixed key material (deterministic across all runs)
// ─────────────────────────────────────────────────────────────────────────────
static constexpr std::array<uint8_t,32> FIXED_KEY = {
    0x60,0x3d,0xeb,0x10,0x15,0xca,0x71,0xbe,
    0x2b,0x73,0xae,0xf0,0x85,0x7d,0x77,0x81,
    0x1f,0x35,0x2c,0x07,0x3b,0x61,0x08,0xd7,
    0x2d,0x98,0x10,0xa3,0x09,0x14,0xdf,0xf4
};
static constexpr std::array<uint8_t,16> FIXED_IV = {
    0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,
    0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f
};
static constexpr std::array<uint8_t,12> FIXED_NONCE12 = {
    0x00,0x01,0x02,0x03,0x04,0x05,
    0x06,0x07,0x08,0x09,0x0a,0x0b
};

// ─────────────────────────────────────────────────────────────────────────────
//  Timing: wall-clock (ns) and CPU cycles (rdtsc)
// ─────────────────────────────────────────────────────────────────────────────
inline uint64_t now_ns() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    return static_cast<uint64_t>(ts.tv_sec) * 1'000'000'000ULL + ts.tv_nsec;
}

inline uint64_t rdtsc() {
    uint32_t lo, hi;
    __asm__ volatile ("rdtsc" : "=a"(lo), "=d"(hi));
    return (static_cast<uint64_t>(hi) << 32) | lo;
}

// Serialise RDTSC so out-of-order execution doesn't skew the reading
inline uint64_t rdtsc_start() {
    uint32_t lo, hi;
    __asm__ volatile (
        "cpuid\n\t"
        "rdtsc\n\t"
        : "=a"(lo), "=d"(hi) :: "rbx", "rcx"
    );
    return (static_cast<uint64_t>(hi) << 32) | lo;
}
inline uint64_t rdtsc_end() {
    uint32_t lo, hi;
    __asm__ volatile (
        "rdtscp\n\t"
        "mov %%eax, %0\n\t"
        "mov %%edx, %1\n\t"
        "cpuid\n\t"
        : "=r"(lo), "=r"(hi) :: "rax", "rbx", "rcx", "rdx"
    );
    return (static_cast<uint64_t>(hi) << 32) | lo;
}

// ─────────────────────────────────────────────────────────────────────────────
//  Peak RSS (bytes) — Linux only
// ─────────────────────────────────────────────────────────────────────────────
inline long peak_rss_kb() {
    struct rusage ru;
    getrusage(RUSAGE_SELF, &ru);
    return ru.ru_maxrss;  // kB on Linux
}

inline long current_rss_kb() {
    std::ifstream f("/proc/self/status");
    std::string line;
    while (std::getline(f, line)) {
        if (line.rfind("VmRSS:", 0) == 0) {
            long kb = 0;
            sscanf(line.c_str(), "VmRSS: %ld kB", &kb);
            return kb;
        }
    }
    return -1;
}

// ─────────────────────────────────────────────────────────────────────────────
//  PAPI hardware counter session
// ─────────────────────────────────────────────────────────────────────────────
struct PapiSession {
    int  es = PAPI_NULL;
    bool ok = false;
    long long ins = 0, cyc = 0;

    PapiSession() {
        static bool papi_inited = false;
        if (!papi_inited) {
            int ret = PAPI_library_init(PAPI_VER_CURRENT);
            papi_inited = (ret == PAPI_VER_CURRENT);
        }
        if (PAPI_create_eventset(&es) != PAPI_OK) return;
        if (PAPI_add_event(es, PAPI_TOT_INS) != PAPI_OK) return;
        if (PAPI_add_event(es, PAPI_TOT_CYC) != PAPI_OK) return;
        ok = true;
    }
    ~PapiSession() { if (es != PAPI_NULL) PAPI_destroy_eventset(&es); }

    void start() { if (ok) PAPI_start(es); }
    void stop()  {
        if (!ok) return;
        long long vals[2] = {};
        PAPI_stop(es, vals);
        ins += vals[0];
        cyc += vals[1];
    }
    double ipc() const { return cyc > 0 ? static_cast<double>(ins) / cyc : 0.0; }
    void reset() { ins = 0; cyc = 0; }
};

// ─────────────────────────────────────────────────────────────────────────────
//  Per-run measurement result
// ─────────────────────────────────────────────────────────────────────────────
struct RunResult {
    // Timing
    uint64_t wall_ns    = 0;   // wall clock nanoseconds
    uint64_t cycles     = 0;   // RDTSC cycles
    // PAPI
    long long instructions = 0;
    long long cpu_cycles   = 0;
    double    ipc          = 0.0;
    // Memory
    long  rss_before_kb = 0;
    long  rss_after_kb  = 0;
    long  rss_delta_kb  = 0;
    // Payload
    size_t bytes = 0;
};

// ─────────────────────────────────────────────────────────────────────────────
//  Aggregated statistics across N warmup + M measurement runs
// ─────────────────────────────────────────────────────────────────────────────
struct Stats {
    double mean_ns   = 0;
    double median_ns = 0;
    double stddev_ns = 0;
    double min_ns    = 0;
    double max_ns    = 0;
    double cv        = 0;   // coefficient of variation (stddev/mean)

    double throughput_mbps = 0;   // MB/s  (mebibytes, 1024^2)
    double throughput_gbps = 0;   // GB/s

    double mean_ipc  = 0;
    double mean_cycles_per_byte = 0;
    long   rss_delta_kb = 0;

    size_t n      = 0;
    size_t bytes  = 0;

    static Stats compute(const std::vector<RunResult>& runs, int warmup) {
        Stats s;
        if (runs.size() <= static_cast<size_t>(warmup)) return s;

        std::vector<double> ns_vals;
        double sum_ipc = 0;
        double sum_cyc = 0;
        long   sum_rss = 0;

        for (size_t i = warmup; i < runs.size(); ++i) {
            ns_vals.push_back(static_cast<double>(runs[i].wall_ns));
            sum_ipc += runs[i].ipc;
            sum_cyc += static_cast<double>(runs[i].cycles);
            sum_rss += runs[i].rss_delta_kb;
        }

        s.n     = ns_vals.size();
        s.bytes = runs[warmup].bytes;

        std::sort(ns_vals.begin(), ns_vals.end());

        s.mean_ns   = std::accumulate(ns_vals.begin(), ns_vals.end(), 0.0) / s.n;
        s.median_ns = ns_vals[s.n / 2];
        s.min_ns    = ns_vals.front();
        s.max_ns    = ns_vals.back();

        double var = 0;
        for (double v : ns_vals) var += (v - s.mean_ns) * (v - s.mean_ns);
        s.stddev_ns = std::sqrt(var / s.n);
        s.cv        = s.mean_ns > 0 ? s.stddev_ns / s.mean_ns : 0;

        double secs = s.mean_ns / 1e9;
        s.throughput_mbps = (static_cast<double>(s.bytes) / (1024.0 * 1024.0)) / secs;
        s.throughput_gbps = s.throughput_mbps / 1024.0;

        s.mean_ipc               = sum_ipc / s.n;
        s.mean_cycles_per_byte   = (sum_cyc / s.n) / static_cast<double>(s.bytes);
        s.rss_delta_kb           = sum_rss / static_cast<long>(s.n);

        return s;
    }
};

// ─────────────────────────────────────────────────────────────────────────────
//  Image quality / cryptographic metrics
// ─────────────────────────────────────────────────────────────────────────────
struct ImageMetrics {
    double entropy       = 0;
    double npcr          = 0;
    double uaci          = 0;
    double corr_h        = 0;
    double corr_v        = 0;
    double corr_d        = 0;
    double chi2          = 0;
    double psnr_decrypt  = 0;   // ∞ → perfect
    double mse_decrypt   = 0;
    bool   lossless      = false;
};

inline double shannon_entropy(const cv::Mat& img) {
    size_t freq[256] = {};
    size_t total = static_cast<size_t>(img.rows) * img.cols * img.channels();
    const uint8_t* d = img.data;
    for (size_t i = 0; i < total; ++i) freq[d[i]]++;
    double H = 0;
    for (int v = 0; v < 256; ++v) {
        if (!freq[v]) continue;
        double p = static_cast<double>(freq[v]) / total;
        H -= p * std::log2(p);
    }
    return H;
}

inline double compute_npcr(const cv::Mat& a, const cv::Mat& b) {
    int H = a.rows, W = a.cols, C = a.channels(), N = H * W;
    int diff = 0;
    for (int y = 0; y < H; ++y) {
        const uint8_t *pa = a.ptr<uint8_t>(y), *pb = b.ptr<uint8_t>(y);
        for (int x = 0; x < W; ++x) {
            for (int c = 0; c < C; ++c)
                if (pa[x*C+c] != pb[x*C+c]) { diff++; break; }
        }
    }
    return 100.0 * diff / N;
}

inline double compute_uaci(const cv::Mat& a, const cv::Mat& b) {
    int H = a.rows, W = a.cols, C = a.channels();
    double sum = 0;
    for (int y = 0; y < H; ++y) {
        const uint8_t *pa = a.ptr<uint8_t>(y), *pb = b.ptr<uint8_t>(y);
        for (int x = 0; x < W*C; ++x)
            sum += std::abs((int)pa[x] - (int)pb[x]);
    }
    return 100.0 * sum / ((double)H * W * C * 255.0);
}

inline double correlation_coeff(const cv::Mat& img, char dir) {
    int H = img.rows, W = img.cols;
    const int NS = std::min(5000, H*W);
    double sx=0,sy=0,sxy=0,sx2=0,sy2=0; int n=0;
    srand(42);
    for (int k = 0; k < NS; ++k) {
        int y = rand() % (H-1), x = rand() % (W-1);
        int ny = y + (dir=='V'||dir=='D' ? 1:0);
        int nx = x + (dir=='H'||dir=='D' ? 1:0);
        double xi = img.at<cv::Vec3b>(y,x)[0];
        double yi = img.at<cv::Vec3b>(ny,nx)[0];
        sx+=xi; sy+=yi; sxy+=xi*yi; sx2+=xi*xi; sy2+=yi*yi; n++;
    }
    double num = n*sxy - sx*sy;
    double den = std::sqrt((n*sx2-sx*sx)*(n*sy2-sy*sy));
    return den < 1e-12 ? 0.0 : num/den;
}

inline double chi2_uniform(const cv::Mat& img) {
    size_t freq[256]={};
    size_t total = static_cast<size_t>(img.rows)*img.cols*img.channels();
    for (size_t i=0;i<total;++i) freq[img.data[i]]++;
    double exp = (double)total/256.0, chi2=0;
    for (int i=0;i<256;++i) { double d=freq[i]-exp; chi2+=d*d/exp; }
    return chi2;
}

inline ImageMetrics compute_image_metrics(const cv::Mat& plain,
                                           const cv::Mat& cipher,
                                           const cv::Mat& recovered) {
    ImageMetrics m;
    m.entropy  = shannon_entropy(cipher);
    m.npcr     = compute_npcr(plain, cipher);
    m.uaci     = compute_uaci(plain, cipher);
    m.corr_h   = correlation_coeff(cipher, 'H');
    m.corr_v   = correlation_coeff(cipher, 'V');
    m.corr_d   = correlation_coeff(cipher, 'D');
    m.chi2     = chi2_uniform(cipher);

    if (!recovered.empty()) {
        cv::Mat diff; cv::absdiff(plain, recovered, diff);
        diff.convertTo(diff, CV_64F);
        cv::multiply(diff, diff, diff);
        double mse = (cv::sum(diff)[0]+cv::sum(diff)[1]+cv::sum(diff)[2])
                     / (plain.rows*plain.cols*plain.channels());
        m.mse_decrypt  = mse;
        m.lossless     = (mse < 1e-12);
        m.psnr_decrypt = m.lossless ? std::numeric_limits<double>::infinity()
                                    : 10.0*std::log10(255.0*255.0/mse);
    }
    return m;
}

// ─────────────────────────────────────────────────────────────────────────────
//  Full benchmark result for one cipher × one image
// ─────────────────────────────────────────────────────────────────────────────
struct BenchRecord {
    std::string cipher_name;
    std::string image_name;
    int         width = 0, height = 0, channels = 0;
    size_t      bytes = 0;

    Stats       enc_stats;
    Stats       dec_stats;

    ImageMetrics img_metrics;

    // Code complexity (lines of relevant cipher code, filled by caller)
    int loc = 0;
    // Reproducibility flag (ciphertext hash matches reference)
    bool reproducible = true;
    // Ciphertext SHA256 hex (first 16 chars for table)
    std::string ciphertext_hash;
};

// ─────────────────────────────────────────────────────────────────────────────
//  Measurement harness: run fn() N+warmup times, collect RunResult each time
// ─────────────────────────────────────────────────────────────────────────────
inline std::vector<RunResult> measure(std::function<void()> fn,
                                       size_t payload_bytes,
                                       int warmup = 3,
                                       int runs   = 15) {
    std::vector<RunResult> results;
    results.reserve(warmup + runs);

    PapiSession papi;

    for (int i = 0; i < warmup + runs; ++i) {
        RunResult r;
        r.bytes        = payload_bytes;
        r.rss_before_kb = current_rss_kb();

        papi.reset();
        papi.start();
        uint64_t t0 = now_ns();
        uint64_t c0 = rdtsc_start();

        fn();

        uint64_t c1 = rdtsc_end();
        uint64_t t1 = now_ns();
        papi.stop();

        r.wall_ns       = t1 - t0;
        r.cycles        = c1 - c0;
        r.instructions  = papi.ins;
        r.cpu_cycles    = papi.cyc;
        r.ipc           = papi.ipc();
        r.rss_after_kb  = current_rss_kb();
        r.rss_delta_kb  = r.rss_after_kb - r.rss_before_kb;

        results.push_back(r);
    }
    return results;
}

// ─────────────────────────────────────────────────────────────────────────────
//  CSV / console output helpers
// ─────────────────────────────────────────────────────────────────────────────
inline std::string csv_header() {
    return "cipher,image,W,H,bytes,"
           "enc_mean_ns,enc_median_ns,enc_stddev_ns,enc_cv,"
           "enc_throughput_MBs,enc_cycles_per_byte,enc_ipc,"
           "dec_mean_ns,dec_throughput_MBs,dec_cycles_per_byte,"
           "rss_delta_kb,"
           "entropy,npcr,uaci,corr_h,corr_v,corr_d,chi2,"
           "psnr_decrypt,lossless,loc,reproducible,ciphertext_hash\n";
}

inline std::string to_csv(const BenchRecord& b) {
    auto f = [](double v, int p=4) {
        std::ostringstream ss; ss << std::fixed << std::setprecision(p) << v;
        return ss.str();
    };
    auto& e = b.enc_stats;
    auto& d = b.dec_stats;
    auto& m = b.img_metrics;
    std::string psnr_str = std::isinf(m.psnr_decrypt) ? "inf" : f(m.psnr_decrypt,2);

    return b.cipher_name + "," + b.image_name + ","
        + std::to_string(b.width)  + "," + std::to_string(b.height) + ","
        + std::to_string(b.bytes)  + ","
        + f(e.mean_ns,1)   + "," + f(e.median_ns,1)  + ","
        + f(e.stddev_ns,1) + "," + f(e.cv,4)          + ","
        + f(e.throughput_mbps,2) + "," + f(e.mean_cycles_per_byte,4) + ","
        + f(e.mean_ipc,3)  + ","
        + f(d.mean_ns,1)   + "," + f(d.throughput_mbps,2) + ","
        + f(d.mean_cycles_per_byte,4) + ","
        + std::to_string(e.rss_delta_kb) + ","
        + f(m.entropy,6)   + "," + f(m.npcr,4)  + "," + f(m.uaci,4) + ","
        + f(m.corr_h,6)    + "," + f(m.corr_v,6)+ "," + f(m.corr_d,6) + ","
        + f(m.chi2,2)      + ","
        + psnr_str         + "," + (m.lossless?"1":"0") + ","
        + std::to_string(b.loc) + "," + (b.reproducible?"1":"0") + ","
        + b.ciphertext_hash + "\n";
}

} // namespace Bench
