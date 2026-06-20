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

class EvpAeadCipher final : public IImageCipher {
public:
    EvpAeadCipher(std::string name, const EVP_CIPHER* (*cipher_fn)())
        : name_(std::move(name)), cipher_fn_(cipher_fn) {}

    std::string name() const override { return name_; }
    std::string variant() const override { return "openssl_evp_aead"; }

    CipherResult encrypt(const cv::Mat& plain, const CipherContext& ctx) override {
        Timer total;
        CipherResult result;
        const std::vector<uint8_t> input = mat_to_bytes(plain);
        std::vector<uint8_t> output(input.size() + EVP_MAX_BLOCK_LENGTH);
        std::vector<uint8_t> key = ctx.key;
        key.resize(32, 0);
        std::vector<uint8_t> nonce = ctx.nonce;
        nonce.resize(12, 0);

        Timer evp_timer;
        EVP_CIPHER_CTX* raw = EVP_CIPHER_CTX_new();
        if (!raw) throw std::bad_alloc();
        std::unique_ptr<EVP_CIPHER_CTX, decltype(&EVP_CIPHER_CTX_free)> cipher(raw, EVP_CIPHER_CTX_free);
        evp_check(EVP_EncryptInit_ex(cipher.get(), cipher_fn_(), nullptr, nullptr, nullptr), "AEAD init");
        evp_check(EVP_CIPHER_CTX_ctrl(cipher.get(), EVP_CTRL_AEAD_SET_IVLEN,
                                      static_cast<int>(nonce.size()), nullptr), "AEAD IV length");
        evp_check(EVP_EncryptInit_ex(cipher.get(), nullptr, nullptr, key.data(), nonce.data()), "AEAD key/IV");
        int written = 0;
        evp_check(EVP_EncryptUpdate(cipher.get(), output.data(), &written, input.data(),
                                    static_cast<int>(input.size())), "AEAD encrypt");
        int final_written = 0;
        evp_check(EVP_EncryptFinal_ex(cipher.get(), output.data() + written, &final_written), "AEAD final");
        output.resize(static_cast<size_t>(written + final_written));
        result.authentication_tag.resize(16);
        evp_check(EVP_CIPHER_CTX_ctrl(cipher.get(), EVP_CTRL_AEAD_GET_TAG,
                                      static_cast<int>(result.authentication_tag.size()),
                                      result.authentication_tag.data()), "AEAD get tag");
        result.times.diffusion_ms = evp_timer.elapsed_ms();
        result.image = bytes_to_mat(output, plain.rows, plain.cols, plain.type());
        result.times.total_ms = total.elapsed_ms();
        return result;
    }

    CipherResult decrypt(const cv::Mat&, const CipherContext&) override {
        throw std::invalid_argument("authenticated decryption requires the ciphertext tag");
    }

    CipherResult decrypt_result(const CipherResult& encrypted, const CipherContext& ctx) override {
        if (encrypted.authentication_tag.size() != 16) {
            throw std::invalid_argument("missing or invalid authentication tag");
        }
        Timer total;
        CipherResult result;
        const std::vector<uint8_t> input = mat_to_bytes(encrypted.image);
        std::vector<uint8_t> output(input.size() + EVP_MAX_BLOCK_LENGTH);
        std::vector<uint8_t> key = ctx.key;
        key.resize(32, 0);
        std::vector<uint8_t> nonce = ctx.nonce;
        nonce.resize(12, 0);

        Timer evp_timer;
        EVP_CIPHER_CTX* raw = EVP_CIPHER_CTX_new();
        if (!raw) throw std::bad_alloc();
        std::unique_ptr<EVP_CIPHER_CTX, decltype(&EVP_CIPHER_CTX_free)> cipher(raw, EVP_CIPHER_CTX_free);
        evp_check(EVP_DecryptInit_ex(cipher.get(), cipher_fn_(), nullptr, nullptr, nullptr), "AEAD decrypt init");
        evp_check(EVP_CIPHER_CTX_ctrl(cipher.get(), EVP_CTRL_AEAD_SET_IVLEN,
                                      static_cast<int>(nonce.size()), nullptr), "AEAD decrypt IV length");
        evp_check(EVP_DecryptInit_ex(cipher.get(), nullptr, nullptr, key.data(), nonce.data()),
                  "AEAD decrypt key/IV");
        int written = 0;
        evp_check(EVP_DecryptUpdate(cipher.get(), output.data(), &written, input.data(),
                                    static_cast<int>(input.size())), "AEAD decrypt");
        std::vector<uint8_t> tag = encrypted.authentication_tag;
        evp_check(EVP_CIPHER_CTX_ctrl(cipher.get(), EVP_CTRL_AEAD_SET_TAG,
                                      static_cast<int>(tag.size()), tag.data()), "AEAD set tag");
        int final_written = 0;
        if (EVP_DecryptFinal_ex(cipher.get(), output.data() + written, &final_written) != 1) {
            throw std::runtime_error("AEAD authentication failed");
        }
        output.resize(static_cast<size_t>(written + final_written));
        result.times.diffusion_ms = evp_timer.elapsed_ms();
        result.image = bytes_to_mat(output, encrypted.image.rows, encrypted.image.cols, encrypted.image.type());
        result.times.total_ms = total.elapsed_ms();
        return result;
    }

private:
    std::string name_;
    const EVP_CIPHER* (*cipher_fn_)();
};

inline std::vector<CipherPtr> make_evp_ciphers() {
    std::vector<CipherPtr> c;
    c.push_back(std::make_unique<EvpStreamCipher>("aes_ctr", EVP_aes_256_ctr));
    c.push_back(std::make_unique<EvpStreamCipher>("chacha20", EVP_chacha20));
    c.push_back(std::make_unique<EvpAeadCipher>("aes_256_gcm", EVP_aes_256_gcm));
    c.push_back(std::make_unique<EvpAeadCipher>("chacha20_poly1305", EVP_chacha20_poly1305));
    return c;
}

}  // namespace bench
