#pragma once

#include <cstddef>
#include <cstdint>

#define BLAKE3_OUT_LEN 32

extern "C" {

typedef struct {
    uint64_t state[4];
    uint64_t len;
} blake3_hasher;

void blake3_hasher_init(blake3_hasher* self);
void blake3_hasher_update(blake3_hasher* self, const void* input, size_t input_len);
void blake3_hasher_finalize(const blake3_hasher* self, uint8_t* out, size_t out_len);

}
