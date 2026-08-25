// A small deterministic fuzz driver (see fuzz.h).
#include "fuzz.h"

#include <stdio.h>

// splitmix64: small, fast, well-distributed PRNG.
static uint64_t next_random(uint64_t *state) {
  uint64_t z = (*state += 0x9e3779b97f4a7c15ULL);
  z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
  z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
  return z ^ (z >> 31);
}

static uint8_t next_byte(uint64_t *state, fuzz_mode mode) {
  uint64_t r = next_random(state);
  if (mode == FUZZ_RANDOM) {
    return (uint8_t)(r & 0xff);
  }
  switch (r % 100) {
    case 0 ... 59:
      return (uint8_t)('a' + r % 26);
    case 60 ... 69:
      return (uint8_t)('A' + r % 26);
    case 70 ... 79:
      return (uint8_t)('0' + r % 10);
    case 80 ... 84:
      return ' ';
    case 85 ... 89:
      return (uint8_t)"-:=\n"[r % 4];
    default:
      return (uint8_t)(r & 0xff);
  }
}

int fuzz_run(const char *name, fuzz_target_t target, fuzz_mode mode,
             uint64_t seed, uint64_t runs) {
  static uint8_t buf[FUZZ_MAX_LEN + 1];
  uint64_t state = seed;
  for (uint64_t i = 0; i < runs; i++) {
    size_t n = (size_t)(next_random(&state) % (FUZZ_MAX_LEN + 1));
    for (size_t j = 0; j < n; j++) {
      buf[j] = next_byte(&state, mode);
    }
    buf[n] = 0;
    target(buf, n);
  }
  printf("%s: %llu runs, seed %llu, no errors\n", name,
         (unsigned long long)runs, (unsigned long long)seed);
  return 0;
}
