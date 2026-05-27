#pragma once

#include <cstddef>
#include <cstdint>
#include <random>
#include <string>
#include <vector>

namespace chaosref {

struct Image {
    int width = 0;
    int height = 0;
    int channels = 0;
    std::vector<uint8_t> data;

    size_t bytes() const { return data.size(); }
    int pixels() const { return width * height; }
    bool empty() const { return data.empty(); }
};

inline Image make_random_image(int width, int height, int channels, uint64_t seed) {
    Image img;
    img.width = width;
    img.height = height;
    img.channels = channels;
    img.data.resize(static_cast<size_t>(width) * height * channels);

    std::mt19937_64 rng(seed);
    std::uniform_int_distribution<int> dist(0, 255);
    for (auto& b : img.data) b = static_cast<uint8_t>(dist(rng));
    return img;
}

inline bool equal_image(const Image& a, const Image& b) {
    return a.width == b.width && a.height == b.height && a.channels == b.channels && a.data == b.data;
}

inline std::string shape_string(const Image& img) {
    return std::to_string(img.width) + "x" + std::to_string(img.height) + "x" + std::to_string(img.channels);
}

}  // namespace chaosref
