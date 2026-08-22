// Unit tests for bobcat48 against the published reference vectors.
#include "bobcat.h"
#include "test.h"

// The four vectors published with the reference implementation.
static const int v1[16] = {0x5920, 0xc7cc, 0x6234, 0xb111, 0x3090, 0x8ab7,
                           0xf373, 0x46dd, 0x35d3, 0xc06e, 0x0000, 0x0000,
                           0x0000, 0x0000, 0x0000, 0x0000};
static const int v2[16] = {0xbe43, 0x83c6, 0x017a, 0x3e71, 0x44a5, 0x0a9f,
                           0x3349, 0x2c8a, 0x0f84, 0x6ce7, 0x0000, 0x0000,
                           0x0000, 0x0000, 0x0000, 0x0000};
static const int v3[32] = {0xbf37, 0xc638, 0x17a0, 0x1f71, 0xe4a5, 0x6a9e,
                           0x3449, 0x2caa, 0x0f84, 0x6ce7, 0x3578, 0x60fd,
                           0xa780, 0x81a8, 0x67c0, 0x789f, 0xbe34, 0x3c66,
                           0x417a, 0x3e71, 0x44a5, 0x0a9f, 0x3349, 0x2c8a,
                           0x0f43, 0x0ce7, 0x4321, 0x1234, 0xfe23, 0x0b0a,
                           0x0000, 0x0000};
static const int v4[32] = {0xa401, 0xeb2c, 0x988d, 0xe8a4, 0xbf27, 0x189b,
                           0xb231, 0x2faa, 0x23bc, 0xfd34, 0x2033, 0xb107,
                           0x797f, 0xe9d0, 0x3d16, 0xf236, 0xb871, 0x85ff,
                           0xd95b, 0x8721, 0x74f2, 0xd547, 0x4f91, 0x6f0c,
                           0x3e68, 0x549b, 0xfbd3, 0x9e3d, 0xc869, 0x593f,
                           0x75db, 0x6c6a};

// Expand 16-bit words to a little-endian byte string, as bobcat48 packs.
static size_t pack_le(const int *words, size_t n, unsigned char *out) {
  for (size_t i = 0; i < n; i++) {
    out[i * 2] = (unsigned char)(words[i] & 0xff);
    out[i * 2 + 1] = (unsigned char)((words[i] >> 8) & 0xff);
  }
  return n * 2;
}

int main(void) {
  unsigned char buf[64];
  struct vec {
    const int *words;
    size_t nwords;
    unsigned long long want;
  };
  struct vec vecs[] = {
      {v1, 16, 0x2bce98fa6186ULL},
      {v2, 16, 0x75ebe721c8cdULL},
      {v3, 32, 0xf20aec83c477ULL},
      {v4, 32, 0x7e5abf5fad22ULL},
  };
  for (size_t i = 0; i < sizeof(vecs) / sizeof(vecs[0]); i++) {
    size_t n = pack_le(vecs[i].words, vecs[i].nwords, buf);
    unsigned long long got = bobcat48(buf, n);
    if (got != vecs[i].want) {
      fprintf(stderr, "vector %zu: got 0x%012llx want 0x%012llx\n", i,
              (unsigned long long)got, (unsigned long long)vecs[i].want);
    }
    T_CHECK(got == vecs[i].want);
  }

  // Empty input must equal one block of zero bytes (both hash one
  // zero-padded block), keeping the empty-string hash well-defined.
  unsigned char zeros[16] = {0};
  T_CHECK(bobcat48("", 0) == bobcat48(zeros, sizeof(zeros)));
  // Expected value certified by the independent oracle (check_oracle.sh).
  T_CHECK(bobcat48("", 0) == 0xe6be9fc25f39ULL);

  // Truncating an input mid-word changes the hash (sanity).
  T_CHECK(bobcat48(buf, 16) != bobcat48(buf, 15));

  T_SUMMARY("unit_bobcat");
}
