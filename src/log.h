// Simple leveled logging to stderr.
#ifndef LOG_H
#define LOG_H

// Log levels, most to least severe.
#define LOG_OFF 0
#define LOG_FATAL 1
#define LOG_ERROR 2
#define LOG_WARN 3
#define LOG_INFO 4
#define LOG_DEBUG 5

// Current level; messages with level <= log_level are printed.
extern int log_level;

// Returns 1 or 0 if s is a recognized boolean string (yes/no/on/off/
// true/false/1/0, case-insensitive), else -1.
int parse_bool(const char *s);

// Returns a level 0..5 if s is a level name (off/fatal/error/warn/
// info/debug, case-insensitive, plus none/err/warning aliases) or a
// number 0..5, else -1.
int parse_log_level(const char *s);

// Prints "prog: level: message" to stderr if level <= log_level.
void log_msg(const char *prog, int level, const char *fmt, ...);

// Prints "prog: error: message" to stderr regardless of log_level.
// Usage errors must stay visible even when logging is turned off.
void log_error(const char *prog, const char *fmt, ...);

// Prints "prog: fatal: message" to stderr and exits with status 1.
void log_abort(const char *prog, const char *fmt, ...)
    __attribute__((noreturn, format(printf, 2, 3)));

#endif
