// Command-line option parser.
#include "args.h"

#include "log.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

static const opt_spec_t *find_long(const opt_spec_t *specs, int nspecs,
                                   const char *name, size_t len) {
  for (int i = 0; i < nspecs; i++) {
    if (specs[i].long_name && strlen(specs[i].long_name) == len &&
        !strncmp(specs[i].long_name, name, len)) {
      return &specs[i];
    }
  }
  return NULL;
}

static const opt_spec_t *find_short(const opt_spec_t *specs, int nspecs,
                                    char c) {
  for (int i = 0; i < nspecs; i++) {
    if (specs[i].short_name == c) {
      return &specs[i];
    }
  }
  return NULL;
}

// Reports an error with a hint to the help flag; returns -1.
// Errors bypass log_level: usage errors must stay visible.
static int fail(const char *prog, const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  fprintf(stderr, "%s: error: ", prog);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "; try '%s --help' for usage\n", prog);
  va_end(ap);
  return -1;
}

int parse_opts(const char *prog, int argc, char **argv,
               const opt_spec_t *specs, int nspecs,
               opt_cb_t on_positional, void *user) {
  int seen_positional = 0;
  for (int i = 1; i < argc; i++) {
    const char *a = argv[i];

    // -- ends option parsing; everything after is positional.
    if (!seen_positional && !strcmp(a, "--")) {
      seen_positional = 1;
      continue;
    }

    // Positional argument, including any dashed argument that follows
    // the first positional one.
    if (seen_positional || a[0] != '-' || a[1] == '\0') {
      if (on_positional && on_positional(a, user) != 0) {
        return -1;
      }
      seen_positional = 1;
      continue;
    }

    if (a[1] == '-') {
      // Long option, with an optional =value.
      const char *name = a + 2;
      const char *eq = strchr(name, '=');
      size_t len = eq ? (size_t)(eq - name) : strlen(name);
      const opt_spec_t *sp = find_long(specs, nspecs, name, len);
      if (!sp) {
        return fail(prog, "unknown option '%s'", a);
      }
      const char *val = eq ? eq + 1 : NULL;
      if (sp->takes_value) {
        if (!val) {
          if (i + 1 >= argc) {
            return fail(prog, "option '%s' requires a value", a);
          }
          val = argv[++i];
        }
        if (sp->cb && sp->cb(val, sp->user) != 0) {
          return -1;
        }
      } else if (val) {
        int b = parse_bool(val);
        if (b < 0) {
          return fail(prog,
                      "option '%s' has invalid boolean value "
                      "(expected yes/no/on/off/true/false/0/1)",
                      a);
        }
        *sp->bool_dest = b;
      } else {
        *sp->bool_dest = 1;
      }
      continue;
    }

    // Clustered short options: -abc. A value-taking option ends the
    // cluster; the rest of it (or the next argument) is its value.
    for (const char *p = a + 1; *p; p++) {
      const opt_spec_t *sp = find_short(specs, nspecs, *p);
      if (!sp) {
        char opt[3] = {'-', *p, 0};
        return fail(prog, "unknown option '%s'", opt);
      }
      if (sp->takes_value) {
        const char *val = p + 1;
        if (!*val) {
          if (i + 1 >= argc) {
            char opt[3] = {'-', *p, 0};
            return fail(prog, "option '%s' requires a value", opt);
          }
          val = argv[++i];
        }
        if (sp->cb && sp->cb(val, sp->user) != 0) {
          return -1;
        }
        break;
      }
      *sp->bool_dest = 1;
    }
  }
  return 0;
}
