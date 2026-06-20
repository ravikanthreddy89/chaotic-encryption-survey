#include <filesystem>
#include <cstring>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <unordered_map>
#include <random>
#include <sstream>
#include <string>
#include <vector>

#include <opencv2/opencv.hpp>

#include "chaosref/bench.hpp"
#include "chaosref/catalog.hpp"
#include "chaosref/chaotic_rng.hpp"

namespace {

using chaosref::BenchCase;
using chaosref::DiffusionKernel;
using chaosref::PermutationKind;
using chaosref::SchemeConfig;

struct Cli {
    bool list = false;
    bool buckets = false;
    bool roadmap = false;
    bool bench = false;
    bool stages = false;
    bool microbench = false;
    bool precision = false;

    int size = 1024;
    int channels = 3;
    int warmup = 2;
    int iters = 6;
    int precision_samples = 32;
    int precision_max_steps = 1000000;
    int precision_bits = 16;
    uint64_t image_seed = 42;
    uint64_t order_seed = 2026;
    uint64_t precision_seed = 0x9E3779B97F4A7C15ULL;

    std::string scheme = "all";    // all | map | sort | bitplane | symbolic
    std::string kernel = "all";    // all | scalar | scan
    std::string csv_path = "results/bench_ref.csv";
    std::string stages_csv_path = "results/bench_stages.csv";
    std::string micro_csv_path = "results/bench_micro.csv";
    std::string precision_csv_path = "results/precision_orbits.csv";
    std::string input_path;
};

void print_usage(const char* argv0) {
    std::cout
        << "chaos_ref usage\n"
        << "  " << argv0 << " --list\n"
        << "  " << argv0 << " --buckets\n"
        << "  " << argv0 << " --roadmap\n"
        << "  " << argv0 << " --bench [--size N] [--channels C] [--warmup W] [--iters I]\\n"
        << "       [--scheme all|map|sort|bitplane|symbolic] [--kernel all|scalar|scan]\n"
        << "       [--order-seed N] [--csv PATH]\n"
        << "       [--input IMAGE] (uses the image dimensions instead of --size)\n"
        << "  " << argv0 << " --stages [same options] [--stages-csv PATH]\n"
        << "  " << argv0 << " --microbench [same options] [--micro-csv PATH]\n"
        << "  " << argv0 << " --precision [--precision-samples N] [--precision-max-steps N]\n"
        << "       [--precision-bits N] [--precision-seed N] [--precision-csv PATH]\n";
}

void print_list() {
    const auto& schemes = chaosref::scheme_catalog();
    std::cout << "State-of-practice chaotic image-encryption families\n";
    std::cout << "--------------------------------------------------\n";
    for (const auto& s : schemes) {
        std::cout << s.name << " [" << s.year << "]\n";
        std::cout << "  family: " << s.family << "\n";
        std::cout << "  permutation: " << s.permutation << "\n";
        std::cout << "  diffusion: " << s.diffusion << "\n";
        std::cout << "  keygen: " << s.keygen << "\n";
        std::cout << "  SIMD/AVX: " << s.simd_feasibility << "\n";
    }
}

void print_buckets() {
    std::cout << "Buckets for reference SIMD implementation\n";
    std::cout << "------------------------------------------\n";
    std::cout << "1. Permutation-only maps (Arnold/Baker/affine maps)\n";
    std::cout << "2. Sort-based permutation (chaotic sequence argsort)\n";
    std::cout << "3. Diffusion kernels\n";
    std::cout << "   - chain recurrence (exact scan form)\n";
    std::cout << "   - modular add/xor variants\n";
    std::cout << "4. Keystream generators\n";
    std::cout << "5. Optional symbolic layers (DNA/CA/bit-plane rules)\n";
}

void print_roadmap() {
    std::cout << "Reference optimization roadmap\n";
    std::cout << "------------------------------\n";
    std::cout << "Phase A: canonical scalar kernels + correctness tests\n";
    std::cout << "Phase B: AVX2/SSE diffusion kernels (exact equivalence)\n";
    std::cout << "Phase C: permutation kernels (map-based + sort-based)\n";
    std::cout << "Phase D: OpenMP parallel variants and scaling study\n";
    std::cout << "Phase E: reproducibility package (datasets, scripts, plots)\n";
}

bool parse_int_arg(const char* s, int& out) {
    try {
        out = std::stoi(s);
        return true;
    } catch (...) {
        return false;
    }
}

bool parse_u64_arg(const char* s, uint64_t& out) {
    try {
        out = static_cast<uint64_t>(std::stoull(s));
        return true;
    } catch (...) {
        return false;
    }
}

bool parse_cli(int argc, char** argv, Cli& cli) {
    for (int i = 1; i < argc; ++i) {
        const std::string a = argv[i];
        if (a == "--list") {
            cli.list = true;
        } else if (a == "--buckets") {
            cli.buckets = true;
        } else if (a == "--roadmap") {
            cli.roadmap = true;
        } else if (a == "--bench") {
            cli.bench = true;
        } else if (a == "--stages") {
            cli.stages = true;
        } else if (a == "--microbench") {
            cli.microbench = true;
        } else if (a == "--precision") {
            cli.precision = true;
        } else if (a == "--size" && i + 1 < argc) {
            if (!parse_int_arg(argv[++i], cli.size)) return false;
        } else if (a == "--channels" && i + 1 < argc) {
            if (!parse_int_arg(argv[++i], cli.channels)) return false;
        } else if (a == "--warmup" && i + 1 < argc) {
            if (!parse_int_arg(argv[++i], cli.warmup)) return false;
        } else if (a == "--iters" && i + 1 < argc) {
            if (!parse_int_arg(argv[++i], cli.iters)) return false;
        } else if (a == "--seed" && i + 1 < argc) {
            if (!parse_u64_arg(argv[++i], cli.image_seed)) return false;
        } else if (a == "--order-seed" && i + 1 < argc) {
            if (!parse_u64_arg(argv[++i], cli.order_seed)) return false;
        } else if (a == "--scheme" && i + 1 < argc) {
            cli.scheme = argv[++i];
        } else if (a == "--kernel" && i + 1 < argc) {
            cli.kernel = argv[++i];
        } else if (a == "--csv" && i + 1 < argc) {
            cli.csv_path = argv[++i];
        } else if (a == "--stages-csv" && i + 1 < argc) {
            cli.stages_csv_path = argv[++i];
        } else if (a == "--micro-csv" && i + 1 < argc) {
            cli.micro_csv_path = argv[++i];
        } else if (a == "--precision-samples" && i + 1 < argc) {
            if (!parse_int_arg(argv[++i], cli.precision_samples)) return false;
        } else if (a == "--precision-max-steps" && i + 1 < argc) {
            if (!parse_int_arg(argv[++i], cli.precision_max_steps)) return false;
        } else if (a == "--precision-bits" && i + 1 < argc) {
            if (!parse_int_arg(argv[++i], cli.precision_bits)) return false;
        } else if (a == "--precision-seed" && i + 1 < argc) {
            if (!parse_u64_arg(argv[++i], cli.precision_seed)) return false;
        } else if (a == "--precision-csv" && i + 1 < argc) {
            cli.precision_csv_path = argv[++i];
        } else if (a == "--input" && i + 1 < argc) {
            cli.input_path = argv[++i];
        } else {
            return false;
        }
    }
    return true;
}

chaosref::Image load_plain(const Cli& cli) {
    if (cli.input_path.empty()) {
        return chaosref::make_random_image(cli.size, cli.size, cli.channels, cli.image_seed);
    }
    cv::Mat mat = cv::imread(cli.input_path, cv::IMREAD_UNCHANGED);
    if (mat.empty()) throw std::runtime_error("failed to load input image: " + cli.input_path);
    if (mat.depth() != CV_8U) mat.convertTo(mat, CV_8U);
    if (!mat.isContinuous()) mat = mat.clone();
    chaosref::Image image;
    image.width = mat.cols;
    image.height = mat.rows;
    image.channels = mat.channels();
    image.data.assign(mat.data, mat.data + mat.total() * mat.elemSize());
    return image;
}

std::vector<BenchCase> build_cases(const Cli& cli) {
    std::vector<BenchCase> out;

    const bool use_map = (cli.scheme == "all" || cli.scheme == "map");
    const bool use_sort = (cli.scheme == "all" || cli.scheme == "sort");
    const bool use_bitplane = (cli.scheme == "all" || cli.scheme == "bitplane");
    const bool use_symbolic = (cli.scheme == "all" || cli.scheme == "symbolic");

    const bool use_scalar = (cli.kernel == "all" || cli.kernel == "scalar");
    const bool use_scan = (cli.kernel == "all" || cli.kernel == "scan");

    auto add_case = [&](const std::string& name, PermutationKind p, DiffusionKernel d,
                        uint64_t perm_seed, uint64_t diff_seed) {
        SchemeConfig cfg;
        cfg.perm_kind = p;
        cfg.diff_kernel = d;
        cfg.map_rounds = 2;
        cfg.perm_seed = perm_seed;
        cfg.diff_seed = diff_seed;
        out.push_back({name, cfg});
    };

    if (use_map && use_scalar)
        add_case("map_fridrich+scalar_chain", PermutationKind::MapFridrich, DiffusionKernel::ScalarChain,
                 0x12345678ABCDEF01ULL, 0xBADC0FFEE0DDF00DULL);
    if (use_map && use_scan)
        add_case("map_fridrich+scan_exact", PermutationKind::MapFridrich, DiffusionKernel::ScanExact,
                 0x12345678ABCDEF01ULL, 0xBADC0FFEE0DDF00DULL);
    if (use_sort && use_scalar)
        add_case("sort_logistic+scalar_chain", PermutationKind::SortLogistic, DiffusionKernel::ScalarChain,
                 0xCAFEBABE12345678ULL, 0x0D15EA5E600DF00DULL);
    if (use_sort && use_scan)
        add_case("sort_logistic+scan_exact", PermutationKind::SortLogistic, DiffusionKernel::ScanExact,
                 0xCAFEBABE12345678ULL, 0x0D15EA5E600DF00DULL);

    if (use_bitplane && use_scalar)
        add_case("bitplane+scalar_chain", PermutationKind::BitPlaneTranspose, DiffusionKernel::ScalarChain,
                 0x6B8B4567327B23C6ULL, 0x643C986966334873ULL);
    if (use_bitplane && use_scan)
        add_case("bitplane+scan_exact", PermutationKind::BitPlaneTranspose, DiffusionKernel::ScanExact,
                 0x6B8B4567327B23C6ULL, 0x643C986966334873ULL);
    if (use_symbolic && use_scalar)
        add_case("symbolic+scalar_chain", PermutationKind::SymbolicSubstitution, DiffusionKernel::ScalarChain,
                 0x74B0DC5119495CFFULL, 0x2AE8944A625558ECULL);
    if (use_symbolic && use_scan)
        add_case("symbolic+scan_exact", PermutationKind::SymbolicSubstitution, DiffusionKernel::ScanExact,
                 0x74B0DC5119495CFFULL, 0x2AE8944A625558ECULL);

    std::mt19937_64 order_rng(cli.order_seed);
    std::shuffle(out.begin(), out.end(), order_rng);

    return out;
}

void run_benchmark(const Cli& cli) {
    if (cli.size <= 0 || cli.channels <= 0 || cli.warmup < 0 || cli.iters <= 0) {
        std::cerr << "invalid numeric arguments\n";
        return;
    }

    std::vector<BenchCase> cases = build_cases(cli);
    if (cases.empty()) {
        std::cerr << "no benchmark cases selected (check --scheme / --kernel)\n";
        return;
    }

    chaosref::Image plain = load_plain(cli);
    const std::string image_id = cli.input_path.empty() ? "synthetic_seed_" + std::to_string(cli.image_seed)
                                                        : cli.input_path;

    std::filesystem::path csvp(cli.csv_path);
    if (!csvp.parent_path().empty()) {
        std::filesystem::create_directories(csvp.parent_path());
    }
    std::ofstream csv(cli.csv_path);
    csv << chaosref::csv_header();

    std::cout << "Benchmarking shared chaotic buckets\n";
    std::cout << "  SIMD backend: " << chaosref::simd_backend_name() << "\n";
    std::cout << "  image: " << chaosref::shape_string(plain) << "  bytes=" << plain.bytes() << "\n";
    std::cout << "  warmup=" << cli.warmup << " iters=" << cli.iters << "\n\n";

    std::cout << std::left
              << std::setw(34) << "case"
              << std::setw(14) << "enc MB/s"
              << std::setw(14) << "dec MB/s"
              << std::setw(12) << "enc ms"
              << std::setw(12) << "dec ms"
              << std::setw(10) << "correct"
              << "\n";
    std::cout << std::string(96, '-') << "\n";

    for (const auto& bc : cases) {
        chaosref::BenchResult r = chaosref::run_bench_case(plain, bc, cli.warmup, cli.iters, image_id);
        csv << chaosref::to_csv(r);

        std::cout << std::left
                  << std::setw(34) << r.name
                  << std::setw(14) << std::fixed << std::setprecision(1) << r.enc_mb_s
                  << std::setw(14) << std::fixed << std::setprecision(1) << r.dec_mb_s
                  << std::setw(12) << std::fixed << std::setprecision(3) << r.enc_ms
                  << std::setw(12) << std::fixed << std::setprecision(3) << r.dec_ms
                  << std::setw(10) << (r.correct ? "yes" : "NO")
                  << "\n";
    }

    std::cout << "\nCSV written: " << cli.csv_path << "\n";
}

void run_stage_benchmark(const Cli& cli) {
    if (cli.size <= 0 || cli.channels <= 0 || cli.warmup < 0 || cli.iters <= 0) {
        std::cerr << "invalid numeric arguments\n";
        return;
    }

    std::vector<BenchCase> cases = build_cases(cli);
    if (cases.empty()) {
        std::cerr << "no benchmark cases selected (check --scheme / --kernel)\n";
        return;
    }

    chaosref::Image plain = load_plain(cli);
    const std::string image_id = cli.input_path.empty() ? "synthetic_seed_" + std::to_string(cli.image_seed)
                                                        : cli.input_path;

    std::filesystem::path csvp(cli.stages_csv_path);
    if (!csvp.parent_path().empty()) {
        std::filesystem::create_directories(csvp.parent_path());
    }
    std::ofstream csv(cli.stages_csv_path);
    csv << chaosref::stage_csv_header();

    std::cout << "Benchmarking staged chaotic buckets\n";
    std::cout << "  SIMD backend: " << chaosref::simd_backend_name() << "\n";
    std::cout << "  image: " << chaosref::shape_string(plain) << "  bytes=" << plain.bytes() << "\n";
    std::cout << "  warmup=" << cli.warmup << " iters=" << cli.iters << "\n\n";

    for (const auto& bc : cases) {
        std::vector<chaosref::StageBenchResult> rows =
            chaosref::run_stage_bench_case(plain, bc, cli.warmup, cli.iters, image_id);
        for (const auto& r : rows) {
            csv << chaosref::to_csv(r);
        }
        std::cout << "  " << bc.name << ": " << rows.size() << " stage rows\n";
    }

    std::cout << "\nStage CSV written: " << cli.stages_csv_path << "\n";
}

void run_micro_benchmark(const Cli& cli) {
    if (cli.size <= 0 || cli.channels <= 0 || cli.warmup < 0 || cli.iters <= 0) {
        std::cerr << "invalid numeric arguments\n";
        return;
    }

    chaosref::Image plain = load_plain(cli);

    std::filesystem::path csvp(cli.micro_csv_path);
    if (!csvp.parent_path().empty()) {
        std::filesystem::create_directories(csvp.parent_path());
    }
    std::ofstream csv(cli.micro_csv_path);
    csv << chaosref::stage_csv_header();

    std::cout << "Benchmarking isolated SIMD-friendly kernels\n";
    std::cout << "  SIMD backend: " << chaosref::simd_backend_name() << "\n";
    std::cout << "  image: " << chaosref::shape_string(plain) << "  bytes=" << plain.bytes() << "\n";
    std::cout << "  warmup=" << cli.warmup << " iters=" << cli.iters << "\n\n";

    std::vector<chaosref::StageBenchResult> rows =
        chaosref::run_microbench_cases(plain, cli.warmup, cli.iters);
    for (const auto& r : rows) {
        csv << chaosref::to_csv(r);
    }

    std::cout << "  wrote " << rows.size() << " microbench rows\n";
    std::cout << "\nMicrobench CSV written: " << cli.micro_csv_path << "\n";
}

struct PrecisionOrbitRow {
    std::string family;
    uint64_t sample = 0;
    uint64_t seed = 0;
    int quant_bits = 0;
    int warmup = 0;
    int max_steps = 0;
    bool repeated = false;
    int first_repeat_step = -1;
    int cycle_length = -1;
    int distinct_states = 0;
    std::string note;
};

static uint64_t double_bits(double x) {
    uint64_t bits = 0;
    std::memcpy(&bits, &x, sizeof(bits));
    return bits;
}

static uint64_t quantize_unit_interval(double x, int bits) {
    if (bits <= 0) return 0;
    if (bits >= 63) return double_bits(x);
    const uint64_t levels = 1ULL << bits;
    double clamped = x;
    if (clamped < 0.0) clamped = 0.0;
    if (clamped >= 1.0) clamped = std::nextafter(1.0, 0.0);
    const uint64_t q = static_cast<uint64_t>(clamped * static_cast<double>(levels));
    return (q >= levels) ? (levels - 1ULL) : q;
}

static PrecisionOrbitRow probe_logistic_orbit(uint64_t seed, uint64_t sample, int warmup, int max_steps) {
    PrecisionOrbitRow row;
    row.family = "logistic_double";
    row.sample = sample;
    row.seed = seed;
    row.warmup = warmup;
    row.max_steps = max_steps;

    double x0 = 0.0, r = 0.0;
    chaosref::derive_logistic_params(seed, x0, r);
    chaosref::Logistic gen(x0, r, warmup);

    std::unordered_map<uint64_t, int> seen;
    seen.reserve(static_cast<size_t>(max_steps / 2 + 1));

    for (int step = 1; step <= max_steps; ++step) {
        const double x = gen.step();
        const uint64_t bits = double_bits(x);
        auto [it, inserted] = seen.emplace(bits, step);
        if (!inserted) {
            row.repeated = true;
            row.first_repeat_step = step;
            row.cycle_length = step - it->second;
            row.distinct_states = static_cast<int>(seen.size());
            row.note = "exact repeat detected";
            return row;
        }
    }

    row.distinct_states = static_cast<int>(seen.size());
    row.note = "no exact repeat within budget";
    return row;
}

static PrecisionOrbitRow probe_logistic_quantized_orbit(uint64_t seed, uint64_t sample, int warmup,
                                                        int max_steps, int quant_bits) {
    PrecisionOrbitRow row;
    row.family = "logistic_q" + std::to_string(quant_bits);
    row.sample = sample;
    row.seed = seed;
    row.quant_bits = quant_bits;
    row.warmup = warmup;
    row.max_steps = max_steps;

    double x0 = 0.0, r = 0.0;
    chaosref::derive_logistic_params(seed, x0, r);
    chaosref::Logistic gen(x0, r, warmup);

    std::unordered_map<uint64_t, int> seen;
    seen.reserve(static_cast<size_t>(max_steps / 2 + 1));

    for (int step = 1; step <= max_steps; ++step) {
        const double x = gen.step();
        const uint64_t q = quantize_unit_interval(x, quant_bits);
        auto [it, inserted] = seen.emplace(q, step);
        if (!inserted) {
            row.repeated = true;
            row.first_repeat_step = step;
            row.cycle_length = step - it->second;
            row.distinct_states = static_cast<int>(seen.size());
            row.note = "quantized repeat detected";
            return row;
        }
    }

    row.distinct_states = static_cast<int>(seen.size());
    row.note = "no quantized repeat within budget";
    return row;
}

static PrecisionOrbitRow probe_fixed_point_tent_orbit(uint64_t seed, uint64_t sample, int warmup, int max_steps) {
    PrecisionOrbitRow row;
    row.family = "fixed_point_tent";
    row.sample = sample;
    row.seed = seed;
    row.warmup = warmup;
    row.max_steps = max_steps;

    uint32_t x = static_cast<uint32_t>(seed ^ 0x54454E54ULL);
    if (x == 0) x = 1;
    for (int i = 0; i < warmup; ++i) {
        x = (x < 0x80000000U) ? (x << 1U) : ((~x) << 1U);
        x ^= x >> 13U;
    }

    std::unordered_map<uint32_t, int> seen;
    seen.reserve(static_cast<size_t>(max_steps / 2 + 1));

    for (int step = 1; step <= max_steps; ++step) {
        x = (x < 0x80000000U) ? (x << 1U) : ((~x) << 1U);
        x ^= x >> 13U;
        auto [it, inserted] = seen.emplace(x, step);
        if (!inserted) {
            row.repeated = true;
            row.first_repeat_step = step;
            row.cycle_length = step - it->second;
            row.distinct_states = static_cast<int>(seen.size());
            row.note = "exact repeat detected";
            return row;
        }
    }

    row.distinct_states = static_cast<int>(seen.size());
    row.note = "no exact repeat within budget";
    return row;
}

static void write_precision_csv(const std::string& path, const std::vector<PrecisionOrbitRow>& rows) {
    std::filesystem::path csvp(path);
    if (!csvp.parent_path().empty()) {
        std::filesystem::create_directories(csvp.parent_path());
    }
    std::ofstream csv(path);
    csv << "family,sample,seed,quant_bits,warmup,max_steps,repeated,first_repeat_step,cycle_length,distinct_states,note\n";
    for (const auto& r : rows) {
        csv << r.family << ','
            << r.sample << ','
            << r.seed << ','
            << r.quant_bits << ','
            << r.warmup << ','
            << r.max_steps << ','
            << (r.repeated ? 1 : 0) << ','
            << r.first_repeat_step << ','
            << r.cycle_length << ','
            << r.distinct_states << ','
            << r.note << '\n';
    }
}

static void run_precision_benchmark(const Cli& cli) {
    if (cli.precision_samples <= 0 || cli.precision_max_steps <= 0 || cli.warmup < 0) {
        std::cerr << "invalid precision arguments\n";
        return;
    }

    std::vector<PrecisionOrbitRow> rows;
    rows.reserve(static_cast<size_t>(cli.precision_samples) * 2);

    std::cout << "Finite-precision orbit probe\n";
    std::cout << "  warmup=" << cli.warmup
              << " samples=" << cli.precision_samples
              << " max_steps=" << cli.precision_max_steps << "\n\n";

    std::cout << std::left
              << std::setw(20) << "family"
              << std::setw(10) << "sample"
              << std::setw(10) << "repeat"
              << std::setw(12) << "qbits"
              << std::setw(14) << "first_step"
              << std::setw(14) << "cycle_len"
              << std::setw(16) << "distinct"
              << "note"
              << "\n";
    std::cout << std::string(90, '-') << "\n";

    uint64_t s = cli.precision_seed;
    for (int i = 0; i < cli.precision_samples; ++i) {
        const uint64_t sample_seed = chaosref::splitmix64(s);
        PrecisionOrbitRow logistic = probe_logistic_orbit(sample_seed, static_cast<uint64_t>(i), cli.warmup, cli.precision_max_steps);
        PrecisionOrbitRow logistic_q = probe_logistic_quantized_orbit(
            sample_seed, static_cast<uint64_t>(i), cli.warmup, cli.precision_max_steps, cli.precision_bits);
        PrecisionOrbitRow tent = probe_fixed_point_tent_orbit(sample_seed, static_cast<uint64_t>(i), cli.warmup, cli.precision_max_steps);
        rows.push_back(logistic);
        rows.push_back(logistic_q);
        rows.push_back(tent);

        const auto print_row = [&](const PrecisionOrbitRow& r) {
            std::cout << std::left
                      << std::setw(20) << r.family
                      << std::setw(10) << r.sample
                      << std::setw(10) << (r.repeated ? "yes" : "no")
                      << std::setw(12) << (r.quant_bits > 0 ? std::to_string(r.quant_bits) : "-")
                      << std::setw(14) << (r.first_repeat_step >= 0 ? std::to_string(r.first_repeat_step) : "n/a")
                      << std::setw(14) << (r.cycle_length >= 0 ? std::to_string(r.cycle_length) : "n/a")
                      << std::setw(16) << r.distinct_states
                      << r.note
                      << "\n";
        };
        print_row(logistic);
        print_row(logistic_q);
        print_row(tent);
    }

    write_precision_csv(cli.precision_csv_path, rows);
    std::cout << "\nPrecision CSV written: " << cli.precision_csv_path << "\n";
}

}  // namespace

int main(int argc, char** argv) {
    Cli cli;
    if (!parse_cli(argc, argv, cli)) {
        print_usage(argv[0]);
        return 1;
    }

    if (argc == 1) {
        print_usage(argv[0]);
        return 0;
    }

    if (cli.list) {
        print_list();
        return 0;
    }
    if (cli.buckets) {
        print_buckets();
        return 0;
    }
    if (cli.roadmap) {
        print_roadmap();
        return 0;
    }
    if (cli.bench) {
        run_benchmark(cli);
        return 0;
    }
    if (cli.stages) {
        run_stage_benchmark(cli);
        return 0;
    }
    if (cli.microbench) {
        run_micro_benchmark(cli);
        return 0;
    }
    if (cli.precision) {
        run_precision_benchmark(cli);
        return 0;
    }

    print_usage(argv[0]);
    return 0;
}
