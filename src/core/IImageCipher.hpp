#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include <opencv2/opencv.hpp>

namespace bench {

struct StageTimes {
    double keygen_ms = 0.0;
    double permutation_ms = 0.0;
    double diffusion_ms = 0.0;
    double total_ms = 0.0;
};

struct CipherResult {
    cv::Mat image;
    StageTimes times;
};

struct CipherContext {
    std::vector<uint8_t> key;
    std::vector<uint8_t> nonce;
    bool use_avx2 = true;
};

class IImageCipher {
public:
    virtual ~IImageCipher() = default;
    virtual std::string name() const = 0;
    virtual std::string variant() const = 0;
    virtual CipherResult encrypt(const cv::Mat& plain, const CipherContext& ctx) = 0;
    virtual CipherResult decrypt(const cv::Mat& cipher, const CipherContext& ctx) = 0;
};

using CipherPtr = std::unique_ptr<IImageCipher>;

}  // namespace bench
