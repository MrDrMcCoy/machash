// 48-bit Bobcat hash, modeled on Tiger.
//
// The message is split into 128-bit blocks of eight 16-bit words. Each
// block passes through three outer rounds (multipliers 5, 7, 9) with a
// key schedule between them. The state is three 16-bit words (a, b, c)
// combined as (a << 32) | (b << 16) | c.
//
// The algorithm follows the reference at
// https://github.com/padolph/StampInfosec/blob/master/Chapter5/bobcat/bobcat.c
// and is validated against the test vectors published with it.

#include "bobcat.h"

// Two S-boxes, each mapping 4 bits to 16 bits.
static const int sbox[2][16] = {
    {0xd131, 0x0ba6, 0x98df, 0xb5ac, 0x2ffd, 0x72db, 0xd01a, 0xdfb7,
     0xb8e1, 0xafed, 0x6a26, 0x7e96, 0xba7c, 0x9045, 0xf12c, 0x7f99},
    {0x24a1, 0x9947, 0xb391, 0x6cf7, 0x0801, 0xf2e2, 0x858e, 0xfc16,
     0x6369, 0x20d8, 0x7157, 0x4e69, 0xa458, 0xfea3, 0xf493, 0x3d7e},
};

// Inner round f_{m,i}: mix one word x into the state.
static void f_inner(int abc[3], int z, int o, int t, int x, int m) {
  int aa = abc[z], bb = abc[o], cc = abc[t];
  cc = (cc ^ x) & 0xffff;
  aa = (aa - (sbox[0][cc & 0xf] ^ sbox[1][(cc >> 8) & 0xf])) & 0xffff;
  bb = (bb + (sbox[1][(cc >> 4) & 0xf] ^ sbox[0][(cc >> 12) & 0xf])) & 0xffff;
  abc[z] = aa;
  abc[o] = (bb * m) & 0xffff;
  abc[t] = cc;
}

// Outer round F_m: eight inner rounds over one block.
static void f_outer(int abc[3], int m, const int x[8]) {
  f_inner(abc, 0, 1, 2, x[0], m);
  f_inner(abc, 1, 2, 0, x[1], m);
  f_inner(abc, 2, 0, 1, x[2], m);
  f_inner(abc, 0, 1, 2, x[3], m);
  f_inner(abc, 1, 2, 0, x[4], m);
  f_inner(abc, 2, 0, 1, x[5], m);
  f_inner(abc, 0, 1, 2, x[6], m);
  f_inner(abc, 1, 2, 0, x[7], m);
}

// Derive the round words for the next outer round.
static void key_schedule(int x[8]) {
  x[0] = (x[0] - (x[7] ^ 0xa5a5a5a5)) & 0xffff;
  x[1] = (x[1] ^ x[0]) & 0xffff;
  x[2] = (x[2] + x[1]) & 0xffff;
  x[3] = (x[3] - (x[2] ^ ((~x[1]) << 5))) & 0xffff;
  x[4] = (x[4] ^ x[3]) & 0xffff;
  x[5] = (x[5] + x[4]) & 0xffff;
  x[6] = (x[6] - (x[5] ^ ((~x[4]) >> 6))) & 0xffff;
  x[7] = (x[7] ^ x[6]) & 0xffff;
  x[0] = (x[0] + x[7]) & 0xffff;
  x[1] = (x[1] - (x[0] ^ ((~x[7]) << 5))) & 0xffff;
  x[2] = (x[2] ^ x[1]) & 0xffff;
  x[3] = (x[3] + x[2]) & 0xffff;
  x[4] = (x[4] - (x[3] ^ ((~x[2]) >> 6))) & 0xffff;
  x[5] = (x[5] ^ x[4]) & 0xffff;
  x[6] = (x[6] + x[5]) & 0xffff;
  x[7] = (x[7] - (x[6] ^ 0x235689bd)) & 0xffff;
}

// Fold the pre-round state back in (feedforward).
static void feedforward(int abc[3], const int old[3]) {
  abc[0] = (abc[0] ^ old[0]) & 0xffff;
  abc[1] = (abc[1] - old[1]) & 0xffff;
  abc[2] = (abc[2] + old[2]) & 0xffff;
}

unsigned long long bobcat48(const void *msg, size_t len) {
  const unsigned char *p = (const unsigned char *)msg;
  int abc[3] = {0xface, 0xe961, 0x041d};
  size_t nblocks = (len + 15) / 16;
  if (nblocks == 0) {
    nblocks = 1;  // keep the hash defined for empty input
  }
  for (size_t b = 0; b < nblocks; b++) {
    int old[3] = {abc[0], abc[1], abc[2]};
    int x[8];
    // Pack bytes little-endian into 16-bit words; pad with zero words.
    for (int w = 0; w < 8; w++) {
      size_t off = b * 16 + (size_t)w * 2;
      int lo = off < len ? p[off] : 0;
      int hi = off + 1 < len ? p[off + 1] : 0;
      x[w] = lo | (hi << 8);
    }
    f_outer(abc, 5, x);
    key_schedule(x);
    f_outer(abc, 7, x);
    key_schedule(x);
    f_outer(abc, 9, x);
    feedforward(abc, old);
  }
  return ((unsigned long long)abc[0] << 32) |
         ((unsigned long long)abc[1] << 16) | (unsigned long long)abc[2];
}
