// Command-line option parser.
#ifndef ARGS_H
#define ARGS_H

// Called for each value of a value-taking option and for each
// positional argument. Return 0 to continue parsing, -1 to stop.
typedef int (*opt_cb_t)(const char *value, void *user);

typedef struct {
  const char *long_name;  // NULL if the option has no long form
  char short_name;        // 0 if the option has no short form
  int takes_value;        // 1: option requires a value
  int *bool_dest;         // for flags: stores 1 when the flag is seen
  opt_cb_t cb;            // for value options: receives the value
  void *user;             // passed through to cb
} opt_spec_t;

// Parses argv (argv[0] is the program name) against specs.
//
// Accepted forms:
//   -a, --word             boolean flags; short flags may be clustered
//   -aV, -a V              value attached or separated by whitespace
//   --word V, --word=V     long option value
//   --word yes|no|...      explicit boolean value for a flag
//
// Options are only recognized before the first positional argument;
// once one appears, all later arguments (dashed or not) are treated
// as positional. A bare -- ends option parsing immediately. A bare -
// is a positional argument.
//
// Errors are reported with log_msg() and -1 is returned.
int parse_opts(const char *prog, int argc, char **argv,
               const opt_spec_t *specs, int nspecs,
               opt_cb_t on_positional, void *user);

#endif
