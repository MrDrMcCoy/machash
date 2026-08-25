// Fuzz target: the 48-bit Bobcat hash.
#include "bobcat.h"

#include "fuzz.h"

#include <stdlib.h>
#include <string.h>

static void bobcat_target(const uint8_t *data, size_t size) {
  bobcat48(data, size);
}

int main(int argc, char **argv) {
  uint64_t runs = 100000;
  uint64_t seed = 1;
  for (int i = 1; i < argc; i++) {
    if (!strncmp(argv[i], "-runs=", 6)) {
      runs = strtoull(argv[i] + 6, NULL, 0);
    } else if (!strncmp(argv[i], "-seed=", 6)) {
      seed = strtoull(argv[i] + 6, NULL, 0);
    }
  }
  return fuzz_run("fuzz_bobcat", bobcat_target, FUZZ_RANDOM, seed, runs);
}
