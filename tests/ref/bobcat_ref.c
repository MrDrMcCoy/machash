// Independent Bobcat oracle for testing machash.
//
// A second, differently structured implementation of the 48-bit Bobcat
// hash (single state triple, loop-driven rounds, no helper functions).
// It reads input bytes on stdin and prints the hash as 0x%012llx.
//
// It is validated against the four published reference vectors by
// tests/ref/check_oracle.sh before its output is used as expected
// values in the test suite. The algorithm follows the reference at
// https://github.com/padolph/StampInfosec/blob/master/Chapter5/bobcat/bobcat.c

#include <stdio.h>
#include <stdlib.h>

static const unsigned short S0[16] = {
    0xd131, 0x0ba6, 0x98df, 0xb5ac, 0x2ffd, 0x72db, 0xd01a, 0xdfb7,
    0xb8e1, 0xafed, 0x6a26, 0x7e96, 0xba7c, 0x9045, 0xf12c, 0x7f99};
static const unsigned short S1[16] = {
    0x24a1, 0x9947, 0xb391, 0x6cf7, 0x0801, 0xf2e2, 0x858e, 0xfc16,
    0x6369, 0x20d8, 0x7157, 0x4e69, 0xa458, 0xfea3, 0xf493, 0x3d7e};

int main(void) {
  static unsigned char in[65536];
  size_t n = fread(in, 1, sizeof(in), stdin);
  if (ferror(stdin)) {
    fprintf(stderr, "bobcat_ref: failed to read stdin\n");
    return 2;
  }

  // State: three 16-bit words.
  unsigned int a = 0xface, b = 0xe961, c = 0x041d;
  size_t nb = (n + 15) / 16;
  if (nb == 0) {
    nb = 1;  // empty input hashes one zero block
  }

  for (size_t blk = 0; blk < nb; blk++) {
    unsigned int x[8];
    for (int i = 0; i < 8; i++) {
      size_t o = blk * 16 + (size_t)i * 2;
      unsigned int lo = o < n ? in[o] : 0;
      unsigned int hi = o + 1 < n ? in[o + 1] : 0;
      x[i] = lo | (hi << 8);  // little-endian byte packing
    }

    unsigned int a0 = a, b0 = b, c0 = c;
    for (int r = 0; r < 3; r++) {
      unsigned int m = 5 + 2u * (unsigned int)r;
      for (int i = 0; i < 8; i++) {
        // Inner round; the (a,b,c) roles rotate each step, and with
        // them the formula applied to each state word.
        if (i % 3 == 0) {
          // roles: a=zero, b=one, c=two
          unsigned int cc = (c ^ x[i]) & 0xffff;
          unsigned int aa = (a - (S0[cc & 0xf] ^ S1[(cc >> 8) & 0xf])) & 0xffff;
          unsigned int bb =
              (b + (S1[(cc >> 4) & 0xf] ^ S0[(cc >> 12) & 0xf])) & 0xffff;
          a = aa;
          b = (bb * m) & 0xffff;
          c = cc;
        } else if (i % 3 == 1) {
          // roles: b=zero, c=one, a=two
          unsigned int cc = (a ^ x[i]) & 0xffff;
          unsigned int aa = (b - (S0[cc & 0xf] ^ S1[(cc >> 8) & 0xf])) & 0xffff;
          unsigned int bb =
              (c + (S1[(cc >> 4) & 0xf] ^ S0[(cc >> 12) & 0xf])) & 0xffff;
          b = aa;
          c = (bb * m) & 0xffff;
          a = cc;
        } else {
          // roles: c=zero, a=one, b=two
          unsigned int cc = (b ^ x[i]) & 0xffff;
          unsigned int aa = (c - (S0[cc & 0xf] ^ S1[(cc >> 8) & 0xf])) & 0xffff;
          unsigned int bb =
              (a + (S1[(cc >> 4) & 0xf] ^ S0[(cc >> 12) & 0xf])) & 0xffff;
          c = aa;
          a = (bb * m) & 0xffff;
          b = cc;
        }
      }
      // Key schedule for the next outer round.
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
    a = (a ^ a0) & 0xffff;
    b = (b - b0) & 0xffff;
    c = (c + c0) & 0xffff;
  }

  unsigned long long h = ((unsigned long long)a << 32) |
                         ((unsigned long long)b << 16) | (unsigned long long)c;
  printf("0x%012llx\n", h);
  return 0;
}
