// Simple leveled logging to stderr.
#include "log.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int log_level = LOG_WARN;

static const char *level_names[] = {"off", "fatal", "error", "warning",
                                    "info", "debug"};

int parse_bool(const char *s) {
  if (!s || !*s) {
    return -1;
  }
  if (!strcasecmp(s, "yes") || !strcasecmp(s, "on") ||
      !strcasecmp(s, "true") || !strcmp(s, "1")) {
    return 1;
  }
  if (!strcasecmp(s, "no") || !strcasecmp(s, "off") ||
      !strcasecmp(s, "false") || !strcmp(s, "0")) {
    return 0;
  }
  return -1;
}

int parse_log_level(const char *s) {
  if (!s || !*s) {
    return -1;
  }
  // Single digit 0..5 selects the level numerically.
  if (s[0] >= '0' && s[0] <= '5' && !s[1]) {
    return s[0] - '0';
  }
  static const struct {
    const char *name;
    int level;
  } names[] = {
      {"off", LOG_OFF},    {"none", LOG_OFF},   {"fatal", LOG_FATAL},
      {"error", LOG_ERROR}, {"err", LOG_ERROR}, {"warn", LOG_WARN},
      {"warning", LOG_WARN}, {"info", LOG_INFO}, {"debug", LOG_DEBUG},
  };
  for (size_t i = 0; i < sizeof(names) / sizeof(names[0]); i++) {
    if (!strcasecmp(s, names[i].name)) {
      return names[i].level;
    }
  }
  return -1;
}

void log_msg(const char *prog, int level, const char *fmt, ...) {
  if (level > log_level) {
    return;
  }
  fprintf(stderr, "%s: %s: ", prog, level_names[level]);
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  va_end(ap);
  fputc('\n', stderr);
}

void log_abort(const char *prog, const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  fprintf(stderr, "%s: fatal: ", prog);
  vfprintf(stderr, fmt, ap);
  va_end(ap);
  fputc('\n', stderr);
  exit(1);
}
