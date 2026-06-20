#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "chaosref/chaotic_rng.hpp"
#include "chaosref/diffusion.hpp"
#include "chaosref/permutation.hpp"
#include "chaosref/types.hpp"

namespace chaosref {

enum class PermutationKind {
    MapFridrich,
    SortLogistic,
    BitPlaneTranspose,
    SymbolicSubstitution
};

inline const char* permutation_kind_name(PermutationKind k) {
    switch (k) {
        case PermutationKind::MapFridrich: return "map_fridrich";
        case PermutationKind::SortLogistic: return "sort_logistic";
        case PermutationKind::BitPlaneTranspose: return "bitplane_transpose";
        case PermutationKind::SymbolicSubstitution: return "symbolic_substitution";
    }
    return "unknown";
}

struct SchemeConfig {
    PermutationKind perm_kind = PermutationKind::MapFridrich;
    DiffusionKernel diff_kernel = DiffusionKernel::ScalarChain;
    int map_rounds = 2;
    uint64_t perm_seed = 0x12345678ABCDEF01ULL;
    uint64_t diff_seed = 0xBADC0FFEE0DDF00DULL;
};

inline Image encrypt_image(const Image& plain, const SchemeConfig& cfg) {
    Image permuted;
    std::vector<uint32_t> idx;

    if (cfg.perm_kind == PermutationKind::MapFridrich) {
        permuted = permute_map_fridrich(plain, cfg.map_rounds);
    } else if (cfg.perm_kind == PermutationKind::SortLogistic) {
        idx = build_sort_permutation(static_cast<size_t>(plain.pixels()), cfg.perm_seed);
        permuted = permute_sort(plain, idx);
    } else if (cfg.perm_kind == PermutationKind::BitPlaneTranspose) {
        permuted = permute_bitplanes(plain);
    } else {
        permuted = permute_symbolic(plain, cfg.perm_seed);
    }

    Image cipher = permuted;
    std::vector<uint8_t> ks(cipher.bytes());
    fill_logistic_bytes(ks, cfg.diff_seed);
    diffuse(permuted.data.data(), ks.data(), cipher.data.data(), cipher.bytes(), cfg.diff_kernel, 0);
    return cipher;
}

inline Image decrypt_image(const Image& cipher, const SchemeConfig& cfg) {
    Image undiff = cipher;
    std::vector<uint8_t> ks(cipher.bytes());
    fill_logistic_bytes(ks, cfg.diff_seed);
    undiffuse(cipher.data.data(), ks.data(), undiff.data.data(), undiff.bytes(), cfg.diff_kernel, 0);

    if (cfg.perm_kind == PermutationKind::MapFridrich) {
        return invert_permute_map_fridrich(undiff, cfg.map_rounds);
    }
    if (cfg.perm_kind == PermutationKind::SortLogistic) {
        std::vector<uint32_t> idx = build_sort_permutation(static_cast<size_t>(cipher.pixels()), cfg.perm_seed);
        return invert_permute_sort(undiff, idx);
    }
    if (cfg.perm_kind == PermutationKind::BitPlaneTranspose) return permute_bitplanes(undiff);
    return permute_symbolic(undiff, cfg.perm_seed, true);
}

}  // namespace chaosref
