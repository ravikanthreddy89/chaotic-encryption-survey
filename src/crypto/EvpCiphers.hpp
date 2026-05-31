#pragma once

#include <openssl/err.h>
#include <openssl/evp.h>

#include <stdexcept>
#include <string>
#include <vector>

#include "core/IImageCipher.hpp"
#include "core/Utils.hpp"

namespace bench {

inline void evp_check(int rc, const char* what) {
    if (rc != 1) {
        char buf[256];
        ERR_error_string_n(ERR_get_error(), buf, sizeof(buf));
        throw std::runtime_error(std::string(what) + ": " + buf);
    }
}

class EvpStreamCipher final : public IImageCipher {
public:
    EvpStreamCipher(std::string name, const EVP_CIPHER* (*cipher_fn)())
        : name_(std::move(name)), cipher_fn_(cipher_fn) {}

    std::string name() const override { return name_; }
    std::string variant() const override { return "openssl_evp"; }

    CipherResult encrypt(const cv::Mat& plain, const CipherContext& ctx) override {
        return crypt(plain, ctx);
    }

    CipherResult decrypt(const cv::Mat& cipher, const CipherContext& ctx) override {
        return crypt(cipher, ctx);
    }

private:
    CipherResult crypt(const cv::Mat& input, const CipherContext& ctx) {
        Timer total;
        CipherResult result;
        std::vector<uint8_t> in = mat_to_bytes(input);
        std::vector<uint8_t> out(in.size() + EVP_MAX_BLOCK_LENGTH);

        std::vector<uint8_t> key = ctx.key;
        key.resize(32, 0);
        std::vector<uint8_t> iv = ctx.nonce;
        iv.resize(16, 0);

        Timer evp_timer;
        EVP_CIPHER_CTX* c = EVP_CIPHER_CTX_new();
        if (!c) throw std::bad_alloc();
        evp_check(EVP_EncryptInit_ex(c, cipher_fn_(), nullptr, key.data(), iv.data()), "EVP init");
        int out_len = 0;
        evp_check(EVP_EncryptUpdate(c, out.data(), &out_len, in.data(), static_cast<int>(in.size())), "EVP update");
        int final_len = 0;
        evp_check(EVP_EncryptFinal_ex(c, out.data() + out_len, &final_len), "EVP final");
        EVP_CIPHER_CTX_free(c);
        out.resize(static_cast<size_t>(out_len + final_len));
        result.times.diffusion_ms = evp_timer.elapsed_ms();

        result.image = bytes_to_mat(out, input.rows, input.cols, input.type());
        result.times.total_ms = total.elapsed_ms();
        return result;
    }

    std::string name_;
    const EVP_CIPHER* (*cipher_fn_)();
};

inline std::vector<CipherPtr> make_evp_ciphers() {
    std::vector<CipherPtr> c;
    c.push_back(std::make_unique<EvpStreamCipher>("aes_ctr", EVP_aes_256_ctr));
    c.push_back(std::make_unique<EvpStreamCipher>("chacha20", EVP_chacha20));
    return c;
}

}  // namespace bench
