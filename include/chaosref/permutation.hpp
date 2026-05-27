#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <numeric>
#include <vector>

#include "chaosref/chaotic_rng.hpp"
#include "chaosref/types.hpp"

namespace chaosref {

inline Image permute_map_fridrich(const Image& src, int rounds = 2) {
    Image cur = src;
    Image tmp = src;

    const int H = src.height;
    const int W = src.width;
    const int C = src.channels;

    for (int round = 0; round < rounds; ++round) {
        const uint8_t* in = cur.data.data();
        uint8_t* out = tmp.data.data();
        for (int y = 0; y < H; ++y) {
            int nx = y % W;
            int ny = (2 * y) % H;
            const size_t row = static_cast<size_t>(y) * W * C;
            for (int x = 0; x < W; ++x) {
                const size_t src_idx = row + static_cast<size_t>(x) * C;
                const size_t dst_idx = (static_cast<size_t>(ny) * W + nx) * C;
                if (C == 3) {
                    out[dst_idx + 0] = in[src_idx + 0];
                    out[dst_idx + 1] = in[src_idx + 1];
                    out[dst_idx + 2] = in[src_idx + 2];
                } else {
                    std::memcpy(out + dst_idx, in + src_idx, static_cast<size_t>(C));
                }
                if (++nx == W) nx = 0;
                if (++ny == H) ny = 0;
            }
        }
        cur.data.swap(tmp.data);
    }
    return cur;
}

inline Image invert_permute_map_fridrich(const Image& src, int rounds = 2) {
    Image cur = src;
    Image tmp = src;

    const int H = src.height;
    const int W = src.width;
    const int C = src.channels;

    for (int round = 0; round < rounds; ++round) {
        const uint8_t* in = cur.data.data();
        uint8_t* out = tmp.data.data();
        for (int y = 0; y < H; ++y) {
            const size_t row = static_cast<size_t>(y) * W * C;
            for (int x = 0; x < W; ++x) {
                const int nx = (x + y) % W;
                const int ny = (x + 2 * y) % H;
                const size_t src_idx = (static_cast<size_t>(ny) * W + nx) * C;
                const size_t dst_idx = row + static_cast<size_t>(x) * C;
                if (C == 3) {
                    out[dst_idx + 0] = in[src_idx + 0];
                    out[dst_idx + 1] = in[src_idx + 1];
                    out[dst_idx + 2] = in[src_idx + 2];
                } else {
                    std::memcpy(out + dst_idx, in + src_idx, static_cast<size_t>(C));
                }
            }
        }
        cur.data.swap(tmp.data);
    }
    return cur;
}

inline std::vector<uint32_t> build_sort_permutation(size_t pixels, uint64_t key_seed) {
    std::vector<double> scores(pixels);
    fill_logistic_scores(scores, key_seed);

    std::vector<uint32_t> idx(pixels);
    std::iota(idx.begin(), idx.end(), 0U);

    std::stable_sort(idx.begin(), idx.end(), [&](uint32_t a, uint32_t b) {
        return scores[a] < scores[b];
    });
    return idx;
}

inline Image permute_sort(const Image& src, const std::vector<uint32_t>& idx) {
    Image out = src;
    const int C = src.channels;
    const uint8_t* in = src.data.data();
    uint8_t* dst = out.data.data();

    const size_t N = idx.size();
    for (size_t i = 0; i < N; ++i) {
        const size_t src_off = i * static_cast<size_t>(C);
        const size_t dst_off = static_cast<size_t>(idx[i]) * C;
        if (C == 3) {
            dst[dst_off + 0] = in[src_off + 0];
            dst[dst_off + 1] = in[src_off + 1];
            dst[dst_off + 2] = in[src_off + 2];
        } else {
            std::memcpy(dst + dst_off, in + src_off, static_cast<size_t>(C));
        }
    }
    return out;
}

inline Image invert_permute_sort(const Image& src, const std::vector<uint32_t>& idx) {
    Image out = src;
    const int C = src.channels;
    const uint8_t* in = src.data.data();
    uint8_t* dst = out.data.data();

    const size_t N = idx.size();
    for (size_t i = 0; i < N; ++i) {
        const size_t src_off = static_cast<size_t>(idx[i]) * C;
        const size_t dst_off = i * static_cast<size_t>(C);
        if (C == 3) {
            dst[dst_off + 0] = in[src_off + 0];
            dst[dst_off + 1] = in[src_off + 1];
            dst[dst_off + 2] = in[src_off + 2];
        } else {
            std::memcpy(dst + dst_off, in + src_off, static_cast<size_t>(C));
        }
    }
    return out;
}

}  // namespace chaosref
