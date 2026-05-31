#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <memory>
#include <numeric>
#include <string>
#include <vector>

#include "core/IImageCipher.hpp"
#include "core/Utils.hpp"
#include "simd/avx2/Xor.hpp"
#include "blake3.h"

namespace bench {

inline uint64_t seed_from_key(const CipherContext& ctx, uint64_t salt) {
    uint64_t s = salt ^ 0x9E3779B97F4A7C15ULL;
    for (uint8_t b : ctx.key) {
        s ^= static_cast<uint64_t>(b) + 0x9E3779B97F4A7C15ULL + (s << 6) + (s >> 2);
    }
    return s;
}

inline uint64_t splitmix64_next(uint64_t& x) {
    uint64_t z = (x += 0x9E3779B97F4A7C15ULL);
    z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
    z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;
    return z ^ (z >> 31);
}

enum class ChaosFlavor {
    Logistic,
    Tent,
    Sine,
    CoupledLattice,
    HamiltonianLattice,
    Blake3Seed
};

inline std::vector<uint8_t> make_keystream(size_t n, const CipherContext& ctx, ChaosFlavor flavor) {
    std::vector<uint8_t> ks(n);
    uint64_t seed = seed_from_key(ctx, static_cast<uint64_t>(flavor) * 0x123456789ABCDEFULL);

    if (flavor == ChaosFlavor::Blake3Seed) {
        uint8_t blake_key[BLAKE3_KEY_LEN] = {};
        for (size_t i = 0; i < ctx.key.size(); ++i) {
            blake_key[i % BLAKE3_KEY_LEN] ^= ctx.key[i];
            blake_key[(i * 7U + 3U) % BLAKE3_KEY_LEN] ^= static_cast<uint8_t>(ctx.key[i] + i);
        }
        blake3_hasher h;
        blake3_hasher_init_keyed(&h, blake_key);
        static constexpr char label[] = "chaotic-image-encryption blake3 seeded keystream v1";
        blake3_hasher_update(&h, label, sizeof(label) - 1U);
        if (!ctx.nonce.empty()) blake3_hasher_update(&h, ctx.nonce.data(), ctx.nonce.size());
        blake3_hasher_finalize(&h, ks.data(), ks.size());
        return ks;
    }

    double x = 0.001 + (static_cast<double>(splitmix64_next(seed) & 0xFFFFFU) / 0xFFFFFU) * 0.998;
    double y = 0.113 + (static_cast<double>(splitmix64_next(seed) & 0xFFFFFU) / 0xFFFFFU) * 0.731;
    double z = 0.217 + (static_cast<double>(splitmix64_next(seed) & 0xFFFFFU) / 0xFFFFFU) * 0.619;

    for (int i = 0; i < 512; ++i) {
        x = 3.99 * x * (1.0 - x);
        y = std::sin(3.99 * y);
    }

    for (size_t i = 0; i < n; ++i) {
        switch (flavor) {
            case ChaosFlavor::Logistic:
                x = 3.99 * x * (1.0 - x);
                break;
            case ChaosFlavor::Tent:
                x = (x < 0.5) ? (1.999 * x) : (1.999 * (1.0 - x));
                break;
            case ChaosFlavor::Sine:
                x = std::sin(3.99 * x);
                x = std::fabs(x - std::floor(x));
                break;
            case ChaosFlavor::CoupledLattice: {
                const double nx = 3.91 * x * (1.0 - x);
                const double ny = 3.87 * y * (1.0 - y);
                x = std::fabs(nx + 0.07 * (ny - nx));
                y = std::fabs(ny + 0.07 * (nx - ny));
                x -= std::floor(x);
                y -= std::floor(y);
                break;
            }
            case ChaosFlavor::HamiltonianLattice: {
                const double nx = std::sin(3.7 * y) + 0.13 * z;
                const double ny = std::sin(3.9 * z) + 0.11 * x;
                const double nz = std::sin(4.1 * x) + 0.17 * y;
                x = std::fabs(nx - std::floor(nx));
                y = std::fabs(ny - std::floor(ny));
                z = std::fabs(nz - std::floor(nz));
                break;
            }
            case ChaosFlavor::Blake3Seed:
                break;
        }
        ks[i] = static_cast<uint8_t>(static_cast<int>(x * 256.0) & 0xFF);
    }
    return ks;
}

inline std::vector<uint32_t> logistic_permutation(size_t pixels, const CipherContext& ctx) {
    std::vector<double> scores(pixels);
    uint64_t seed = seed_from_key(ctx, 0xCAFEBABE12345678ULL);
    double x = 0.001 + (static_cast<double>(splitmix64_next(seed) & 0xFFFFFU) / 0xFFFFFU) * 0.998;
    for (double& v : scores) {
        x = 3.99 * x * (1.0 - x);
        v = x;
    }
    std::vector<uint32_t> idx(pixels);
    std::iota(idx.begin(), idx.end(), 0U);
    std::stable_sort(idx.begin(), idx.end(), [&](uint32_t a, uint32_t b) { return scores[a] < scores[b]; });
    return idx;
}

inline cv::Mat apply_pixel_permutation(const cv::Mat& src, const std::vector<uint32_t>& idx) {
    cv::Mat out(src.rows, src.cols, src.type());
    const int channels = src.channels();
    const uint8_t* in = src.data;
    uint8_t* dst = out.data;
    for (size_t i = 0; i < idx.size(); ++i) {
        std::memcpy(dst + static_cast<size_t>(idx[i]) * channels, in + i * channels, channels);
    }
    return out;
}

inline cv::Mat invert_pixel_permutation(const cv::Mat& src, const std::vector<uint32_t>& idx) {
    cv::Mat out(src.rows, src.cols, src.type());
    const int channels = src.channels();
    const uint8_t* in = src.data;
    uint8_t* dst = out.data;
    for (size_t i = 0; i < idx.size(); ++i) {
        std::memcpy(dst + i * channels, in + static_cast<size_t>(idx[i]) * channels, channels);
    }
    return out;
}

inline cv::Mat arnold_map(const cv::Mat& src, bool inverse, int tile = 0) {
    cv::Mat out(src.rows, src.cols, src.type());
    const int channels = src.channels();
    const int block = tile > 0 ? tile : std::min(src.rows, src.cols);
    for (int by = 0; by < src.rows; by += block) {
        for (int bx = 0; bx < src.cols; bx += block) {
            const int h = std::min(block, src.rows - by);
            const int w = std::min(block, src.cols - bx);
            const int n = std::min(h, w);
            for (int y = 0; y < h; ++y) {
                for (int x = 0; x < w; ++x) {
                    int nx = x, ny = y;
                    if (x < n && y < n) {
                        if (!inverse) {
                            nx = (x + y) % n;
                            ny = (x + 2 * y) % n;
                        } else {
                            nx = (2 * x - y + 2 * n) % n;
                            ny = (-x + y + 2 * n) % n;
                        }
                    }
                    const uint8_t* s = src.ptr<uint8_t>(by + y) + (bx + x) * channels;
                    uint8_t* d = out.ptr<uint8_t>(by + ny) + (bx + nx) * channels;
                    std::memcpy(d, s, channels);
                }
            }
        }
    }
    return out;
}

class XorChaosCipher final : public IImageCipher {
public:
    XorChaosCipher(std::string name, ChaosFlavor flavor, bool logistic_perm = false,
                   bool arnold = false, bool tiled = false)
        : name_(std::move(name)), flavor_(flavor), logistic_perm_(logistic_perm), arnold_(arnold), tiled_(tiled) {}

    std::string name() const override { return name_; }
    std::string variant() const override { return "scalar_avx2"; }

    CipherResult encrypt(const cv::Mat& plain, const CipherContext& ctx) override {
        Timer total;
        CipherResult result;
        cv::Mat work = make_contiguous(plain).clone();

        Timer perm_timer;
        if (logistic_perm_) {
            auto idx = logistic_permutation(plain.total(), ctx);
            work = apply_pixel_permutation(work, idx);
        } else if (arnold_) {
            work = arnold_map(work, false, tiled_ ? 32 : 0);
        }
        result.times.permutation_ms = perm_timer.elapsed_ms();

        Timer key_timer;
        std::vector<uint8_t> ks = make_keystream(image_bytes(work), ctx, flavor_);
        result.times.keygen_ms = key_timer.elapsed_ms();

        Timer diff_timer;
        std::vector<uint8_t> input = mat_to_bytes(work);
        std::vector<uint8_t> output;
        xor_bytes(input, ks, output, ctx.use_avx2);
        result.times.diffusion_ms = diff_timer.elapsed_ms();
        result.image = bytes_to_mat(output, work.rows, work.cols, work.type());
        result.times.total_ms = total.elapsed_ms();
        return result;
    }

    CipherResult decrypt(const cv::Mat& cipher, const CipherContext& ctx) override {
        Timer total;
        CipherResult result;

        Timer key_timer;
        std::vector<uint8_t> ks = make_keystream(image_bytes(cipher), ctx, flavor_);
        result.times.keygen_ms = key_timer.elapsed_ms();

        Timer diff_timer;
        std::vector<uint8_t> input = mat_to_bytes(cipher);
        std::vector<uint8_t> output;
        xor_bytes(input, ks, output, ctx.use_avx2);
        cv::Mat work = bytes_to_mat(output, cipher.rows, cipher.cols, cipher.type());
        result.times.diffusion_ms = diff_timer.elapsed_ms();

        Timer perm_timer;
        if (logistic_perm_) {
            auto idx = logistic_permutation(cipher.total(), ctx);
            work = invert_pixel_permutation(work, idx);
        } else if (arnold_) {
            work = arnold_map(work, true, tiled_ ? 32 : 0);
        }
        result.times.permutation_ms = perm_timer.elapsed_ms();
        result.image = work;
        result.times.total_ms = total.elapsed_ms();
        return result;
    }

private:
    std::string name_;
    ChaosFlavor flavor_;
    bool logistic_perm_ = false;
    bool arnold_ = false;
    bool tiled_ = false;
};

inline std::vector<CipherPtr> make_chaotic_ciphers() {
    std::vector<CipherPtr> c;
    c.push_back(std::make_unique<XorChaosCipher>("logistic_xor", ChaosFlavor::Logistic));
    c.push_back(std::make_unique<XorChaosCipher>("logistic_permute_xor", ChaosFlavor::Logistic, true));
    c.push_back(std::make_unique<XorChaosCipher>("arnold_xor", ChaosFlavor::Logistic, false, true));
    c.push_back(std::make_unique<XorChaosCipher>("tiled_arnold_xor", ChaosFlavor::Logistic, false, true, true));
    c.push_back(std::make_unique<XorChaosCipher>("tent_block_xor", ChaosFlavor::Tent));
    c.push_back(std::make_unique<XorChaosCipher>("sine_xor", ChaosFlavor::Sine));
    c.push_back(std::make_unique<XorChaosCipher>("coupled_lattice_xor", ChaosFlavor::CoupledLattice));
    c.push_back(std::make_unique<XorChaosCipher>("hamiltonian_lattice_xor", ChaosFlavor::HamiltonianLattice));
    c.push_back(std::make_unique<XorChaosCipher>("chaotic_seed_blake3_xor", ChaosFlavor::Blake3Seed));
    return c;
}

}  // namespace bench
