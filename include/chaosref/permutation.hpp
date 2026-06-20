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
            const size_t row = static_cast<size_t>(y) * W * C;
            for (int x = 0; x < W; ++x) {
                const int nx = (x + y) % W;
                const int ny = (y + nx) % H;
                const size_t src_idx = row + static_cast<size_t>(x) * C;
                const size_t dst_idx = (static_cast<size_t>(ny) * W + nx) * C;
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
                const int ny = (y + nx) % H;
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

inline Image permute_bitplanes(const Image& src) {
    Image out = src;
    size_t i = 0;
    for (; i + 8 <= src.data.size(); i += 8) {
        for (unsigned row = 0; row < 8; ++row) {
            uint8_t value = 0;
            for (unsigned col = 0; col < 8; ++col) {
                value |= static_cast<uint8_t>(((src.data[i + col] >> row) & 1U) << col);
            }
            out.data[i + row] = value;
        }
    }
    return out;
}

inline uint8_t symbolic_add(uint8_t value, uint8_t delta) {
    uint8_t out = 0;
    for (unsigned shift = 0; shift < 8; shift += 2) {
        const uint8_t symbol = static_cast<uint8_t>((value >> shift) & 3U);
        out |= static_cast<uint8_t>(((symbol + delta) & 3U) << shift);
    }
    return out;
}

inline Image permute_symbolic(const Image& src, uint64_t key_seed, bool inverse = false) {
    Image out = src;
    uint64_t state = key_seed;
    for (size_t i = 0; i < src.data.size(); ++i) {
        if ((i & 7U) == 0) state = splitmix64(state);
        uint8_t delta = static_cast<uint8_t>((state >> ((i & 7U) * 8U)) & 3U);
        if (inverse) delta = static_cast<uint8_t>((4U - delta) & 3U);
        out.data[i] = symbolic_add(src.data[i], delta);
    }
    return out;
}

}  // namespace chaosref
