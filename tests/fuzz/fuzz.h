// A small deterministic fuzz driver.
//
// The GCC-based cosmocc has no libFuzzer mode, so this header
// provides a fixed-seed, splitmix64-driven input generator instead.
// A target is a callback that receives pseudo-random input; a crash
// or a UBSan report (the fuzz targets build with
// -fsanitize=undefined -fno-sanitize-recover=all) is a finding.
#ifndef FUZZ_H
#define FUZZ_H

#include <stddef.h>
#include <stdint.h>

// Input style for one fuzz run.
typedef enum {
  FUZZ_RANDOM, // uniform random bytes
  FUZZ_TEXT,   // mostly printable ASCII, with NULs mixed in
} fuzz_mode;

// The target under test.
typedef void (*fuzz_target_t)(const uint8_t *data, size_t size);

// Maximum input length in bytes.
#define FUZZ_MAX_LEN 4096

// Calls target on runs pseudo-random inputs derived from seed and
// prints a one-line summary. Returns 0.
int fuzz_run(const char *name, fuzz_target_t target, fuzz_mode mode,
             uint64_t seed, uint64_t runs);

#endif
