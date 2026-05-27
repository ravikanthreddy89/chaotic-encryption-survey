#pragma once
/**
 * cipher_chacha.h
 * ─────────────────────────────────────────────────────────────────────────────
 * ChaCha20-Poly1305 image encryption via libsodium.
 *
 * We use crypto_aead_chacha20poly1305_ietf_* (RFC 7539 / IETF variant):
 *   - 256-bit key
 *   - 96-bit nonce (12 bytes)
 *   - 128-bit Poly1305 authentication tag
 *
 * libsodium auto-selects the fastest available implementation:
 *   AVX2 / NEON / generic C — transparent to the caller.
 *
 * ChaCha20 is a software-friendly stream cipher designed by D.J. Bernstein.
 * It performs especially well on CPUs without AES-NI.  On our test CPU (which
 * HAS AES-NI + AVX512) AES-CTR will likely be faster; ChaCha20 is included as
 * the modern software-baseline comparison point.
 *
 * LOC: ~30 lines of cipher logic (libsodium is very high-level)
 */

#include <sodium.h>
#include <vector>
#include <cstring>
#include <stdexcept>
#include <opencv2/opencv.hpp>
#include "bench_common.h"

namespace Ciphers {

struct ChaCha20Poly1305 {
    static constexpr const char* NAME = "ChaCha20-Poly1305";
    static constexpr int LOC = 28;

    static constexpr size_t KEY_LEN   = crypto_aead_chacha20poly1305_ietf_KEYBYTES;   // 32
    static constexpr size_t NONCE_LEN = crypto_aead_chacha20poly1305_ietf_NPUBBYTES;  // 12
    static constexpr size_t TAG_LEN   = crypto_aead_chacha20poly1305_ietf_ABYTES;     // 16

    // ── Init libsodium once ─────────────────────────────────────────────────
    static void init() {
        static bool done = false;
        if (!done) {
            if (sodium_init() < 0)
                throw std::runtime_error("libsodium init failed");
            done = true;
        }
    }

    // Returns ciphertext || tag  (plaintext_len + TAG_LEN bytes)
    static std::vector<uint8_t> encrypt(
        const uint8_t* pt, size_t pt_len,
        const uint8_t key[32], const uint8_t nonce[12],
        const uint8_t* ad = nullptr, size_t ad_len = 0)
    {
        init();
        std::vector<uint8_t> out(pt_len + TAG_LEN);
        unsigned long long out_len = 0;

        int rc = crypto_aead_chacha20poly1305_ietf_encrypt(
            out.data(), &out_len,
            pt, pt_len,
            ad, ad_len,
            nullptr,   // nsec (unused)
            nonce, key);

        if (rc != 0) throw std::runtime_error("ChaCha20-Poly1305 encrypt failed");
        out.resize(out_len);
        return out;
    }

    // Returns plaintext; throws on authentication failure
    static std::vector<uint8_t> decrypt(
        const uint8_t* ct_tag, size_t ct_len,
        const uint8_t key[32], const uint8_t nonce[12],
        const uint8_t* ad = nullptr, size_t ad_len = 0)
    {
        init();
        if (ct_len < TAG_LEN) throw std::runtime_error("ChaCha20: ciphertext too short");
        std::vector<uint8_t> pt(ct_len - TAG_LEN);
        unsigned long long pt_len = 0;

        int rc = crypto_aead_chacha20poly1305_ietf_decrypt(
            pt.data(), &pt_len,
            nullptr,   // nsec
            ct_tag, ct_len,
            ad, ad_len,
            nonce, key);

        if (rc != 0) throw std::runtime_error("ChaCha20-Poly1305 authentication FAILED");
        pt.resize(pt_len);
        return pt;
    }

    // ── Image wrappers ──────────────────────────────────────────────────────
    static cv::Mat encrypt_image(const cv::Mat& src) {
        size_t len = (size_t)src.rows * src.cols * src.channels();
        // AAD = image dimensions (4+4 bytes)
        uint8_t aad[8];
        int w = src.cols, h = src.rows;
        std::memcpy(aad,   &w, 4);
        std::memcpy(aad+4, &h, 4);

        auto ct = encrypt(src.data, len,
                          Bench::FIXED_KEY.data(),
                          Bench::FIXED_NONCE12.data(),
                          aad, sizeof(aad));

        // The Mat stores only the ciphertext portion (tag stored separately
        // in a real system; here we strip it for the image-metric pass)
        cv::Mat out(src.rows, src.cols, src.type());
        std::memcpy(out.data, ct.data(), len);
        return out;
    }

    // Full encrypt+decrypt roundtrip for decryption benchmark
    static cv::Mat decrypt_image(const cv::Mat& src_plain, const cv::Mat& src_cipher) {
        size_t len = (size_t)src_plain.rows * src_plain.cols * src_plain.channels();
        uint8_t aad[8];
        int w = src_plain.cols, h = src_plain.rows;
        std::memcpy(aad,   &w, 4);
        std::memcpy(aad+4, &h, 4);

        // Encrypt the original plaintext to get authenticated ciphertext+tag
        auto ct = encrypt(src_plain.data, len,
                          Bench::FIXED_KEY.data(),
                          Bench::FIXED_NONCE12.data(),
                          aad, sizeof(aad));

        // Decrypt it
        auto pt = decrypt(ct.data(), ct.size(),
                          Bench::FIXED_KEY.data(),
                          Bench::FIXED_NONCE12.data(),
                          aad, sizeof(aad));

        cv::Mat out(src_plain.rows, src_plain.cols, src_plain.type());
        std::memcpy(out.data, pt.data(), std::min(pt.size(), len));
        return out;
    }
};

} // namespace Ciphers
