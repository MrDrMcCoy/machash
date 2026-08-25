// Fuzz target: the option parser and its validators.
//
// Each input is a NUL-separated run of command-line words. The spec
// table mirrors machash's real options, including the validating
// callbacks, so the parser and the validators run together.
#include "args.h"

#include "fuzz.h"
#include "log.h"
#include "mac.h"

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_WORDS 64
#define VALUE_BUF 64

static char stored[8][VALUE_BUF];
static int nstored;
static int flags;
static int check_enabled;
static unsigned long long check_value;

static int store_value(const char *v, void *user) {
  (void)user;
  if (nstored < 8) {
    snprintf(stored[nstored], VALUE_BUF, "%s", v);
  }
  nstored++;
  return 0;
}

static int set_check(const char *v, void *user) {
  (void)user;
  if (mac_parse(v, &check_value) != 0) {
    return -1;
  }
  check_enabled = 1;
  return 0;
}

static int set_loglevel(const char *v, void *user) {
  (void)user;
  if (parse_log_level(v) < 0) {
    return -1;
  }
  return 0;
}

// Mirrors the option table of machash main(), with the same shapes:
// value options with validating callbacks and boolean flags.
static const opt_spec_t specs[] = {
    {.long_name = "string", .short_name = 's', .takes_value = 1,
     .cb = store_value},
    {.long_name = "plain", .short_name = 'p', .bool_dest = &flags},
    {.long_name = "0x", .bool_dest = &flags},
    {.long_name = "swap", .short_name = 'S', .bool_dest = &flags},
    {.long_name = "unicast", .short_name = 'u', .bool_dest = &flags},
    {.long_name = "local", .bool_dest = &flags},
    {.long_name = "lines", .short_name = 'l', .bool_dest = &flags},
    {.long_name = "file", .short_name = 'f', .takes_value = 1,
     .cb = store_value},
    {.long_name = "hostname", .short_name = 'n', .bool_dest = &flags},
    {.long_name = "interface", .short_name = 'i', .takes_value = 1,
     .cb = store_value},
    {.long_name = "check", .takes_value = 1, .cb = set_check},
    {.long_name = "loglevel", .short_name = 'L', .takes_value = 1,
     .cb = set_loglevel},
    {.long_name = "help", .short_name = 'h', .bool_dest = &flags},
    {.long_name = "version", .bool_dest = &flags},
};

static void args_target(const uint8_t *data, size_t size) {
  // Split the input into at most MAX_WORDS NUL-terminated words.
  static char *argv[MAX_WORDS + 1];
  int argc = 0;
  argv[argc++] = (char *)"fuzz";
  size_t off = 0;
  while (off <= size && argc < MAX_WORDS) {
    argv[argc++] = (char *)(data + off);
    off += strlen((const char *)data + off) + 1;
  }
  parse_opts("fuzz", argc, argv, specs, 14, store_value, NULL);
  // Exercise the scalar validators on the first few words.
  for (int i = 1; i < argc && i < 5; i++) {
    parse_bool(argv[i]);
    parse_log_level(argv[i]);
    mac_parse(argv[i], &check_value);
  }
}

int main(int argc, char **argv) {
  // The parser prints a usage error for nearly every random input;
  // send that to a file so the run stays readable. A UBSan report
  // lands in the same file; the Makefile prints it on failure.
  int fd = open("build/fuzz_args.stderr", O_WRONLY | O_CREAT | O_TRUNC, 0644);
  if (fd >= 0) {
    dup2(fd, STDERR_FILENO);
    close(fd);
  }
  log_level = LOG_OFF;
  uint64_t runs = 100000;
  uint64_t seed = 1;
  for (int i = 1; i < argc; i++) {
    if (!strncmp(argv[i], "-runs=", 6)) {
      runs = strtoull(argv[i] + 6, NULL, 0);
    } else if (!strncmp(argv[i], "-seed=", 6)) {
      seed = strtoull(argv[i] + 6, NULL, 0);
    }
  }
  fuzz_run("fuzz_args", args_target, FUZZ_RANDOM, seed, runs);
  return fuzz_run("fuzz_args", args_target, FUZZ_TEXT, seed + 1, runs);
}
