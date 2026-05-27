#pragma once

#include <cmath>
#include <cstdint>
#include <cstring>
#include <vector>

namespace chaosref {

inline uint64_t splitmix64(uint64_t& x) {
    uint64_t z = (x += 0x9E3779B97F4A7C15ULL);
    z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
    z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;
    return z ^ (z >> 31);
}

inline void derive_logistic_params(uint64_t key, double& x0, double& r) {
    uint64_t s = key;
    uint64_t a = splitmix64(s);
    uint64_t b = splitmix64(s);

    const double u = static_cast<double>(a & 0x000FFFFFFFFFFFFFULL) /
                     static_cast<double>(0x000FFFFFFFFFFFFFULL);
    const double v = static_cast<double>(b & 0x00FFFFFFULL) /
                     static_cast<double>(0x00FFFFFFULL);

    x0 = 0.001 + 0.998 * u;
    r = 3.9 + 0.1 * v;
}

struct Logistic {
    double x = 0.5;
    double r = 3.99;

    Logistic(double x0, double rr, int warmup = 500) : x(x0), r(rr) {
        for (int i = 0; i < warmup; ++i) step();
    }

    inline double step() {
        x = r * x * (1.0 - x);
        if (x <= 0.0) x = 1e-12;
        if (x >= 1.0) x = 1.0 - 1e-12;
        return x;
    }

    inline uint8_t byte() {
        return static_cast<uint8_t>(static_cast<int>(step() * 256.0) & 0xFF);
    }
};

inline void fill_logistic_bytes(std::vector<uint8_t>& out, uint64_t key_seed) {
    double x0 = 0.0, r = 0.0;
    derive_logistic_params(key_seed, x0, r);
    Logistic gen(x0, r);
    for (auto& b : out) b = gen.byte();
}

inline void fill_logistic_scores(std::vector<double>& out, uint64_t key_seed) {
    double x0 = 0.0, r = 0.0;
    derive_logistic_params(key_seed, x0, r);
    Logistic gen(x0, r);
    for (auto& v : out) v = gen.step();
}

}  // namespace chaosref
