// Unit tests for the option parser.
#include "args.h"
#include "log.h"
#include "test.h"

#include <string.h>

static const char *strings[8];
static int nstrings;
static const char *positional[8];
static int npositional;
static int plain, unicast, local, help;

static int add_string(const char *v, void *u) {
  (void)u;
  if (nstrings < 8) {
    strings[nstrings++] = v;
  }
  return 0;
}

static int add_pos(const char *v, void *u) {
  (void)u;
  if (npositional < 8) {
    positional[npositional++] = v;
  }
  return 0;
}

static const opt_spec_t specs[] = {
    {.long_name = "string", .short_name = 's', .takes_value = 1,
     .cb = add_string},
    {.long_name = "plain", .short_name = 'p', .bool_dest = &plain},
    {.long_name = "unicast", .short_name = 'u', .bool_dest = &unicast},
    {.long_name = "local", .bool_dest = &local},
    {.long_name = "help", .short_name = 'h', .bool_dest = &help},
};

static void reset(void) {
  nstrings = npositional = plain = unicast = local = help = 0;
}

// cppcheck-suppress unusedFunction; called with compound literals
static int run(char **argv) {
  int argc = 0;
  while (argv[argc]) {
    argc++;
  }
  return parse_opts("t", argc, argv, specs, 5, add_pos, NULL);
}

int main(void) {
  char *t = "t";
  log_level = LOG_OFF;  // keep error-path output quiet

  // Bare flags, short and long.
  reset();
  T_CHECK(run((char *[]){t, "-p", NULL}) == 0 && plain && !unicast);
  reset();
  T_CHECK(run((char *[]){t, "--plain", NULL}) == 0 && plain);
  reset();
  T_CHECK(run((char *[]){t, "-u", "--local", NULL}) == 0 && unicast && local);

  // Clustered short flags.
  reset();
  T_CHECK(run((char *[]){t, "-pu", NULL}) == 0 && plain && unicast);

  // Value forms: separated, attached, =.
  reset();
  T_CHECK(run((char *[]){t, "-s", "foo", NULL}) == 0 && nstrings == 1 &&
          !strcmp(strings[0], "foo"));
  reset();
  T_CHECK(run((char *[]){t, "-sfoo", NULL}) == 0 && nstrings == 1 &&
          !strcmp(strings[0], "foo"));
  reset();
  T_CHECK(run((char *[]){t, "--string=foo", NULL}) == 0 && nstrings == 1 &&
          !strcmp(strings[0], "foo"));
  reset();
  T_CHECK(run((char *[]){t, "--string", "foo", NULL}) == 0 && nstrings == 1 &&
          !strcmp(strings[0], "foo"));
  // = inside a value is preserved.
  reset();
  T_CHECK(run((char *[]){t, "--string=a=b", NULL}) == 0 && nstrings == 1 &&
          !strcmp(strings[0], "a=b"));
  // Explicit empty value.
  reset();
  T_CHECK(run((char *[]){t, "--string=", NULL}) == 0 && nstrings == 1 &&
          !strcmp(strings[0], ""));

  // Boolean values for flags.
  reset();
  T_CHECK(run((char *[]){t, "--plain=true", NULL}) == 0 && plain == 1);
  reset();
  T_CHECK(run((char *[]){t, "--plain=OFF", NULL}) == 0 && plain == 0);
  reset();
  T_CHECK(run((char *[]){t, "--plain=1", NULL}) == 0 && plain == 1);
  reset();
  T_CHECK(run((char *[]){t, "--plain=0", NULL}) == 0 && plain == 0);
  reset();
  T_CHECK(run((char *[]){t, "--plain=maybe", NULL}) == -1);

  // A value-taking option ends the cluster.
  reset();
  T_CHECK(run((char *[]){t, "-ps", "foo", NULL}) == 0 && plain &&
          nstrings == 1 && !strcmp(strings[0], "foo"));
  reset();
  T_CHECK(run((char *[]){t, "-psfoo", NULL}) == 0 && plain &&
          nstrings == 1 && !strcmp(strings[0], "foo"));

  // Positional arguments.
  reset();
  T_CHECK(run((char *[]){t, "a", "b", NULL}) == 0 && npositional == 2 &&
          !strcmp(positional[0], "a") && !strcmp(positional[1], "b"));

  // Dashed arguments after the first positional are positional.
  reset();
  T_CHECK(run((char *[]){t, "a", "-p", NULL}) == 0 && npositional == 2 &&
          !strcmp(positional[1], "-p") && !plain);

  // -- ends option parsing.
  reset();
  T_CHECK(run((char *[]){t, "-p", "--", "-s", NULL}) == 0 && plain &&
          npositional == 1 && !strcmp(positional[0], "-s"));

  // A bare - is a positional argument.
  reset();
  T_CHECK(run((char *[]){t, "-", NULL}) == 0 && npositional == 1 &&
          !strcmp(positional[0], "-"));

  // Repeated options accumulate.
  reset();
  T_CHECK(run((char *[]){t, "-s", "a", "-s", "b", NULL}) == 0 &&
          nstrings == 2 && !strcmp(strings[0], "a") &&
          !strcmp(strings[1], "b"));

  // Errors.
  reset();
  T_CHECK(run((char *[]){t, "--bogus", NULL}) == -1);
  reset();
  T_CHECK(run((char *[]){t, "-z", NULL}) == -1);
  reset();
  T_CHECK(run((char *[]){t, "-s", NULL}) == -1);
  reset();
  T_CHECK(run((char *[]){t, "--string", NULL}) == -1);
  reset();
  T_CHECK(run((char *[]){t, "--=x", NULL}) == -1);

  T_SUMMARY("unit_args");
}
