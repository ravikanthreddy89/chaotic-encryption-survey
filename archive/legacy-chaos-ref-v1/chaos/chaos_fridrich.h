#pragma once

#include <opencv2/opencv.hpp>

#include "chaos_common.h"

namespace Ciphers {

struct Fridrich1998 {
    static constexpr const char* NAME = "Fridrich-1998";
    static constexpr int LOC = 82;

    struct WorkBuffers {
        std::vector<uint8_t> ks;
        std::vector<uint8_t> scratch;
    };
    static WorkBuffers& buffers() { thread_local WorkBuffers wb; return wb; }

    static cv::Mat encrypt_image(const cv::Mat& src) {
        double x0, y0, r;
        {
            ChaosProfile::Scope s(NAME, ChaosProfile::Stage::KeyDerive);
            derive_ic(Bench::FIXED_KEY.data(), x0, y0, r);
        }

        const int H = src.rows, W = src.cols, C = src.channels();
        const int N = H * W;
        const int total_bytes = N * C;
        WorkBuffers& wb = buffers();
        wb.ks.resize(total_bytes);

        cv::Mat perm = src.clone();
        cv::Mat tmp(H, W, src.type());
        {
            ChaosProfile::Scope s(NAME, ChaosProfile::Stage::PermuteApply);
            for (int round = 0; round < 2; ++round) {
                const uint8_t* perm_ptr = perm.data;
                uint8_t* tmp_ptr = tmp.data;
                for (int y = 0; y < H; ++y) {
                    int nx = y % W;
                    int ny = (2 * y) % H;
                    const size_t src_row = static_cast<size_t>(y) * W * C;
                    for (int x = 0; x < W; ++x) {
                        const size_t src_idx = src_row + static_cast<size_t>(x) * C;
                        const size_t dst_idx = (static_cast<size_t>(ny) * W + nx) * C;
                        if (C == 3) {
                            tmp_ptr[dst_idx + 0] = perm_ptr[src_idx + 0];
                            tmp_ptr[dst_idx + 1] = perm_ptr[src_idx + 1];
                            tmp_ptr[dst_idx + 2] = perm_ptr[src_idx + 2];
                        } else {
                            std::memcpy(tmp_ptr + dst_idx, perm_ptr + src_idx, static_cast<size_t>(C));
                        }
                        if (++nx == W) nx = 0;
                        if (++ny == H) ny = 0;
                    }
                }
                std::swap(perm, tmp);
            }
        }

        Logistic lm(x0, r);
        cv::Mat out(H, W, src.type());
        {
            ChaosProfile::Scope s(NAME, ChaosProfile::Stage::SeqGen);
            for (auto& b : wb.ks) b = lm.byte();
        }

        {
            ChaosProfile::Scope s(NAME, ChaosProfile::Stage::Diffuse);
            uint8_t chain = 0;
#if CHAOS_DIFFUSE_KERNEL != 0
            diffuse_chain(perm.data, wb.ks.data(), out.data, static_cast<size_t>(total_bytes), chain, wb.scratch);
#else
            for (int i = 0; i < N; ++i) {
                for (int c = 0; c < C; ++c) {
                    uint8_t p = perm.data[i * C + c];
                    uint8_t k = wb.ks[i * C + c];
                    uint8_t e = p ^ k ^ chain;
                    out.data[i * C + c] = e;
                    chain = e;
                }
            }
#endif
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
        const int total_bytes = N * C;
        WorkBuffers& wb = buffers();
        wb.ks.resize(total_bytes);
        {
            ChaosProfile::Scope s(NAME, ChaosProfile::Stage::SeqGen);
            for (auto& b : wb.ks) b = lm.byte();
        }

        cv::Mat undiff(H, W, type);
        {
            ChaosProfile::Scope s(NAME, ChaosProfile::Stage::Reverse);
            uint8_t chain = 0;
#if CHAOS_DIFFUSE_KERNEL != 0
            undiffuse_chain(src.data, wb.ks.data(), undiff.data, static_cast<size_t>(total_bytes), chain, wb.scratch);
#else
            for (int i = 0; i < N; ++i) {
                for (int c = 0; c < C; ++c) {
                    uint8_t e = src.data[i * C + c];
                    uint8_t k = wb.ks[i * C + c];
                    undiff.data[i * C + c] = e ^ k ^ chain;
                    chain = e;
                }
            }
#endif
        }

        cv::Mat out = undiff.clone();
        cv::Mat tmp(H, W, type);
        {
            ChaosProfile::Scope s(NAME, ChaosProfile::Stage::PermuteApply);
            for (int round = 0; round < 2; ++round) {
                const uint8_t* out_ptr = out.data;
                uint8_t* tmp_ptr = tmp.data;
                for (int y = 0; y < H; ++y) {
                    int ox = ((-y) % W + W) % W;
                    int oy = y % H;
                    const size_t src_row = static_cast<size_t>(y) * W * C;
                    for (int x = 0; x < W; ++x) {
                        const size_t src_idx = src_row + static_cast<size_t>(x) * C;
                        const size_t dst_idx = (static_cast<size_t>(oy) * W + ox) * C;
                        if (C == 3) {
                            tmp_ptr[dst_idx + 0] = out_ptr[src_idx + 0];
                            tmp_ptr[dst_idx + 1] = out_ptr[src_idx + 1];
                            tmp_ptr[dst_idx + 2] = out_ptr[src_idx + 2];
                        } else {
                            std::memcpy(tmp_ptr + dst_idx, out_ptr + src_idx, static_cast<size_t>(C));
                        }
                        ox += 2;
                        if (ox >= W) ox -= W;
                        if (ox >= W) ox -= W;
                        --oy;
                        if (oy < 0) oy += H;
                    }
                }
                std::swap(out, tmp);
            }
        }
        return out;
    }
};

} // namespace Ciphers
