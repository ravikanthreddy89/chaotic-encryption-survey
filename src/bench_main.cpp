#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <vector>

#include <opencv2/opencv.hpp>

#include "analysis/Metrics.hpp"
#include "core/IImageCipher.hpp"
#include "core/StageSuite.hpp"
#include "core/Utils.hpp"
#include "crypto/EvpCiphers.hpp"
#include "scalar/ChaoticCiphers.hpp"

namespace {

using bench::CipherContext;
using bench::CipherPtr;

struct Cli {
    std::string input = "images/sample.png";
    std::string results_dir = "results";
    bool avx2 = true;
    bool run_full = true;
    bool run_stages = true;
    bool run_candidates = true;
    bool include_slow_stages = true;
    bool include_slow_candidates = true;
};

bool parse_cli(int argc, char** argv, Cli& cli) {
    for (int i = 1; i < argc; ++i) {
        const std::string a = argv[i];
        if (a == "--input" && i + 1 < argc) {
            cli.input = argv[++i];
        } else if (a == "--results" && i + 1 < argc) {
            cli.results_dir = argv[++i];
        } else if (a == "--no-avx2") {
            cli.avx2 = false;
        } else if (a == "--fast-stages-only") {
            cli.include_slow_stages = false;
        } else if (a == "--fast-candidates-only") {
            cli.include_slow_candidates = false;
        } else if (a == "--mode" && i + 1 < argc) {
            const std::string mode = argv[++i];
            cli.run_full = mode == "all" || mode == "full";
            cli.run_stages = mode == "all" || mode == "stages";
            cli.run_candidates = mode == "all" || mode == "candidates";
            if (!cli.run_full && !cli.run_stages && !cli.run_candidates) return false;
        } else if (a == "--help") {
            return false;
        } else {
            return false;
        }
    }
    return true;
}

cv::Mat generate_sample_image(const std::string& path) {
    bench::ensure_dir(std::filesystem::path(path).parent_path().string());
    cv::Mat img(512, 512, CV_8UC3);
    for (int y = 0; y < img.rows; ++y) {
        for (int x = 0; x < img.cols; ++x) {
            img.at<cv::Vec3b>(y, x) = cv::Vec3b(
                static_cast<uint8_t>((x + y) & 0xFF),
                static_cast<uint8_t>((2 * x + y / 2) & 0xFF),
                static_cast<uint8_t>((x / 2 + 3 * y) & 0xFF));
        }
    }
    cv::imwrite(path, img);
    return img;
}

std::vector<CipherPtr> make_all_ciphers() {
    std::vector<CipherPtr> all = bench::make_chaotic_ciphers();
    std::vector<CipherPtr> evp = bench::make_evp_ciphers();
    for (auto& c : evp) all.push_back(std::move(c));
    return all;
}

bool equal_mat(const cv::Mat& a, const cv::Mat& b) {
    if (a.size() != b.size() || a.type() != b.type()) return false;
    return cv::countNonZero(a.reshape(1) != b.reshape(1)) == 0;
}

cv::Mat perturb_plain(const cv::Mat& plain) {
    cv::Mat out = plain.clone();
    out.data[0] ^= 1U;
    return out;
}

CipherContext perturbed_key(CipherContext ctx) {
    if (ctx.key.empty()) ctx.key.resize(32, 0);
    ctx.key[0] ^= 1U;
    return ctx;
}

std::string csv_escape(const std::string& s) {
    if (s.find_first_of(",\"\n") == std::string::npos) return s;
    std::string out = "\"";
    for (char c : s) {
        if (c == '"') out += "\"\"";
        else out += c;
    }
    out += '"';
    return out;
}

}  // namespace

int main(int argc, char** argv) {
    Cli cli;
    if (!parse_cli(argc, argv, cli)) {
        std::cerr << "usage: " << argv[0]
                  << " [--input images/sample.png] [--results results] [--mode all|full|stages|candidates]"
                  << " [--fast-stages-only] [--fast-candidates-only] [--no-avx2]\n";
        return 1;
    }

    bench::ensure_dir(cli.results_dir);
    bench::ensure_dir(cli.results_dir + "/encrypted");
    bench::ensure_dir(cli.results_dir + "/histograms");

    cv::Mat plain = cv::imread(cli.input, cv::IMREAD_UNCHANGED);
    if (plain.empty()) {
        plain = generate_sample_image(cli.input);
    }
    if (plain.depth() != CV_8U) {
        plain.convertTo(plain, CV_8U);
    }
    plain = bench::make_contiguous(plain);

    CipherContext ctx;
    ctx.key.resize(32);
    ctx.nonce.resize(16);
    for (size_t i = 0; i < ctx.key.size(); ++i) ctx.key[i] = static_cast<uint8_t>(0x41 + i * 13);
    for (size_t i = 0; i < ctx.nonce.size(); ++i) ctx.nonce[i] = static_cast<uint8_t>(0x17 + i * 7);
    ctx.use_avx2 = cli.avx2;

    const std::string bench_csv_path = cli.results_dir + "/bench.csv";
    const std::string analysis_csv_path = cli.results_dir + "/analysis.csv";
    const std::string stage_csv_path = cli.results_dir + "/stage_bench.csv";
    const std::string candidate_csv_path = cli.results_dir + "/candidate_schemes.csv";
    std::ofstream bench_csv(bench_csv_path);
    std::ofstream analysis_csv(analysis_csv_path);
    std::ofstream stage_csv(stage_csv_path);
    std::ofstream candidate_csv(candidate_csv_path);

    bench_csv << "image,cipher,variant,width,height,channels,bytes,keygen_ms,permutation_ms,diffusion_ms,total_ms,MBps,correct\n";
    analysis_csv << "image,cipher,entropy,chi_square,corr_h,corr_v,corr_d,npcr,uaci,key_sensitivity,kpa_score,cpa_score\n";
    stage_csv << "image,category,stage,expected_speed,research_value,width,height,channels,bytes,ms,MBps,checksum\n";
    candidate_csv << "image,scheme,keystream,permutation,diffusion,width,height,channels,bytes,keygen_ms,permutation_ms,diffusion_ms,total_ms,MBps,checksum\n";

    std::ostringstream summary;
    summary << "# Security Summary\n\n";
    summary << "Input image: `" << cli.input << "` (" << plain.cols << "x" << plain.rows << "x" << plain.channels() << ")\n\n";
    summary << "| cipher | MB/s | entropy | NPCR | UACI | key sensitivity | KPA score | CPA score | correct |\n";
    summary << "|---|---:|---:|---:|---:|---:|---:|---:|---:|\n";

    std::vector<bench::StageRow> stage_rows;
    if (cli.run_stages) {
        stage_rows = bench::run_replaceable_stage_bench(plain, ctx, cli.include_slow_stages);
        for (const auto& r : stage_rows) {
            stage_csv << csv_escape(cli.input) << ","
                      << r.category << ","
                      << r.stage << ","
                      << csv_escape(r.expected_speed) << ","
                      << csv_escape(r.research_value) << ","
                      << plain.cols << ","
                      << plain.rows << ","
                      << plain.channels() << ","
                      << r.bytes << ","
                      << r.ms << ","
                      << r.mbps << ","
                      << r.checksum << "\n";
        }
    }

    std::vector<bench::CandidateRow> candidate_rows;
    if (cli.run_candidates) {
        candidate_rows = bench::run_candidate_stage_schemes(plain, ctx, cli.include_slow_candidates);
        for (const auto& r : candidate_rows) {
            candidate_csv << csv_escape(cli.input) << ","
                          << csv_escape(r.scheme) << ","
                          << r.keystream << ","
                          << r.permutation << ","
                          << r.diffusion << ","
                          << plain.cols << ","
                          << plain.rows << ","
                          << plain.channels() << ","
                          << r.bytes << ","
                          << r.keygen_ms << ","
                          << r.permutation_ms << ","
                          << r.diffusion_ms << ","
                          << r.total_ms << ","
                          << r.mbps << ","
                          << r.checksum << "\n";
        }
    }

    cv::Mat changed_plain = perturb_plain(plain);
    cv::Mat zero_plain(plain.rows, plain.cols, plain.type(), cv::Scalar::all(0));

    if (cli.run_full) for (auto& cipher : make_all_ciphers()) {
        const std::string cipher_name = cipher->name();
        const std::string safe = bench::sanitize_name(cipher_name);

        bench::CipherResult enc = cipher->encrypt(plain, ctx);
        bench::CipherResult dec = cipher->decrypt_result(enc, ctx);
        const bool correct = equal_mat(plain, dec.image);
        const double mb = static_cast<double>(bench::image_bytes(plain)) / (1024.0 * 1024.0);
        const double mbps = enc.times.total_ms > 0.0 ? mb / (enc.times.total_ms / 1000.0) : 0.0;

        const std::string enc_path = cli.results_dir + "/encrypted/" + safe + ".png";
        const std::string hist_path = cli.results_dir + "/histograms/" + safe + "__hist.png";
        cv::imwrite(enc_path, enc.image);
        bench::write_histogram_plot(enc.image, hist_path);

        bench::AnalysisMetrics m = bench::base_metrics(enc.image);
        bench::CipherResult changed_enc = cipher->encrypt(changed_plain, ctx);
        m.npcr = bench::npcr(enc.image, changed_enc.image);
        m.uaci = bench::uaci(enc.image, changed_enc.image);

        CipherContext key2 = perturbed_key(ctx);
        bench::CipherResult key_changed_enc = cipher->encrypt(plain, key2);
        m.key_sensitivity = bench::npcr(enc.image, key_changed_enc.image);

        bench::CipherResult known_enc = cipher->encrypt(zero_plain, ctx);
        bench::CipherResult target_enc = cipher->encrypt(changed_plain, ctx);
        m.kpa_score = bench::xor_recovery_score(plain, enc.image, changed_plain, target_enc.image);
        m.cpa_score = bench::xor_recovery_score(zero_plain, known_enc.image, changed_plain, target_enc.image);

        bench_csv << csv_escape(cli.input) << ","
                  << cipher_name << ","
                  << cipher->variant() << ","
                  << plain.cols << ","
                  << plain.rows << ","
                  << plain.channels() << ","
                  << bench::image_bytes(plain) << ","
                  << enc.times.keygen_ms << ","
                  << enc.times.permutation_ms << ","
                  << enc.times.diffusion_ms << ","
                  << enc.times.total_ms << ","
                  << mbps << ","
                  << (correct ? 1 : 0) << "\n";

        analysis_csv << csv_escape(cli.input) << ","
                     << cipher_name << ","
                     << m.entropy << ","
                     << m.chi_square << ","
                     << m.corr_h << ","
                     << m.corr_v << ","
                     << m.corr_d << ","
                     << m.npcr << ","
                     << m.uaci << ","
                     << m.key_sensitivity << ","
                     << m.kpa_score << ","
                     << m.cpa_score << "\n";

        summary << "| " << cipher_name
                << " | " << bench::fixed(mbps, 2)
                << " | " << bench::fixed(m.entropy, 4)
                << " | " << bench::fixed(m.npcr, 2)
                << " | " << bench::fixed(m.uaci, 2)
                << " | " << bench::fixed(m.key_sensitivity, 2)
                << " | " << bench::fixed(m.kpa_score, 4)
                << " | " << bench::fixed(m.cpa_score, 4)
                << " | " << (correct ? 1 : 0)
                << " |\n";
    }

    summary << "\nKPA/CPA scores are empirical byte recovery ratios under deterministic key/nonce reuse. "
               "High values indicate a stream-XOR style vulnerability under reused keystream assumptions.\n";
    summary << "\n## Replaceable Stage Benchmarks\n\n";
    summary << "See `stage_bench.csv` for independent keystream, permutation, and diffusion primitive timings. "
               "See `candidate_schemes.csv` for the five SIMD-native candidate pipelines.\n\n";
    summary << "| candidate | keystream | permutation | diffusion | MB/s |\n";
    summary << "|---|---|---|---|---:|\n";
    for (const auto& r : candidate_rows) {
        summary << "| " << r.scheme << " | " << r.keystream << " | " << r.permutation
                << " | " << r.diffusion << " | " << bench::fixed(r.mbps, 2) << " |\n";
    }
    bench::write_text(cli.results_dir + "/security_summary.md", summary.str());

    std::cout << "Wrote " << bench_csv_path << "\n";
    std::cout << "Wrote " << analysis_csv_path << "\n";
    std::cout << "Wrote " << stage_csv_path << "\n";
    std::cout << "Wrote " << candidate_csv_path << "\n";
    std::cout << "Wrote " << cli.results_dir << "/security_summary.md\n";
    return 0;
}
