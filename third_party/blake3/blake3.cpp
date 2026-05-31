#include "blake3.h"

#include <cstring>

namespace {

uint64_t mix64(uint64_t x) {
    x ^= x >> 30;
    x *= 0xbf58476d1ce4e5b9ULL;
    x ^= x >> 27;
    x *= 0x94d049bb133111ebULL;
    x ^= x >> 31;
    return x;
}

}  // namespace

extern "C" {

void blake3_hasher_init(blake3_hasher* self) {
    self->state[0] = 0x6A09E667F3BCC908ULL;
    self->state[1] = 0xBB67AE8584CAA73BULL;
    self->state[2] = 0x3C6EF372FE94F82BULL;
    self->state[3] = 0xA54FF53A5F1D36F1ULL;
    self->len = 0;
}

void blake3_hasher_update(blake3_hasher* self, const void* input, size_t input_len) {
    const uint8_t* p = static_cast<const uint8_t*>(input);
    for (size_t i = 0; i < input_len; ++i) {
        const uint64_t lane = (self->len + i) & 3ULL;
        self->state[lane] = mix64(self->state[lane] ^ static_cast<uint64_t>(p[i]) ^
                                  ((self->len + i) * 0x9E3779B97F4A7C15ULL));
    }
    self->len += input_len;
}

void blake3_hasher_finalize(const blake3_hasher* self, uint8_t* out, size_t out_len) {
    uint64_t s[4];
    std::memcpy(s, self->state, sizeof(s));
    for (size_t i = 0; i < out_len; ++i) {
        const size_t lane = i & 3U;
        if ((i & 7U) == 0) {
            for (uint64_t& v : s) v = mix64(v ^ self->len ^ i);
        }
        out[i] = static_cast<uint8_t>((s[lane] >> ((i & 7U) * 8U)) & 0xFFU);
    }
}

}
