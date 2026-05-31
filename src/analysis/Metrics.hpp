#pragma once

#include <array>
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <numeric>
#include <string>
#include <vector>

#include <opencv2/opencv.hpp>

#include "core/Utils.hpp"

namespace bench {

struct AnalysisMetrics {
    double entropy = 0.0;
    double chi_square = 0.0;
    double corr_h = 0.0;
    double corr_v = 0.0;
    double corr_d = 0.0;
    double npcr = 0.0;
    double uaci = 0.0;
    double key_sensitivity = 0.0;
    double kpa_score = 0.0;
    double cpa_score = 0.0;
};

inline std::array<uint64_t, 256> histogram_counts(const cv::Mat& img) {
    std::array<uint64_t, 256> hist{};
    cv::Mat src = make_contiguous(img);
    const size_t n = image_bytes(src);
    for (size_t i = 0; i < n; ++i) hist[src.data[i]]++;
    return hist;
}

inline double entropy(const cv::Mat& img) {
    auto hist = histogram_counts(img);
    const double n = static_cast<double>(image_bytes(img));
    double e = 0.0;
    for (uint64_t c : hist) {
        if (c == 0) continue;
        const double p = static_cast<double>(c) / n;
        e -= p * std::log2(p);
    }
    return e;
}

inline double chi_square(const cv::Mat& img) {
    auto hist = histogram_counts(img);
    const double expected = static_cast<double>(image_bytes(img)) / 256.0;
    double x2 = 0.0;
    for (uint64_t c : hist) {
        const double d = static_cast<double>(c) - expected;
        x2 += (d * d) / expected;
    }
    return x2;
}

inline double corr_direction(const cv::Mat& img, int dx, int dy) {
    cv::Mat gray;
    if (img.channels() == 1) {
        gray = img;
    } else {
        cv::cvtColor(img, gray, cv::COLOR_BGR2GRAY);
    }

    double sx = 0.0, sy = 0.0, sxx = 0.0, syy = 0.0, sxy = 0.0;
    uint64_t n = 0;
    for (int y = 0; y + dy < gray.rows; ++y) {
        for (int x = 0; x + dx < gray.cols; ++x) {
            const double a = gray.at<uint8_t>(y, x);
            const double b = gray.at<uint8_t>(y + dy, x + dx);
            sx += a;
            sy += b;
            sxx += a * a;
            syy += b * b;
            sxy += a * b;
            ++n;
        }
    }
    if (n < 2) return 0.0;
    const double dn = static_cast<double>(n);
    const double cov = sxy / dn - (sx / dn) * (sy / dn);
    const double vx = sxx / dn - (sx / dn) * (sx / dn);
    const double vy = syy / dn - (sy / dn) * (sy / dn);
    const double denom = std::sqrt(std::max(0.0, vx * vy));
    return denom > 0.0 ? cov / denom : 0.0;
}

inline double npcr(const cv::Mat& a, const cv::Mat& b) {
    cv::Mat aa = make_contiguous(a);
    cv::Mat bb = make_contiguous(b);
    const size_t n = std::min(image_bytes(aa), image_bytes(bb));
    size_t diff = 0;
    for (size_t i = 0; i < n; ++i) diff += aa.data[i] != bb.data[i];
    return n ? (100.0 * static_cast<double>(diff) / static_cast<double>(n)) : 0.0;
}

inline double uaci(const cv::Mat& a, const cv::Mat& b) {
    cv::Mat aa = make_contiguous(a);
    cv::Mat bb = make_contiguous(b);
    const size_t n = std::min(image_bytes(aa), image_bytes(bb));
    double sum = 0.0;
    for (size_t i = 0; i < n; ++i) sum += std::abs(static_cast<int>(aa.data[i]) - static_cast<int>(bb.data[i])) / 255.0;
    return n ? (100.0 * sum / static_cast<double>(n)) : 0.0;
}

inline void write_histogram_plot(const cv::Mat& img, const std::string& path) {
    auto hist = histogram_counts(img);
    const uint64_t max_count = *std::max_element(hist.begin(), hist.end());
    cv::Mat plot(320, 512, CV_8UC3, cv::Scalar(255, 255, 255));
    for (int i = 0; i < 256; ++i) {
        const int h = max_count ? static_cast<int>((static_cast<double>(hist[i]) / max_count) * 300.0) : 0;
        cv::line(plot, cv::Point(i * 2, 319), cv::Point(i * 2, 319 - h), cv::Scalar(40, 40, 40), 1);
        cv::line(plot, cv::Point(i * 2 + 1, 319), cv::Point(i * 2 + 1, 319 - h), cv::Scalar(40, 40, 40), 1);
    }
    cv::imwrite(path, plot);
}

inline AnalysisMetrics base_metrics(const cv::Mat& cipher) {
    AnalysisMetrics m;
    m.entropy = entropy(cipher);
    m.chi_square = chi_square(cipher);
    m.corr_h = corr_direction(cipher, 1, 0);
    m.corr_v = corr_direction(cipher, 0, 1);
    m.corr_d = corr_direction(cipher, 1, 1);
    return m;
}

inline double xor_recovery_score(const cv::Mat& known_plain,
                                 const cv::Mat& known_cipher,
                                 const cv::Mat& target_plain,
                                 const cv::Mat& target_cipher) {
    std::vector<uint8_t> p1 = mat_to_bytes(known_plain);
    std::vector<uint8_t> c1 = mat_to_bytes(known_cipher);
    std::vector<uint8_t> p2 = mat_to_bytes(target_plain);
    std::vector<uint8_t> c2 = mat_to_bytes(target_cipher);
    const size_t n = std::min({p1.size(), c1.size(), p2.size(), c2.size()});
    if (!n) return 0.0;
    size_t recovered = 0;
    for (size_t i = 0; i < n; ++i) {
        const uint8_t ks = p1[i] ^ c1[i];
        recovered += static_cast<uint8_t>(c2[i] ^ ks) == p2[i];
    }
    return static_cast<double>(recovered) / static_cast<double>(n);
}

}  // namespace bench
