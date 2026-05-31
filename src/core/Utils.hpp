#pragma once

#include <chrono>
#include <algorithm>
#include <cctype>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

#include <opencv2/opencv.hpp>

namespace bench {

class Timer {
public:
    Timer() : start_(std::chrono::steady_clock::now()) {}
    double elapsed_ms() const {
        const auto end = std::chrono::steady_clock::now();
        return std::chrono::duration<double, std::milli>(end - start_).count();
    }

private:
    std::chrono::steady_clock::time_point start_;
};

inline size_t image_bytes(const cv::Mat& img) {
    return static_cast<size_t>(img.total()) * img.elemSize();
}

inline cv::Mat make_contiguous(const cv::Mat& img) {
    return img.isContinuous() ? img : img.clone();
}

inline std::vector<uint8_t> mat_to_bytes(const cv::Mat& img) {
    cv::Mat src = make_contiguous(img);
    const size_t n = image_bytes(src);
    return std::vector<uint8_t>(src.data, src.data + n);
}

inline cv::Mat bytes_to_mat(const std::vector<uint8_t>& bytes, int rows, int cols, int type) {
    cv::Mat out(rows, cols, type);
    std::memcpy(out.data, bytes.data(), std::min(bytes.size(), image_bytes(out)));
    return out;
}

inline void ensure_dir(const std::string& path) {
    std::filesystem::create_directories(path);
}

inline void write_text(const std::string& path, const std::string& text) {
    std::ofstream f(path);
    f << text;
}

inline std::string sanitize_name(std::string s) {
    for (char& c : s) {
        if (!(std::isalnum(static_cast<unsigned char>(c)) || c == '_' || c == '-')) c = '_';
    }
    return s;
}

inline std::string fixed(double v, int digits = 6) {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(digits) << v;
    return oss.str();
}

}  // namespace bench
