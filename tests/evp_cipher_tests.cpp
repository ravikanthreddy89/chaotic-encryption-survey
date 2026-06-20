#include <cstdlib>
#include <iostream>
#include <stdexcept>

#include "crypto/EvpCiphers.hpp"

namespace {

bool equal_mat(const cv::Mat& a, const cv::Mat& b) {
    return a.size() == b.size() && a.type() == b.type() &&
           cv::countNonZero(a.reshape(1) != b.reshape(1)) == 0;
}

void require(bool condition, const char* message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << '\n';
        std::exit(1);
    }
}

}  // namespace

int main() {
    cv::Mat plain(17, 19, CV_8UC3);
    cv::randu(plain, 0, 256);
    bench::CipherContext context;
    context.key.resize(32, 0x42);
    context.nonce.resize(12, 0x17);

    for (auto& cipher : bench::make_evp_ciphers()) {
        const bench::CipherResult encrypted = cipher->encrypt(plain, context);
        const bench::CipherResult decrypted = cipher->decrypt_result(encrypted, context);
        require(equal_mat(plain, decrypted.image), "EVP round trip failed");

        if (!encrypted.authentication_tag.empty()) {
            bench::CipherResult tampered = encrypted;
            tampered.image.data[0] ^= 1U;
            bool rejected = false;
            try {
                (void)cipher->decrypt_result(tampered, context);
            } catch (const std::runtime_error&) {
                rejected = true;
            }
            require(rejected, "AEAD ciphertext tampering was accepted");
        }
    }
    std::cout << "EVP cipher tests passed\n";
    return 0;
}
