#pragma once

#include <string>
#include <vector>

namespace chaosref {

struct SchemeInfo {
    std::string name;
    std::string year;
    std::string family;
    std::string permutation;
    std::string diffusion;
    std::string keygen;
    std::string simd_feasibility;
};

inline const std::vector<SchemeInfo>& scheme_catalog() {
    static const std::vector<SchemeInfo> kSchemes = {
        {
            "Fridrich-1998",
            "1998",
            "Permutation-Diffusion (baseline)",
            "Iterative 2D map permutation",
            "CBC-like chained XOR",
            "Static key -> map seeds",
            "Permutation: limited SIMD, Diffusion: prefix-XOR vectorizable"
        },
        {
            "Arnold/Tent/Logistic bit-level family",
            "2010+",
            "Bit-plane permutation-diffusion",
            "Arnold cat map / bit-plane shuffles",
            "XOR/add modular diffusion",
            "Chaotic seeds from key",
            "Bitwise phases are SIMD-friendly; recurrences need scan"
        },
        {
            "YeHuang-style sorted-sequence",
            "2018",
            "Sort-based chaos permutation",
            "Sort indices by chaotic sequence",
            "RGB chained XOR diffusion",
            "SHA-derived parameters",
            "Sort is memory-bound; diffusion SIMD/scan-friendly"
        },
        {
            "LSCM-style sorted-sequence",
            "2020",
            "Sort-based chaos permutation",
            "Sort indices by chaotic sequence",
            "Single-chain XOR diffusion",
            "Seed pair + map control",
            "Sort dominates; diffusion SIMD/scan-friendly"
        },
        {
            "DNA + hyperchaotic families",
            "2020+",
            "Encoding-enhanced chaos",
            "Pixel/bit-plane permutation + DNA rules",
            "DNA ops + chained diffusion",
            "Plaintext-related parameterization",
            "Byte/bit kernels SIMD-friendly; rule switches hurt vector width"
        },
        {
            "Parallel bit-plane ICCM family",
            "2025",
            "Parallel cross bit-plane",
            "Cross-plane permutation",
            "Parallel multi-stage diffusion",
            "Higher-dimensional chaotic seeds",
            "Good SIMD/OpenMP/GPU candidate"
        },
        {
            "4D chaos + cellular automata",
            "2025",
            "Hybrid symbolic-chaotic",
            "Pixel permutation + symbolic remap",
            "Decimal + symbolic diffusion",
            "4D chaos parameter generation",
            "Symbolic stages branchy; numeric stages SIMD-friendly"
        }
    };
    return kSchemes;
}

}  // namespace chaosref
