// Minimal assertion helpers shared by the unit test drivers.
#ifndef TEST_H
#define TEST_H

#include <stdio.h>

static int t_pass = 0;
static int t_fail = 0;

#define T_CHECK(cond)                                       \
  do {                                                      \
    if (cond) {                                             \
      t_pass++;                                             \
    } else {                                                \
      t_fail++;                                             \
      fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond); \
    }                                                       \
  } while (0)

// Print a summary and return a suitable exit code.
#define T_SUMMARY(name)                        \
  do {                                         \
    printf("%s: %d passed, %d failed\n", name, t_pass, t_fail); \
    return t_fail ? 1 : 0;                     \
  } while (0)

#endif
