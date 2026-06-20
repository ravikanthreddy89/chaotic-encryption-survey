#include <array>
#include <cstdlib>
#include <iostream>

#include "chaosref/schemes.hpp"

namespace {

void require(bool condition, const char* message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << '\n';
        std::exit(1);
    }
}

}  // namespace

int main() {
    const chaosref::Image plain = chaosref::make_random_image(32, 32, 3, 42);
    const std::array<chaosref::PermutationKind, 4> families = {
        chaosref::PermutationKind::MapFridrich,
        chaosref::PermutationKind::SortLogistic,
        chaosref::PermutationKind::BitPlaneTranspose,
        chaosref::PermutationKind::SymbolicSubstitution,
    };

    for (const auto family : families) {
        chaosref::SchemeConfig scalar;
        scalar.perm_kind = family;
        scalar.diff_kernel = chaosref::DiffusionKernel::ScalarChain;
        scalar.perm_seed = 0x123456789ABCDEF0ULL;
        scalar.diff_seed = 0x0FEDCBA987654321ULL;

        chaosref::SchemeConfig scan = scalar;
        scan.diff_kernel = chaosref::DiffusionKernel::ScanExact;

        const chaosref::Image scalar_cipher = chaosref::encrypt_image(plain, scalar);
        const chaosref::Image scan_cipher = chaosref::encrypt_image(plain, scan);
        require(chaosref::equal_image(scalar_cipher, scan_cipher),
                "scalar and exact-scan ciphertext differ");
        require(chaosref::equal_image(plain, chaosref::decrypt_image(scalar_cipher, scalar)),
                "scalar round trip failed");
        require(chaosref::equal_image(plain, chaosref::decrypt_image(scan_cipher, scan)),
                "scan round trip failed");
    }

    const chaosref::Image odd = chaosref::make_random_image(7, 5, 3, 7);
    chaosref::SchemeConfig rectangular_map;
    rectangular_map.perm_kind = chaosref::PermutationKind::MapFridrich;
    rectangular_map.diff_kernel = chaosref::DiffusionKernel::ScanExact;
    const auto rectangular_cipher = chaosref::encrypt_image(odd, rectangular_map);
    require(chaosref::equal_image(odd, chaosref::decrypt_image(rectangular_cipher, rectangular_map)),
            "map round trip failed on rectangular dimensions");
    require(chaosref::equal_image(odd, chaosref::permute_bitplanes(chaosref::permute_bitplanes(odd))),
            "bit-plane transpose is not self-inverse on a tail block");
    const auto symbolic = chaosref::permute_symbolic(odd, 99);
    require(chaosref::equal_image(odd, chaosref::permute_symbolic(symbolic, 99, true)),
            "symbolic substitution inverse failed");

    std::cout << "reference tests passed (backend=" << chaosref::simd_backend_name() << ")\n";
    return 0;
}
