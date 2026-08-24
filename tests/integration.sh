#!/bin/sh
# Integration tests for the machash binary.
#
# Expected values were generated with the independent Bobcat oracle
# (tests/ref/check_oracle.sh) and hard-coded below.
set -u

BIN=dist/machash
if [ ! -x "$BIN" ]; then
  echo "integration: missing $BIN; run 'make build' first" >&2
  exit 1
fi

pass=0
fail=0
R_OUT=
R_ERR=
R_CODE=0

run() {
  R_ERR_FILE=$(mktemp) || exit 1
  R_OUT=$("$BIN" "$@" 2>"$R_ERR_FILE")
  R_CODE=$?
  R_ERR=$(cat "$R_ERR_FILE")
  rm -f "$R_ERR_FILE"
}

feed() {
  input=$1
  shift
  R_ERR_FILE=$(mktemp) || exit 1
  R_OUT=$(printf '%s' "$input" | "$BIN" "$@" 2>"$R_ERR_FILE")
  R_CODE=$?
  R_ERR=$(cat "$R_ERR_FILE")
  rm -f "$R_ERR_FILE"
}

check_out() {
  if [ "$R_OUT" = "$2" ]; then
    pass=$((pass + 1))
  else
    fail=$((fail + 1))
    echo "FAIL: $1 (stdout)" >&2
    printf '  want: %s\n  got:  %s\n' "$2" "$R_OUT" >&2
  fi
}

check_outc() {
  if printf '%s' "$R_OUT" | grep -qF -- "$2"; then
    pass=$((pass + 1))
  else
    fail=$((fail + 1))
    echo "FAIL: $1 (stdout missing: $2)" >&2
    printf '  stdout: %s\n' "$R_OUT" >&2
  fi
}

check_code() {
  if [ "$R_CODE" -eq "$2" ]; then
    pass=$((pass + 1))
  else
    fail=$((fail + 1))
    echo "FAIL: $1 (exit code want=$2 got=$R_CODE)" >&2
  fi
}

check_err() {
  if printf '%s' "$R_ERR" | grep -qF -- "$2"; then
    pass=$((pass + 1))
  else
    fail=$((fail + 1))
    echo "FAIL: $1 (stderr missing: $2)" >&2
    printf '  stderr: %s\n' "$R_ERR" >&2
  fi
}

check_no_err() {
  if printf '%s' "$R_ERR" | grep -qF -- "$2"; then
    fail=$((fail + 1))
    echo "FAIL: $1 (stderr should not contain: $2)" >&2
    printf '  stderr: %s\n' "$R_ERR" >&2
  else
    pass=$((pass + 1))
  fi
}

# --- stdin ---
feed 'hello'
check_out "stdin single line" '0f:c2:c1:58:42:59'
check_code "stdin single line" 0
check_no_err "stdin single line" 'warning'

feed 'hello
'
check_out "stdin trailing newline stripped" '0f:c2:c1:58:42:59'
check_code "stdin trailing newline stripped" 0

feed '  hello
  '
check_out "stdin surrounding whitespace stripped" '0f:c2:c1:58:42:59'

feed 'hello
world
'
check_out "stdin multi-line is one input" 'a5:b6:45:79:85:95'
check_code "stdin multi-line is one input" 0
check_err "stdin multi-line is one input" 'multicast MAC address'

feed '   '
check_out "stdin whitespace-only" 'e6:be:9f:c2:5f:39'
check_code "stdin whitespace-only" 0
check_err "stdin whitespace-only" 'empty after stripping'

feed ''
check_code "stdin empty" 1
check_err "stdin empty" 'no input provided'

# --- -s and positional inputs ---
run -s hello
check_out "flag -s" '0f:c2:c1:58:42:59'
check_code "flag -s" 0

run --string hello
check_out "flag --string" '0f:c2:c1:58:42:59'

run --string=hello
check_out "flag --string=" '0f:c2:c1:58:42:59'

run -shello
check_out "flag -s attached value" '0f:c2:c1:58:42:59'

run hello
check_out "positional" '0f:c2:c1:58:42:59'

run -s a -s b c
check_out "inputs in command order" "b4:d6:87:2d:49:9f
00:77:60:99:ed:a1
7c:14:5d:b2:d3:de"
check_code "inputs in command order" 0

run b -s c
check_out "option-like arg after positional is input" "00:77:60:99:ed:a1
ba:03:f0:99:15:3f
7c:14:5d:b2:d3:de"

run -s 'longer-than-sixteen-bytes!!'
check_out "multi-block input" '59:4d:5a:d1:9d:82'

run -s ''
check_out "empty string input" 'e6:be:9f:c2:5f:39'
check_err "empty string input" 'empty after stripping'

# --- output modes ---
run -p -s hello
check_out "plain" '0fc2c1584259'
check_code "plain" 0
check_no_err "plain" 'warning'

run --plain=true -s hello
check_out "plain =true" '0fc2c1584259'

run --plain=off -s hello
check_out "plain =off" '0f:c2:c1:58:42:59'

run -u -s baz
check_out "unicast clears multicast bit" '0e:be:ff:92:0c:64'
check_code "unicast clears multicast bit" 0
check_no_err "unicast clears multicast bit" 'warning'

run -s baz
check_out "multicast result" '8e:be:ff:92:0c:64'
check_code "multicast result" 0
check_err "multicast result" 'multicast MAC address'
check_err "multicast result" '-u/--unicast'

run --local -s hello
check_out "local bit set" '4f:c2:c1:58:42:59'
check_no_err "local bit set" 'warning'

run --local -s baz
check_out "local does not silence multicast warning" 'ce:be:ff:92:0c:64'
check_err "local does not silence multicast warning" 'multicast MAC address'

run -u -s y
check_out "unicast keeps local bit" '43:12:98:dd:21:b9'
check_no_err "unicast keeps local bit" 'warning'

run -pu -s baz
check_out "clustered flags with plain" '0ebeff920c64'
check_code "clustered flags with plain" 0

# --- option parsing rules ---
run -- -p
check_out "-- terminator" '61:02:d2:98:ff:2f'
check_code "-- terminator" 0

run a -b
check_out "dashed args after positional are inputs" "b4:d6:87:2d:49:9f
7b:a8:41:ba:0c:77"

run -
check_out "bare dash is an input" 'ec:ee:68:b1:f3:7a'

run -s hello --help
check_code "help still active after option values" 0
check_outc "help still active after option values" 'Usage: machash'

run hello --help
check_out "--help after positional is an input" "0f:c2:c1:58:42:59
58:48:42:be:b9:2d"

# --- errors ---
run --bogus
check_code "unknown long option" 1
check_err "unknown long option" "unknown option '--bogus'"
check_err "unknown long option" '--help'

run -z
check_code "unknown short option" 1
check_err "unknown short option" "unknown option '-z'"
# The closing quote must follow the option name directly. A stray byte
# in the name (an unterminated buffer) would break this match.
check_err "unknown short option" "-z'; try"

run -s
check_code "missing value" 1
check_err "missing value" 'requires a value'
check_err "missing value" "-s' requires"

run --string
check_code "missing value (long)" 1
check_err "missing value (long)" 'requires a value'

run --plain=maybe
check_code "invalid boolean value" 1
check_err "invalid boolean value" 'invalid boolean value'

run -L bogus -s x
check_code "invalid log level" 1
check_err "invalid log level" 'invalid log level'

# --- logging ---
run -L debug -s hello
check_code "debug level" 0
check_out "debug level" '0f:c2:c1:58:42:59'
check_err "debug level" 'debug'

run -L 5 -s hello
check_err "numeric level" 'debug'

run -L off -s baz
check_code "off level" 0
check_out "off level" '8e:be:ff:92:0c:64'
check_no_err "off level" 'warning'

# --- help and version ---
run -h
check_code "help" 0
check_outc "help" 'Usage: machash'
check_outc "help" '-p, --plain'
check_outc "help" '--local'
check_outc "help" '-L, --loglevel'
check_outc "help" '(default: warn)'

run --help x
check_code "help with inputs" 0
check_outc "help with inputs" 'String inputs:'

run --version
check_code "version" 0
check_outc "version" 'machash 0.1.0'
check_outc "version" 'build:'
check_outc "version" 'commit:'
check_outc "version" 'build number:'

echo "integration: $pass passed, $fail failed"
[ "$fail" -eq 0 ]
