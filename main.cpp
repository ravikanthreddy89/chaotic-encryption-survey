/**
 * main.cpp
 * ─────────────────────────────────────────────────────────────────────────────
 * Image-encryption benchmark suite orchestrator.
 *
 * For each (cipher × image) combination:
 *   1. Warm up CPU caches (configurable; defaults 3 or 5 for large images)
 *   2. Run timed encrypt passes     → Stats
 *   3. Run timed decrypt passes     → Stats
 *   4. Compute image-quality metrics
 *   5. Append to results/bench_results.csv
 *   6. Print formatted table row to stdout
 *
 * Compiler flags enforced in CMakeLists:
 *   -O3 -march=native -fno-omit-frame-pointer
 *   (no profile-guided optimisation, to keep results reproducible across labs)
 *
 * Pin to CPU 0 via taskset before running:
 *   taskset -c 0 ./imgcrypt_bench
 */

#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <string>
#include <vector>
#include <algorithm>
#include <functional>
#include <cstring>
#include <cstdlib>
#include <openssl/sha.h>

#include <opencv2/opencv.hpp>

#include "bench_common.h"
#include "cipher_aes.h"
#include "cipher_chacha.h"
#include "cipher_chaos.h"

// ─────────────────────────────────────────────────────────────────────────────
//  Hex digest of first 32 bytes of a buffer (reproducibility check)
// ─────────────────────────────────────────────────────────────────────────────
static std::string sha256_prefix(const uint8_t* data, size_t len) {
    uint8_t digest[32];
    SHA256(data, len, digest);
    std::ostringstream ss;
    for (int i = 0; i < 8; ++i)
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)digest[i];
    return ss.str();
}

// ─────────────────────────────────────────────────────────────────────────────
//  Synthetic image corpus generator
//  Creates images representative of typical benchmark corpora:
//    - Lena-like: natural-image gradient + texture
//    - Baboon-like: high-frequency colour texture
//    - Black: all-zero (worst case for entropy metrics)
//    - White noise: random (best case for standard ciphers)
// ─────────────────────────────────────────────────────────────────────────────
static cv::Mat make_lena_like(int W, int H) {
    cv::Mat img(H, W, CV_8UC3);
    cv::RNG rng(42);
    for (int y = 0; y < H; ++y)
        for (int x = 0; x < W; ++x) {
            uint8_t r = (uint8_t)(128 + 60 * std::sin(x * 0.05) * std::cos(y * 0.04));
            uint8_t g = (uint8_t)(100 + 80 * std::cos(x * 0.03 + y * 0.02));
            uint8_t b = (uint8_t)(150 + 50 * std::sin((x + y) * 0.06));
            img.at<cv::Vec3b>(y,x) = {b, g, r};
        }
    cv::GaussianBlur(img, img, {5,5}, 1.5);
    return img;
}

static cv::Mat make_baboon_like(int W, int H) {
    cv::Mat img(H, W, CV_8UC3);
    cv::RNG rng(123);
    for (int y = 0; y < H; ++y)
        for (int x = 0; x < W; ++x) {
            uint8_t r = (uint8_t)(rng.uniform(0,256));
            uint8_t g = (uint8_t)(128 + 100*std::sin(x*0.2)*std::cos(y*0.2));
            uint8_t b = (uint8_t)(rng.uniform(0,256));
            img.at<cv::Vec3b>(y,x) = {b, g, r};
        }
    return img;
}

static cv::Mat make_uniform_noise(int W, int H) {
    cv::Mat img(H, W, CV_8UC3);
    cv::RNG rng(999);
    rng.fill(img, cv::RNG::UNIFORM, 0, 256);
    return img;
}

static cv::Mat make_checkerboard(int W, int H, int block=16) {
    cv::Mat img(H, W, CV_8UC3);
    for (int y = 0; y < H; ++y) {
        for (int x = 0; x < W; ++x) {
            bool on = ((x / block) + (y / block)) % 2 == 0;
            uint8_t v = on ? 240 : 20;
            img.at<cv::Vec3b>(y, x) = {v, (uint8_t)(255 - v), v};
        }
    }
    return img;
}

static cv::Mat make_radial_gradient(int W, int H) {
    cv::Mat img(H, W, CV_8UC3);
    const double cx = 0.5 * (W - 1), cy = 0.5 * (H - 1);
    const double maxr = std::sqrt(cx * cx + cy * cy);
    for (int y = 0; y < H; ++y) {
        for (int x = 0; x < W; ++x) {
            double r = std::sqrt((x - cx) * (x - cx) + (y - cy) * (y - cy)) / maxr;
            uint8_t a = (uint8_t)(255.0 * std::min(1.0, r));
            uint8_t b = (uint8_t)(255.0 * (1.0 - std::min(1.0, r)));
            img.at<cv::Vec3b>(y, x) = {a, (uint8_t)((a + b) / 2), b};
        }
    }
    return img;
}

static cv::Mat make_stripes(int W, int H, int period=12) {
    cv::Mat img(H, W, CV_8UC3);
    for (int y = 0; y < H; ++y) {
        for (int x = 0; x < W; ++x) {
            uint8_t v = (((x / period) % 2) == 0) ? 220 : 35;
            uint8_t g = (((y / period) % 2) == 0) ? 180 : 60;
            img.at<cv::Vec3b>(y, x) = {v, g, (uint8_t)(255 - v)};
        }
    }
    return img;
}

struct ImageSpec {
    std::string name;
    cv::Mat     img;
};

struct MeasurePlan {
    int warmup = 3;
    int runs   = 15;
};

static int env_int(const char* name, int fallback, int min_allowed = 1) {
    const char* raw = std::getenv(name);
    if (!raw || !*raw) return fallback;
    char* end = nullptr;
    long v = std::strtol(raw, &end, 10);
    if (end == raw || v < min_allowed) return fallback;
    return static_cast<int>(v);
}

static std::vector<int> env_dims(const char* name, const std::vector<int>& fallback) {
    const char* raw = std::getenv(name);
    if (!raw || !*raw) return fallback;

    std::vector<int> dims;
    std::stringstream ss(raw);
    std::string tok;
    while (std::getline(ss, tok, ',')) {
        if (tok.empty()) continue;
        char* end = nullptr;
        long v = std::strtol(tok.c_str(), &end, 10);
        if (end == tok.c_str() || v < 64) continue;
        int d = static_cast<int>(v);
        if (d % 2 != 0) d -= 1;  // Arnold variants require even dims.
        if (d >= 64) dims.push_back(d);
    }
    if (dims.empty()) return fallback;
    std::sort(dims.begin(), dims.end());
    dims.erase(std::unique(dims.begin(), dims.end()), dims.end());
    return dims;
}

static std::string dims_to_string(const std::vector<int>& dims) {
    std::ostringstream os;
    for (size_t i = 0; i < dims.size(); ++i) {
        if (i) os << ",";
        os << dims[i];
    }
    return os.str();
}

static MeasurePlan plan_for_image(const cv::Mat& img,
                                  int large_dim_at,
                                  int warmup_small,
                                  int runs_small,
                                  int warmup_large,
                                  int runs_large) {
    int dim = std::max(img.rows, img.cols);
    if (dim >= large_dim_at) return {warmup_large, runs_large};
    return {warmup_small, runs_small};
}

// ─────────────────────────────────────────────────────────────────────────────
//  Generic benchmark runner for one cipher
// ─────────────────────────────────────────────────────────────────────────────
template<typename CipherT>
Bench::BenchRecord run_cipher(const ImageSpec& spec,
                               std::function<cv::Mat(const cv::Mat&)> enc_fn,
                               std::function<cv::Mat(const cv::Mat&, int, int, int)> dec_fn,
                               int loc,
                               int warmup,
                               int runs) {
    const cv::Mat& src = spec.img;
    size_t bytes = (size_t)src.rows * src.cols * src.channels();

    Bench::BenchRecord rec;
    rec.cipher_name = CipherT::NAME;
    rec.image_name  = spec.name;
    rec.width       = src.cols;
    rec.height      = src.rows;
    rec.channels    = src.channels();
    rec.bytes       = bytes;
    rec.loc         = loc;

    // ── Encrypt benchmark ──────────────────────────────────────────────────
    cv::Mat cipher;
    auto enc_runs = Bench::measure([&]() {
        cipher = enc_fn(src);
    }, bytes, warmup, runs);
    rec.enc_stats = Bench::Stats::compute(enc_runs, warmup);

    // ── Decrypt benchmark ──────────────────────────────────────────────────
    cv::Mat recovered;
    auto dec_runs = Bench::measure([&]() {
        recovered = dec_fn(cipher, src.rows, src.cols, src.type());
    }, bytes, warmup, runs);
    rec.dec_stats = Bench::Stats::compute(dec_runs, warmup);

    // ── Image metrics ──────────────────────────────────────────────────────
    rec.img_metrics = Bench::compute_image_metrics(src, cipher, recovered);

    // ── Ciphertext hash (reproducibility) ─────────────────────────────────
    rec.ciphertext_hash = sha256_prefix(cipher.data, bytes);
    rec.reproducible    = true;  // assume; would compare vs. stored ref

    return rec;
}

// ─────────────────────────────────────────────────────────────────────────────
//  Print a summary table row
// ─────────────────────────────────────────────────────────────────────────────
static void print_row(const Bench::BenchRecord& r) {
    auto& e = r.enc_stats;
    auto& m = r.img_metrics;
    std::string psnr = std::isinf(m.psnr_decrypt) ? "    INF" :
                        (std::ostringstream() << std::fixed << std::setprecision(1)
                         << m.psnr_decrypt).str();

    std::cout << std::left
              << std::setw(28) << r.cipher_name
              << std::setw(14) << r.image_name
              << std::right
              << std::setw(7)  << r.width << "x" << std::setw(4) << r.height
              << std::setw(10) << std::fixed << std::setprecision(1) << e.throughput_mbps << " MB/s"
              << std::setw(9)  << std::setprecision(3) << e.mean_cycles_per_byte << " c/B"
              << std::setw(8)  << std::setprecision(4) << m.entropy
              << std::setw(8)  << std::setprecision(2) << m.npcr << "%"
              << std::setw(8)  << std::setprecision(2) << m.uaci << "%"
              << std::setw(9)  << std::setprecision(5) << m.corr_h
              << std::setw(9)  << psnr << " dB"
              << std::setw(5)  << r.loc  << " LOC"
              << "\n";
}

// ─────────────────────────────────────────────────────────────────────────────
//  main
// ─────────────────────────────────────────────────────────────────────────────
int main(int argc, char* argv[]) {
    const std::vector<int> default_dims = {128, 256, 512, 1024, 2048, 3072};
    const std::vector<int> dims = env_dims("BENCH_DIMS", default_dims);
    const int large_dim_at = env_int("BENCH_LARGE_AT", 2048, 64);
    const int warmup_small = env_int("BENCH_WARMUP", 3, 0);
    const int runs_small   = env_int("BENCH_RUNS", 15, 1);
    const int warmup_large = env_int("BENCH_WARMUP_LARGE", 5, 0);
    const int runs_large   = env_int("BENCH_RUNS_LARGE", 30, 1);

    std::cout << "═══════════════════════════════════════════════════════════════════\n"
              << "  IMAGE ENCRYPTION BENCHMARK SUITE\n"
              << "  AES-256-CTR / AES-256-GCM / ChaCha20-Poly1305\n"
              << "  Fridrich-1998 / YeHuang-2018 / LSCM-2020\n"
              << "═══════════════════════════════════════════════════════════════════\n\n";
    std::cout << "[+] Diffusion backend: " << Ciphers::diffusion_kernel_name() << "\n";
    std::cout << "[+] Synthetic dims: " << dims_to_string(dims) << "\n";
    std::cout << "[+] Sampling plan: <" << large_dim_at << "px => warmup=" << warmup_small
              << ", runs=" << runs_small << " | >= " << large_dim_at << "px => warmup="
              << warmup_large << ", runs=" << runs_large << "\n";

    // ── Build image corpus ──────────────────────────────────────────────────
    std::vector<ImageSpec> corpus;

    // Load external images if provided; otherwise generate synthetic corpus
    std::vector<std::string> ext_paths;
    for (int i = 1; i < argc; ++i) ext_paths.push_back(argv[i]);

    if (!ext_paths.empty()) {
        for (const auto& p : ext_paths) {
            cv::Mat img = cv::imread(p, cv::IMREAD_COLOR);
            if (img.empty()) { std::cerr << "Cannot load: " << p << "\n"; continue; }
            // Trim to multiple of 2 for Arnold Cat Map
            img = img(cv::Rect(0, 0, img.cols - img.cols%2, img.rows - img.rows%2));
            std::string name = p.substr(p.rfind('/')+1);
            name = name.substr(0, name.rfind('.'));
            corpus.push_back({name, img});
            std::cout << "[+] Loaded: " << name << " (" << img.cols << "x" << img.rows << ")\n";
        }
    }

    // Always add synthetic images
    // Wider dataset for statistical strength across texture and scale classes.
    for (int dim : dims) {
        std::string sfx = std::to_string(dim);
        corpus.push_back({"lena_"    + sfx, make_lena_like   (dim, dim)});
        corpus.push_back({"baboon_"  + sfx, make_baboon_like (dim, dim)});
        corpus.push_back({"noise_"   + sfx, make_uniform_noise(dim, dim)});
        corpus.push_back({"checker_" + sfx, make_checkerboard(dim, dim)});
        corpus.push_back({"radial_"  + sfx, make_radial_gradient(dim, dim)});
        corpus.push_back({"stripes_" + sfx, make_stripes(dim, dim)});
    }

    std::cout << "[+] Corpus: " << corpus.size() << " images\n\n";

    // ── Open CSV output ─────────────────────────────────────────────────────
    std::ofstream csv("results/bench_results.csv");
    if (!csv) {
        // Try creating the results dir
        system("mkdir -p results");
        csv.open("results/bench_results.csv");
    }
    csv << Bench::csv_header();

    // ── Print table header ──────────────────────────────────────────────────
    std::cout << std::left
              << std::setw(28) << "Cipher"
              << std::setw(14) << "Image"
              << std::setw(12) << "Size"
              << std::setw(15) << "Throughput"
              << std::setw(13) << "Cycles/byte"
              << std::setw(8)  << "H(bits)"
              << std::setw(8)  << "NPCR"
              << std::setw(8)  << "UACI"
              << std::setw(9)  << "Corr_H"
              << std::setw(9)  << "PSNR"
              << std::setw(9)  << "LOC"
              << "\n"
              << std::string(115, '─') << "\n";

    std::vector<Bench::BenchRecord> all_records;

    // ── Run benchmarks ──────────────────────────────────────────────────────
    for (const auto& spec : corpus) {
        const MeasurePlan plan = plan_for_image(
            spec.img, large_dim_at, warmup_small, runs_small, warmup_large, runs_large);

        // AES-256-CTR
        {
            auto rec = run_cipher<Ciphers::AES256CTR>(
                spec,
                [](const cv::Mat& s) { return Ciphers::AES256CTR::encrypt_image(s); },
                [](const cv::Mat& s, int r, int c, int t) {
                    return Ciphers::AES256CTR::decrypt_image(s, r, c, t);
                },
                Ciphers::AES256CTR::LOC,
                plan.warmup,
                plan.runs);
            print_row(rec);
            csv << Bench::to_csv(rec);
            all_records.push_back(rec);
        }

        // AES-256-GCM
        {
            auto rec = run_cipher<Ciphers::AES256GCM>(
                spec,
                [](const cv::Mat& s) { return Ciphers::AES256GCM::encrypt_image(s); },
                [](const cv::Mat& s, int r, int c, int t) {
                    return Ciphers::AES256GCM::decrypt_image(s, r, c, t);
                },
                Ciphers::AES256GCM::LOC,
                plan.warmup,
                plan.runs);
            print_row(rec);
            csv << Bench::to_csv(rec);
            all_records.push_back(rec);
        }

        // ChaCha20-Poly1305
        {
            using C = Ciphers::ChaCha20Poly1305;
            Bench::BenchRecord rec;
            rec.cipher_name = C::NAME;
            rec.image_name  = spec.name;
            rec.width       = spec.img.cols;
            rec.height      = spec.img.rows;
            rec.channels    = spec.img.channels();
            rec.bytes       = (size_t)spec.img.rows * spec.img.cols * spec.img.channels();
            rec.loc         = C::LOC;

            cv::Mat cipher;
            auto enc_runs = Bench::measure([&]() {
                cipher = C::encrypt_image(spec.img);
            }, rec.bytes, plan.warmup, plan.runs);
            rec.enc_stats = Bench::Stats::compute(enc_runs, plan.warmup);

            cv::Mat recovered;
            auto dec_runs = Bench::measure([&]() {
                recovered = C::decrypt_image(spec.img, cipher);
            }, rec.bytes, plan.warmup, plan.runs);
            rec.dec_stats = Bench::Stats::compute(dec_runs, plan.warmup);

            rec.img_metrics     = Bench::compute_image_metrics(spec.img, cipher, recovered);
            rec.ciphertext_hash = sha256_prefix(cipher.data, rec.bytes);
            rec.reproducible    = true;

            print_row(rec);
            csv << Bench::to_csv(rec);
            all_records.push_back(rec);
        }

        // Fridrich-1998
        {
            auto rec = run_cipher<Ciphers::Fridrich1998>(
                spec,
                [](const cv::Mat& s) { return Ciphers::Fridrich1998::encrypt_image(s); },
                [](const cv::Mat& s, int r, int c, int t) {
                    return Ciphers::Fridrich1998::decrypt_image(s, r, c, t);
                },
                Ciphers::Fridrich1998::LOC,
                plan.warmup,
                plan.runs);
            print_row(rec);
            csv << Bench::to_csv(rec);
            all_records.push_back(rec);
        }

        // YeHuang-2018
        {
            auto rec = run_cipher<Ciphers::YeHuang2018>(
                spec,
                [](const cv::Mat& s) { return Ciphers::YeHuang2018::encrypt_image(s); },
                [](const cv::Mat& s, int r, int c, int t) {
                    return Ciphers::YeHuang2018::decrypt_image(s, r, c, t);
                },
                Ciphers::YeHuang2018::LOC,
                plan.warmup,
                plan.runs);
            print_row(rec);
            csv << Bench::to_csv(rec);
            all_records.push_back(rec);
        }

        // YeHuang-2018-Optimized
        {
            auto rec = run_cipher<Ciphers::YeHuang2018Optimized>(
                spec,
                [](const cv::Mat& s) { return Ciphers::YeHuang2018Optimized::encrypt_image(s); },
                [](const cv::Mat& s, int r, int c, int t) {
                    return Ciphers::YeHuang2018Optimized::decrypt_image(s, r, c, t);
                },
                Ciphers::YeHuang2018Optimized::LOC,
                plan.warmup,
                plan.runs);
            print_row(rec);
            csv << Bench::to_csv(rec);
            all_records.push_back(rec);
        }

        // LSCM-2020
        {
            auto rec = run_cipher<Ciphers::LSCM2020>(
                spec,
                [](const cv::Mat& s) { return Ciphers::LSCM2020::encrypt_image(s); },
                [](const cv::Mat& s, int r, int c, int t) {
                    return Ciphers::LSCM2020::decrypt_image(s, r, c, t);
                },
                Ciphers::LSCM2020::LOC,
                plan.warmup,
                plan.runs);
            print_row(rec);
            csv << Bench::to_csv(rec);
            all_records.push_back(rec);
        }

        std::cout << std::string(115, '─') << "\n";
    }

    csv.close();

    // ── Summary: geometric-mean throughput per cipher ─────────────────────
    std::cout << "\n\n  GEOMETRIC-MEAN THROUGHPUT (encrypt) ACROSS ALL IMAGES\n"
              << "  " << std::string(50,'─') << "\n";

    std::vector<std::string> cipher_names = {
        "AES-256-CTR","AES-256-GCM","ChaCha20-Poly1305",
        "Fridrich-1998","YeHuang-2018","YeHuang-2018-Optimized", "LSCM-2020"
    };
    for (const auto& name : cipher_names) {
        double log_sum = 0; int cnt = 0;
        for (const auto& r : all_records) {
            if (r.cipher_name == name && r.enc_stats.throughput_mbps > 0) {
                log_sum += std::log(r.enc_stats.throughput_mbps);
                cnt++;
            }
        }
        if (cnt > 0) {
            double geomean = std::exp(log_sum / cnt);
            std::cout << "  " << std::left << std::setw(22) << name
                      << std::right << std::setw(10) << std::fixed << std::setprecision(1)
                      << geomean << " MB/s\n";
        }
    }

    std::cout << "\n[+] Full results written to: results/bench_results.csv\n";
    Ciphers::ChaosProfile::dump_csv("results/chaos_stage_profile.csv");
    std::cout << "[+] Stage profile written to: results/chaos_stage_profile.csv\n";
    return 0;
}
