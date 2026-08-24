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

// Spec values for the 48-bit Bobcat hash.
#define BLOCK_SIZE 16      // bytes per block
#define WORDS_PER_BLOCK 8  // 16-bit words per block
#define WORD_MASK 0xffff
#define INIT_A 0xface
#define INIT_B 0xe961
#define INIT_C 0x041d
#define ROUND1_MULT 5
#define ROUND2_MULT 7
#define ROUND3_MULT 9
#define KS_X0_CONST 0xa5a5a5a5
#define KS_X7_CONST 0x235689bd

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
  cc = (cc ^ x) & WORD_MASK;
  aa = (aa - (sbox[0][cc & 0xf] ^ sbox[1][(cc >> 8) & 0xf])) & WORD_MASK;
  bb = (bb + (sbox[1][(cc >> 4) & 0xf] ^
              sbox[0][(cc >> 12) & 0xf])) & WORD_MASK;
  abc[z] = aa;
  abc[o] = (bb * m) & WORD_MASK;
  abc[t] = cc;
}

// Outer round F_m: eight inner rounds over one block.
static void f_outer(int abc[3], int m, const int x[WORDS_PER_BLOCK]) {
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
static void key_schedule(int x[WORDS_PER_BLOCK]) {
  x[0] = (x[0] - (x[7] ^ KS_X0_CONST)) & WORD_MASK;
  x[1] = (x[1] ^ x[0]) & WORD_MASK;
  x[2] = (x[2] + x[1]) & WORD_MASK;
  x[3] = (x[3] - (x[2] ^ ((~x[1]) << 5))) & WORD_MASK;
  x[4] = (x[4] ^ x[3]) & WORD_MASK;
  x[5] = (x[5] + x[4]) & WORD_MASK;
  x[6] = (x[6] - (x[5] ^ ((~x[4]) >> 6))) & WORD_MASK;
  x[7] = (x[7] ^ x[6]) & WORD_MASK;
  x[0] = (x[0] + x[7]) & WORD_MASK;
  x[1] = (x[1] - (x[0] ^ ((~x[7]) << 5))) & WORD_MASK;
  x[2] = (x[2] ^ x[1]) & WORD_MASK;
  x[3] = (x[3] + x[2]) & WORD_MASK;
  x[4] = (x[4] - (x[3] ^ ((~x[2]) >> 6))) & WORD_MASK;
  x[5] = (x[5] ^ x[4]) & WORD_MASK;
  x[6] = (x[6] + x[5]) & WORD_MASK;
  x[7] = (x[7] - (x[6] ^ KS_X7_CONST)) & WORD_MASK;
}

// Fold the pre-round state back in (feedforward).
static void feedforward(int abc[3], const int old[3]) {
  abc[0] = (abc[0] ^ old[0]) & WORD_MASK;
  abc[1] = (abc[1] - old[1]) & WORD_MASK;
  abc[2] = (abc[2] + old[2]) & WORD_MASK;
}

unsigned long long bobcat48(const void *msg, size_t len) {
  const unsigned char *p = (const unsigned char *)msg;
  int abc[3] = {INIT_A, INIT_B, INIT_C};
  size_t nblocks = (len + BLOCK_SIZE - 1) / BLOCK_SIZE;
  if (nblocks == 0) {
    nblocks = 1;  // keep the hash defined for empty input
  }
  for (size_t b = 0; b < nblocks; b++) {
    int old[3] = {abc[0], abc[1], abc[2]};
    int x[WORDS_PER_BLOCK];
    // Pack bytes little-endian into 16-bit words; pad with zero words.
    for (int w = 0; w < WORDS_PER_BLOCK; w++) {
      size_t off = b * BLOCK_SIZE + (size_t)w * 2;
      int lo = off < len ? p[off] : 0;
      int hi = off + 1 < len ? p[off + 1] : 0;
      x[w] = lo | (hi << 8);
    }
    f_outer(abc, ROUND1_MULT, x);
    key_schedule(x);
    f_outer(abc, ROUND2_MULT, x);
    key_schedule(x);
    f_outer(abc, ROUND3_MULT, x);
    feedforward(abc, old);
  }
  return ((unsigned long long)abc[0] << 32) |
         ((unsigned long long)abc[1] << 16) | (unsigned long long)abc[2];
}
