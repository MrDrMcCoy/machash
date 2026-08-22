// 48-bit Bobcat hash, modeled on Tiger.
#ifndef BOBCAT_H
#define BOBCAT_H

#include <stddef.h>

// Hash a byte string into 48 bits.
//
// Bytes are packed little-endian into 16-bit words; the final block is
// padded with zero words. An empty input hashes one zero block, so the
// result is defined for len == 0.
unsigned long long bobcat48(const void *msg, size_t len);

#endif
