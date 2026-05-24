#pragma once

#include <opencv2/opencv.hpp>

#include "chaos_common.h"

namespace Ciphers {

struct Fridrich1998 {
    static constexpr const char* NAME = "Fridrich-1998";
    static constexpr int LOC = 82;

    struct WorkBuffers { std::vector<uint8_t> ks; };
    static WorkBuffers& buffers() { thread_local WorkBuffers wb; return wb; }

    static cv::Mat encrypt_image(const cv::Mat& src) {
        double x0, y0, r;
        {
            ChaosProfile::Scope s(NAME, ChaosProfile::Stage::KeyDerive);
            derive_ic(Bench::FIXED_KEY.data(), x0, y0, r);
        }

        const int H = src.rows, W = src.cols, C = src.channels();

        cv::Mat perm = src.clone();
        cv::Mat tmp(H, W, src.type());
        {
            ChaosProfile::Scope s(NAME, ChaosProfile::Stage::PermuteApply);
            for (int round = 0; round < 2; ++round) {
                for (int y = 0; y < H; ++y)
                    for (int x = 0; x < W; ++x) {
                        int nx = (x + y) % W;
                        int ny = (x + 2 * y) % H;
                        for (int c = 0; c < C; ++c)
                            tmp.at<cv::Vec3b>(ny, nx)[c] = perm.at<cv::Vec3b>(y, x)[c];
                    }
                std::swap(perm, tmp);
            }
        }

        Logistic lm(x0, r);
        cv::Mat out(H, W, src.type());
        const int N = H * W;
        uint8_t prev = 0;
        {
            ChaosProfile::Scope s(NAME, ChaosProfile::Stage::Diffuse);
            for (int i = 0; i < N; ++i) {
                for (int c = 0; c < C; ++c) {
                    uint8_t p = perm.data[i * C + c];
                    uint8_t k = lm.byte();
                    uint8_t e = p ^ k ^ prev;
                    out.data[i * C + c] = e;
                    prev = e;
                }
            }
        }
        return out;
    }

    static cv::Mat decrypt_image(const cv::Mat& src, int rows, int cols, int type) {
        double x0, y0, r;
        {
            ChaosProfile::Scope s(NAME, ChaosProfile::Stage::KeyDerive);
            derive_ic(Bench::FIXED_KEY.data(), x0, y0, r);
        }

        Logistic lm(x0, r);
        const int H = rows, W = cols, C = src.channels();
        const int N = H * W;
        WorkBuffers& wb = buffers();
        wb.ks.resize(N * C);
        {
            ChaosProfile::Scope s(NAME, ChaosProfile::Stage::SeqGen);
            for (auto& b : wb.ks) b = lm.byte();
        }

        cv::Mat undiff(H, W, type);
        uint8_t prev = 0;
        {
            ChaosProfile::Scope s(NAME, ChaosProfile::Stage::Reverse);
            for (int i = 0; i < N; ++i) {
                for (int c = 0; c < C; ++c) {
                    uint8_t e = src.data[i * C + c];
                    uint8_t k = wb.ks[i * C + c];
                    undiff.data[i * C + c] = e ^ k ^ prev;
                    prev = e;
                }
            }
        }

        cv::Mat out = undiff.clone();
        cv::Mat tmp(H, W, type);
        {
            ChaosProfile::Scope s(NAME, ChaosProfile::Stage::PermuteApply);
            for (int round = 0; round < 2; ++round) {
                for (int y = 0; y < H; ++y)
                    for (int x = 0; x < W; ++x) {
                        int ox = ((2 * x - y) % W + W) % W;
                        int oy = ((-x + y) % H + H) % H;
                        for (int c = 0; c < C; ++c)
                            tmp.at<cv::Vec3b>(oy, ox)[c] = out.at<cv::Vec3b>(y, x)[c];
                    }
                std::swap(out, tmp);
            }
        }
        return out;
    }
};

} // namespace Ciphers
