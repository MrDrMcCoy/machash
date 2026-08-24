// machash: hash arbitrary strings into MAC addresses.
//
// Each input string is hashed with the 48-bit Bobcat hash and printed
// as a colon-separated MAC address by default, or as a raw hex number
// with -p/--plain. Inputs come from -s/--string and positional
// arguments; with no inputs, all of stdin is one input.

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "args.h"
#include "bobcat.h"
#include "log.h"

#define PROG "machash"

// Build info is baked in via build/version.h (Makefile-generated);
// defaults apply when compiling without it.
#if defined(__has_include)
#if __has_include("version.h")
#include "version.h"
#endif
#endif
#ifndef MACHASH_VERSION
#define MACHASH_VERSION "0.1.0"
#endif
#ifndef MACHASH_BUILD
#define MACHASH_BUILD "unknown"
#endif
#ifndef MACHASH_COMMIT
#define MACHASH_COMMIT "unknown"
#endif
#ifndef MACHASH_BUILD_NUMBER
#define MACHASH_BUILD_NUMBER "0"
#endif

// Bit positions in the 48-bit hash; the first octet is the MSB.
#define BIT_MULTICAST 0x800000000000ULL
#define BIT_LOCAL 0x400000000000ULL

// Read size for stdin, in bytes.
#define STDIN_CHUNK 65536
// Buffer size for a formatted MAC address, in bytes (18 + NUL).
#define MAC_STR_SIZE 19

// Output formats for the digest.
typedef enum {
  OUT_MAC,   // colon-separated MAC address
  OUT_PLAIN, // raw 12 hex digits
} output_fmt;

// Bit operations applied to the digest after hashing; combine with |.
typedef enum {
  BITS_NONE = 0,
  BITS_UNICAST = 1, // clear the multicast bit
  BITS_LOCAL = 2,   // set the locally-administered bit
} bit_ops;

// Ordered input strings (-s values and positionals, in command order).
struct inputs {
  const char **items;
  int count;
  int cap;
};

static int add_input(const char *s, void *user) {
  struct inputs *in = user;
  if (in->count == in->cap) {
    int ncap = in->cap ? in->cap * 2 : 8;
    const char **ni = realloc(in->items, (size_t)ncap * sizeof(*ni));
    if (!ni) {
      log_abort(PROG, "out of memory adding input string");
    }
    in->items = ni;
    in->cap = ncap;
  }
  in->items[in->count++] = s;
  return 0;
}

static int set_loglevel(const char *v, void *user) {
  (void)user;
  int lvl = parse_log_level(v);
  if (lvl < 0) {
    log_error(PROG,
              "invalid log level '%s' (expected off/fatal/error/warn/"
              "info/debug or 0-5); try '%s --help' for usage",
              v, PROG);
    return -1;
  }
  log_level = lvl;
  return 0;
}

// Read all of stdin; returns a malloc'd buffer or aborts on error.
static char *read_stdin_all(size_t *len_out) {
  size_t cap = STDIN_CHUNK, len = 0;
  char *buf = malloc(cap);
  if (!buf) {
    log_abort(PROG, "out of memory reading stdin");
  }
  for (;;) {
    if (len + STDIN_CHUNK > cap) {
      cap *= 2;
      char *nb = realloc(buf, cap);
      if (!nb) {
        log_abort(PROG, "out of memory reading stdin");
      }
      buf = nb;
    }
    size_t r = fread(buf + len, 1, STDIN_CHUNK, stdin);
    len += r;
    if (r < STDIN_CHUNK) {
      if (ferror(stdin)) {
        free(buf);
        log_abort(PROG, "failed to read stdin: %s", strerror(errno));
      }
      break;
    }
  }
  *len_out = len;
  return buf;
}

// Format h as a colon-separated MAC address; out needs 19 bytes.
static void format_mac(unsigned long long h, char *out) {
  char *p = out;
  for (int i = 5; i >= 0; i--) {
    if (i != 5) {
      *p++ = ':';
    }
    p += sprintf(p, "%02llx", (h >> (8 * i)) & 0xff);
  }
  *p = 0;
}

static void hash_input(const char *raw, size_t raw_len, output_fmt fmt,
                       bit_ops bits) {
  const char *in = raw;
  size_t len = raw_len;
  // Strip leading and trailing whitespace.
  while (len && isspace((unsigned char)*in)) {
    in++;
    len--;
  }
  while (len && isspace((unsigned char)in[len - 1])) {
    len--;
  }
  if (len == 0) {
    log_msg(PROG, LOG_WARN,
            "input is empty after stripping whitespace; hashing the "
            "empty string");
  }
  unsigned long long h = bobcat48(in, len);
  if (bits & BITS_LOCAL) {
    h |= BIT_LOCAL;
  }
  if (bits & BITS_UNICAST) {
    h &= ~BIT_MULTICAST;
  }
  log_msg(PROG, LOG_DEBUG, "hashed %zu byte input -> 0x%012llx", len, h);
  if (fmt == OUT_PLAIN) {
    printf("%012llx\n", h);
    return;
  }
  char mac[MAC_STR_SIZE];
  format_mac(h, mac);
  printf("%s\n", mac);
  if ((bits & BITS_UNICAST) == 0 && (h & BIT_MULTICAST)) {
    log_msg(PROG, LOG_WARN,
            "%s is a multicast MAC address (the low bit of the first "
            "octet is set). Multicast addresses are group addresses, "
            "not individual device addresses, so a host using one "
            "would have its traffic treated as group traffic. Re-run "
            "with -u/--unicast to clear the bit, or -p/--plain if you "
            "only need the raw hash.",
            mac);
  }
}

static void print_help(int detailed) {
  printf("machash %s - hash strings into MAC addresses\n\n", MACHASH_VERSION);
  printf(
      "Usage: machash [options] [string...]\n"
      "\n"
      "Hashes each input string with the 48-bit Bobcat hash and prints\n"
      "the result as a colon-separated MAC address, or as a raw hex\n"
      "number with -p/--plain. If no strings are given, all of stdin is\n"
      "read as one input (multi-line data stays a single input).\n"
      "\n"
      "Options:\n"
      "  -s, --string STR   Hash STR. May be repeated. (default: none)\n"
      "  -p, --plain        Print the raw 48-bit hash as 12 lowercase\n"
      "                     hex digits, no separators, no 0x prefix.\n"
      "                     (default: off)\n"
      "  -u, --unicast      Clear the multicast bit so the result is\n"
      "                     always a unicast MAC address. (default: off)\n"
      "  --local            Set the locally-administered bit of the\n"
      "                     result. (default: off)\n"
      "  -L, --loglevel LVL Log level: off, fatal, error, warn, info,\n"
      "                     debug, or 0-5. (default: warn)\n"
      "  -h, --help         Show this help and exit.\n"
      "  --version          Show the version and build info, and exit.\n"
      "\n"
      "Positional arguments:\n"
      "  string             Strings to hash, in addition to -s. Options\n"
      "                     are only recognized before the first string;\n"
      "                     after that, all arguments (dashed or not)\n"
      "                     are strings. A bare -- also ends option\n"
      "                     parsing.\n"
      "\n"
      "Examples:\n"
      "  echo hello | machash\n"
      "  machash -s hello\n"
      "  machash -pu -s hello\n"
      "  machash --local -s \"hello world\"\n"
      "  machash -L debug -s hello\n"
      "\n"
      "See docs/hash.md for the hash algorithm and docs/outputs.md for\n"
      "the output format and bit semantics.\n");
  if (detailed) {
    printf(
        "String inputs:\n"
        "  Strings come from -s/--string and from positional arguments,\n"
        "  in the order they appear on the command line. Each string is\n"
        "  hashed separately and one line is printed per string.\n"
        "  Leading and trailing whitespace (spaces, tabs, newlines)\n"
        "  is stripped before hashing; an input that is empty after\n"
        "  stripping is hashed as the empty string, with a warning.\n"
        "  Data piped on stdin is read whole, treated as a single\n"
        "  input, and stripped the same way. Inputs may contain any\n"
        "  bytes, including newlines in the middle of the string.\n");
  }
}

int main(int argc, char **argv) {
  static struct inputs inputs;
  int plain = 0, unicast = 0, local = 0, help = 0, version = 0;
  const opt_spec_t specs[] = {
      {.long_name = "string", .short_name = 's', .takes_value = 1,
       .cb = add_input, .user = &inputs},
      {.long_name = "plain", .short_name = 'p', .bool_dest = &plain},
      {.long_name = "unicast", .short_name = 'u', .bool_dest = &unicast},
      {.long_name = "local", .bool_dest = &local},
      {.long_name = "loglevel", .short_name = 'L', .takes_value = 1,
       .cb = set_loglevel},
      {.long_name = "help", .short_name = 'h', .bool_dest = &help},
      {.long_name = "version", .bool_dest = &version},
  };
  if (parse_opts(PROG, argc, argv, specs, 7, add_input, &inputs) != 0) {
    return 1;
  }
  // cppcheck-suppress knownConditionTrueFalse; set via parse_opts
  if (help) {
    print_help(inputs.count > 0);
    return 0;
  }
  // cppcheck-suppress knownConditionTrueFalse; set via parse_opts
  if (version) {
    printf("%s %s\n", PROG, MACHASH_VERSION);
    printf("build: %s\n", MACHASH_BUILD);
    printf("commit: %s\n", MACHASH_COMMIT);
    printf("build number: %s\n", MACHASH_BUILD_NUMBER);
    return 0;
  }

  char *stdin_buf = NULL;
  size_t stdin_len = 0;
  if (inputs.count == 0) {
    stdin_buf = read_stdin_all(&stdin_len);
    if (stdin_len == 0) {
      free(stdin_buf);
      log_error(PROG,
                "no input provided; use -s/--string or a positional "
                "argument, or pipe data on stdin; try '%s --help' "
                "for usage",
                PROG);
      return 1;
    }
  }

  output_fmt fmt = plain ? OUT_PLAIN : OUT_MAC;
  bit_ops bits = BITS_NONE;
  if (unicast) {
    bits |= BITS_UNICAST;
  }
  if (local) {
    bits |= BITS_LOCAL;
  }
  if (inputs.count > 0) {
    for (int i = 0; i < inputs.count; i++) {
      hash_input(inputs.items[i], strlen(inputs.items[i]), fmt, bits);
    }
  } else {
    hash_input(stdin_buf, stdin_len, fmt, bits);
  }
  free(stdin_buf);
  free(inputs.items);
  return 0;
}
