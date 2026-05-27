#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

#include "chaosref/bench.hpp"
#include "chaosref/catalog.hpp"

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

    int size = 1024;
    int channels = 3;
    int warmup = 2;
    int iters = 6;
    uint64_t image_seed = 42;

    std::string scheme = "all";    // all | map | sort
    std::string kernel = "all";    // all | scalar | scan
    std::string csv_path = "results/bench_ref.csv";
};

void print_usage(const char* argv0) {
    std::cout
        << "chaos_ref usage\n"
        << "  " << argv0 << " --list\n"
        << "  " << argv0 << " --buckets\n"
        << "  " << argv0 << " --roadmap\n"
        << "  " << argv0 << " --bench [--size N] [--channels C] [--warmup W] [--iters I]\\n"
        << "       [--scheme all|map|sort] [--kernel all|scalar|scan] [--csv PATH]\n";
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
        } else if (a == "--scheme" && i + 1 < argc) {
            cli.scheme = argv[++i];
        } else if (a == "--kernel" && i + 1 < argc) {
            cli.kernel = argv[++i];
        } else if (a == "--csv" && i + 1 < argc) {
            cli.csv_path = argv[++i];
        } else {
            return false;
        }
    }
    return true;
}

std::vector<BenchCase> build_cases(const Cli& cli) {
    std::vector<BenchCase> out;

    const bool use_map = (cli.scheme == "all" || cli.scheme == "map");
    const bool use_sort = (cli.scheme == "all" || cli.scheme == "sort");

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

    chaosref::Image plain = chaosref::make_random_image(cli.size, cli.size, cli.channels, cli.image_seed);

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
        chaosref::BenchResult r = chaosref::run_bench_case(plain, bc, cli.warmup, cli.iters);
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

    print_usage(argv[0]);
    return 0;
}
