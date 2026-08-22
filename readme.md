# machash - Hash arbitrary strings in to MAC addresses

machash hashes input strings with the 48-bit Bobcat hash and prints
the result as a colon-separated MAC address:

    $ machash -s hello
    0f:c2:c1:58:42:59

The address is fake but structurally valid; it is useful for deriving
a stable, deterministic MAC address from any string. With
`-p/--plain` the tool doubles as a general 48-bit hash printer.

## Building

The build needs [cosmocc][cosmocc], the compiler for
[Cosmopolitan][cosmo], a C toolchain that emits single-file static
binaries running on many platforms:

    make build

The result is dist/machash. `make install` copies it to
~/.local/bin.

## Usage

    machash [options] [string...]

Strings come from `-s/--string` (repeatable) and from positional
arguments, in the order they appear. If no strings are given, all of
stdin is read as one input; multi-line data stays a single input, and
leading/trailing whitespace is stripped.

Common invocations:

    echo hello | machash            # stdin
    machash -s hello                # flag input
    machash hello world             # positional inputs
    machash -pu -s hello            # plain, unicast
    machash --local -s "hello"      # locally-administered

Run `machash --help` for the full option list with defaults.

## Options

| Option | Default | Effect |
|---|---|---|
| `-s`, `--string STR` | none | hash STR (repeatable) |
| `-p`, `--plain` | off | raw 12 hex digits, no separators or prefix |
| `-u`, `--unicast` | off | clear the multicast bit |
| `--local` | off | set the locally-administered bit |
| `-L`, `--loglevel LVL` | warn | off/fatal/error/warn/info/debug, or 0-5 |
| `-h`, `--help` | - | show help text |
| `--version` | - | show version, build, commit, build number |

See docs/outputs.md for the bit semantics and the multicast warning,
and docs/hash.md for the Bobcat algorithm details.

## Testing and development

    make test    # unit + oracle + integration tests
    make lint    # cppcheck, shellcheck, 80-column check

Expected test values are generated with the independent Bobcat oracle
in tests/ref/ (certified by tests/ref/check_oracle.sh).

## License

BSD 3-clause; see LICENSE. Dependencies and their licenses are listed
in dependencies.md.

---

This code and its documentation were partially written by a large
language model, then carefully planned and reviewed by a human.

[cosmo]: https://github.com/jart/cosmopolitan
[cosmocc]: https://cosmo.zip/pub/cosmocc/
