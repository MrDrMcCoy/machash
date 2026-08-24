// machash: hash arbitrary strings into MAC addresses.
//
// Each input string is hashed with the 48-bit Bobcat hash and printed
// as a colon-separated MAC address by default, or as raw hex with
// -p/--plain or --0x, or with reversed octets by -S/--swap. Inputs
// come from -s/--string and positional arguments; with no inputs, all
// of stdin is one input. Line mode (-l/--lines, -f/--file) hashes one
// line at a time. Check mode (--check MAC) compares inputs against
// an existing MAC address.

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
// Number of octets in a MAC address.
#define MAC_OCTETS 6

// Output formats for the digest.
typedef enum {
  OUT_MAC,     // colon-separated MAC address
  OUT_PLAIN,   // raw 12 hex digits
  OUT_HEX0X,   // 0x prefix plus 12 hex digits
  OUT_SWAPPED, // octets in reversed order
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

// Options that need a callback to store their value.
struct opt_state {
  const char *file;
  int check_enabled;
  unsigned long long check_value;
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

static int set_file(const char *v, void *user) {
  struct opt_state *st = user;
  st->file = v;
  return 0;
}

// Returns the value of a hex digit, or -1.
static int hexval(char c) {
  if (c >= '0' && c <= '9') {
    return c - '0';
  }
  if (c >= 'a' && c <= 'f') {
    return c - 'a' + 10;
  }
  if (c >= 'A' && c <= 'F') {
    return c - 'A' + 10;
  }
  return -1;
}

// Parse a MAC address of the form aa:bb:cc:dd:ee:ff. Returns 0 and
// stores the 48-bit value in out, or returns -1.
static int parse_mac(const char *s, unsigned long long *out) {
  unsigned long long v = 0;
  const char *p = s;
  for (int i = 0; i < MAC_OCTETS; i++) {
    int hi = hexval(p[0]);
    int lo = hexval(p[1]);
    if (hi < 0 || lo < 0) {
      return -1;
    }
    v = (v << 8) | (unsigned long long)(hi << 4) | (unsigned long long)lo;
    p += 2;
    if (i < MAC_OCTETS - 1) {
      if (*p != ':') {
        return -1;
      }
      p++;
    }
  }
  if (*p != 0) {
    return -1;
  }
  *out = v;
  return 0;
}

static int set_check(const char *v, void *user) {
  struct opt_state *st = user;
  if (parse_mac(v, &st->check_value) != 0) {
    log_error(PROG,
              "invalid MAC address '%s' (expected six colon-separated "
              "hex octets, for example 0f:c2:c1:58:42:59); try '%s "
              "--help' for usage",
              v, PROG);
    return -1;
  }
  st->check_enabled = 1;
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

// Read f line by line and add each line (without its newline) to the
// input list. Returns the number of lines read; aborts on error.
static int read_lines(FILE *f, struct inputs *in) {
  char *line = NULL;
  size_t cap = 0;
  ssize_t n;
  int count = 0;
  while ((n = getline(&line, &cap, f)) != -1) {
    if (n > 0 && line[n - 1] == '\n') {
      line[n - 1] = 0;
    }
    const char *copy = strdup(line);
    if (!copy) {
      log_abort(PROG, "out of memory reading a line");
    }
    add_input(copy, in);
    count++;
  }
  free(line);
  if (ferror(f)) {
    log_abort(PROG, "failed to read lines: %s", strerror(errno));
  }
  return count;
}

// Format h as a colon-separated MAC address; out needs MAC_STR_SIZE
// bytes.
static void format_mac(unsigned long long h, char *out) {
  char *p = out;
  for (int i = MAC_OCTETS - 1; i >= 0; i--) {
    if (i != MAC_OCTETS - 1) {
      *p++ = ':';
    }
    p += sprintf(p, "%02llx", (h >> (8 * i)) & 0xff);
  }
  *p = 0;
}

// Format the digest into out (MAC_STR_SIZE bytes) according to fmt.
static void format_digest(unsigned long long h, output_fmt fmt, char *out) {
  if (fmt == OUT_PLAIN) {
    sprintf(out, "%012llx", h);
    return;
  }
  if (fmt == OUT_HEX0X) {
    sprintf(out, "0x%012llx", h);
    return;
  }
  char mac[MAC_STR_SIZE];
  format_mac(h, mac);
  if (fmt == OUT_SWAPPED) {
    for (int i = 0; i < MAC_OCTETS; i++) {
      int src = (MAC_OCTETS - 1 - i) * 3;
      out[i * 3] = mac[src];
      out[i * 3 + 1] = mac[src + 1];
      out[i * 3 + 2] = (i == MAC_OCTETS - 1) ? 0 : ':';
    }
    return;
  }
  strcpy(out, mac);
}

// Hash one raw input and apply the bit operations.
static unsigned long long digest_input(const char *raw, size_t raw_len,
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
  return h;
}

static void hash_input(const char *raw, size_t raw_len, output_fmt fmt,
                       bit_ops bits) {
  unsigned long long h = digest_input(raw, raw_len, bits);
  char out[MAC_STR_SIZE];
  format_digest(h, fmt, out);
  printf("%s\n", out);
  if ((fmt == OUT_MAC || fmt == OUT_SWAPPED) &&
      (bits & BITS_UNICAST) == 0 && (h & BIT_MULTICAST)) {
    log_msg(PROG, LOG_WARN,
            "%s is a multicast MAC address. Multicast addresses are "
            "group addresses, not device addresses. A host that uses "
            "one has its traffic treated as group traffic. Re-run "
            "with -u/--unicast to clear the bit, or -p/--plain if "
            "you only need the raw hash.",
            out);
  }
}

// Compare one raw input against the MAC. Returns 1 on a match.
static int check_input(const char *raw, size_t raw_len,
                       unsigned long long mac, bit_ops bits) {
  unsigned long long h = digest_input(raw, raw_len, bits);
  int match = (h == mac);
  printf("%s\n", match ? "match" : "no match");
  return match;
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
      "  -S, --swap        Print the six octets in reversed order.\n"
      "                     (default: off)\n"
      "  --0x              Print the raw hash with a 0x prefix.\n"
      "                     (default: off)\n"
      "  -u, --unicast      Clear the multicast bit so the result is\n"
      "                     always a unicast MAC address. (default: off)\n"
      "  --local            Set the locally-administered bit of the\n"
      "                     result. (default: off)\n"
      "  -l, --lines        Read stdin as lines. Each line is one\n"
      "                     input. (default: off)\n"
      "  -f, --file FILE    Read FILE as lines. Each line is one\n"
      "                     input. (default: none)\n"
      "  --check MAC       Print match or no match for each input,\n"
      "                     compared against MAC. (default: off)\n"
      "  -L, --loglevel LVL Log level: off, fatal, error, warn, info,\n"
      "                     debug, or 0-5. (default: warn)\n"
      "  -h, --help         Show this help and exit.\n"
      "  --version          Show the version and build info, and exit.\n"
      "\n"
      "Output formats are mutually exclusive: the default MAC form,\n"
      "-p/--plain, --0x, and -S/--swap. --check is a mode and cannot\n"
      "be combined with a format flag.\n"
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
        "  bytes, including newlines in the middle of the string.\n"
        "\n"
        "Line input:\n"
        "  -l/--lines reads each line of stdin as one input.\n"
        "  -f/--file reads each line of FILE as one input.\n"
        "  A line loses its newline, and leading and trailing\n"
        "  whitespace is stripped as for other inputs. A line that is\n"
        "  empty after stripping hashes the empty string, with a\n"
        "  warning. Line mode cannot be combined with -s or\n"
        "  positional arguments, and -l and -f cannot be combined.\n"
        "\n"
        "Check mode:\n"
        "  --check MAC compares each input to MAC after applying\n"
        "  -u/--local, as in normal output mode. It prints match or\n"
        "  no match, one line per input, in input order. The exit\n"
        "  status is 0 if all inputs match, 1 if any input does not\n"
        "  match. The multicast warning is not printed in check mode.\n");
  }
}

int main(int argc, char **argv) {
  static struct inputs inputs;
  static struct opt_state state;
  int plain = 0, unicast = 0, local = 0, help = 0, version = 0;
  int lines = 0, hex0x = 0, swap = 0;
  const opt_spec_t specs[] = {
      {.long_name = "string", .short_name = 's', .takes_value = 1,
       .cb = add_input, .user = &inputs},
      {.long_name = "plain", .short_name = 'p', .bool_dest = &plain},
      {.long_name = "0x", .bool_dest = &hex0x},
      {.long_name = "swap", .short_name = 'S', .bool_dest = &swap},
      {.long_name = "unicast", .short_name = 'u', .bool_dest = &unicast},
      {.long_name = "local", .bool_dest = &local},
      {.long_name = "lines", .short_name = 'l', .bool_dest = &lines},
      {.long_name = "file", .short_name = 'f', .takes_value = 1,
       .cb = set_file, .user = &state},
      {.long_name = "check", .takes_value = 1, .cb = set_check,
       .user = &state},
      {.long_name = "loglevel", .short_name = 'L', .takes_value = 1,
       .cb = set_loglevel},
      {.long_name = "help", .short_name = 'h', .bool_dest = &help},
      {.long_name = "version", .bool_dest = &version},
  };
  if (parse_opts(PROG, argc, argv, specs, 12, add_input, &inputs) != 0) {
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

  // cppcheck-suppress knownConditionTrueFalse; set via parse_opts
  if (lines && state.file != NULL) {
    log_error(PROG,
              "only one of --lines or --file can be given; try '%s "
              "--help' for usage",
              PROG);
    return 1;
  }
  int line_mode = lines || state.file != NULL;
  if (line_mode && inputs.count > 0) {
    log_error(PROG,
              "line mode cannot be combined with -s or positional "
              "arguments; try '%s --help' for usage",
              PROG);
    return 1;
  }

  char *stdin_buf = NULL;
  size_t stdin_len = 0;
  if (line_mode) {
    FILE *f = stdin;
    if (state.file != NULL) {
      f = fopen(state.file, "r");
      if (!f) {
        log_error(PROG,
                  "cannot open file '%s': %s; try '%s --help' for "
                  "usage",
                  state.file, strerror(errno), PROG);
        return 1;
      }
    }
    read_lines(f, &inputs);
    if (state.file != NULL) {
      fclose(f);
    }
    if (inputs.count == 0) {
      log_error(PROG,
                "no input provided; pipe lines on stdin or use "
                "-f/--file; try '%s --help' for usage",
                PROG);
      return 1;
    }
  } else if (inputs.count == 0) {
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

  int nfmt = plain + hex0x + swap;
  if (nfmt > 1) {
    log_error(PROG,
              "only one output format can be selected; try '%s --help' "
              "for usage",
              PROG);
    return 1;
  }
  if (state.check_enabled && nfmt > 0) {
    log_error(PROG,
              "--check cannot be combined with an output format flag; "
              "try '%s --help' for usage",
              PROG);
    return 1;
  }
  output_fmt fmt = OUT_MAC;
  if (plain) {
    fmt = OUT_PLAIN;
  } else if (hex0x) {
    fmt = OUT_HEX0X;
  } else if (swap) {
    fmt = OUT_SWAPPED;
  }
  bit_ops bits = BITS_NONE;
  if (unicast) {
    bits |= BITS_UNICAST;
  }
  if (local) {
    bits |= BITS_LOCAL;
  }
  int all_match = 1;
  if (state.check_enabled) {
    if (inputs.count > 0) {
      for (int i = 0; i < inputs.count; i++) {
        all_match =
            check_input(inputs.items[i], strlen(inputs.items[i]),
                        state.check_value, bits) &&
            all_match;
      }
    } else {
      all_match =
          check_input(stdin_buf, stdin_len, state.check_value, bits) &&
          all_match;
    }
  } else if (inputs.count > 0) {
    for (int i = 0; i < inputs.count; i++) {
      hash_input(inputs.items[i], strlen(inputs.items[i]), fmt, bits);
    }
  } else {
    hash_input(stdin_buf, stdin_len, fmt, bits);
  }
  if (line_mode) {
    // Line mode owns its input strings; free each one.
    for (int i = 0; i < inputs.count; i++) {
      free((void *)inputs.items[i]);
    }
  }
  free(stdin_buf);
  free(inputs.items);
  return state.check_enabled && !all_match ? 1 : 0;
}
