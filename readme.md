# machash - Hash arbitrary strings into MAC addresses

machash hashes input strings with the 48-bit Bobcat hash and prints
the result as a colon-separated MAC address:

    $ machash -s hello
    0f:c2:c1:58:42:59

The address is fake, but it has the form of a valid MAC address. It
is useful for deriving a stable, deterministic MAC address from any
string. With `-p/--plain` the tool prints a general 48-bit hash.

## Building

The build needs [cosmocc][cosmocc], the compiler for
[Cosmopolitan][cosmo], a C toolchain that emits single-file static
binaries running on many platforms. If it is not installed,
`make toolchain` runs tools/install-cosmocc.sh, which fetches the
pinned release zip, verifies it, and installs it under ~/.local.

    make build

The result is dist/machash. `make install` copies it to
~/.local/bin.

## Usage

    machash [options] [string...]

Strings come from `-s/--string` (repeatable) and from positional
arguments, in the order they appear. If you give no strings, machash
reads all of stdin as one input. Multi-line data stays a single
input. machash strips leading and trailing whitespace.

Line mode reads one input per line. `-l/--lines` reads the lines of
stdin. `-f/--file FILE` reads the lines of a file. Each line loses
its newline and is stripped like other inputs.

Hostname mode takes the input from the machine. `-n/--hostname`
uses the host's own name. `-i/--interface IFACE` uses the host name
and IFACE, joined by a colon. See docs/hostname.md for the details.

`--check MAC` compares the inputs to an existing MAC address and
prints match or no match per input. See docs/check.md for the
details.

Common invocations:

    echo hello | machash            # stdin
    machash -s hello                # flag input
    machash hello world             # positional inputs
    machash -pu -s hello            # plain, unicast
    machash --local -s "hello"      # locally-administered
    echo hello | machash -l         # stdin lines
    machash -f names.txt            # each line of a file
    machash -n                      # MAC from the host name
    machash -n -i eth0              # MAC from host + interface
    machash --0x -s hello           # 0x0fc2c1584259
    machash -S -s hello             # 59:42:58:c1:c2:0f
    machash --check 0f:c2:c1:58:42:59 hello   # match, exit 0

Run `machash --help` for the full option list with defaults. The
man page, `machash(1)`, covers the same ground.

## Options

| Option | Default | Effect |
|---|---|---|
| `-s`, `--string STR` | none | hash STR (repeatable) |
| `-p`, `--plain` | off | raw 12 hex digits, no separators or prefix |
| `-S`, `--swap` | off | print the six octets in reversed order |
| `--0x` | off | raw hash with a 0x prefix |
| `-u`, `--unicast` | off | clear the multicast bit |
| `--local` | off | set the locally-administered bit |
| `-l`, `--lines` | off | each line of stdin is one input |
| `-f`, `--file FILE` | none | each line of FILE is one input |
| `-n`, `--hostname` | off | the host's own name is the input |
| `-i`, `--interface IFACE` | none | host name and IFACE, colon-joined |
| `--check MAC` | off | print match or no match per input |
| `-L`, `--loglevel LVL` | warn | off/fatal/error/warn/info/debug, or 0-5 |
| `-h`, `--help` | - | show help text |
| `--version` | - | show version, build, commit, build number |

Output formats are mutually exclusive. `--check` is a mode and
cannot be combined with a format flag.

See docs/outputs.md for the bit semantics and the multicast warning,
docs/hostname.md for hostname mode, docs/check.md for check mode,
and docs/hash.md for the Bobcat algorithm details.

## Packaging

Packages for Homebrew, Alpine, Debian, OpenSuSE, Fedora, Arch,
NixOS, and Void Linux live in packaging/. Each one builds the
universal binary from the release source tarball, which
`make dist` produces. `make packages` builds all of them; see
docs/packaging.md for how the build works and how to build each
package.

## Testing and development

    make test    # unit + oracle + integration tests
    make lint    # cppcheck, shellcheck, 80-column check
    make fuzz    # deterministic fuzz harness (hash + arg parser)

Expected test values come from the independent Bobcat oracle in
tests/ref/. tests/ref/check_oracle.sh certifies the oracle against
the published vectors.

## License

The license is BSD 3-clause. See LICENSE. The dependencies and their
licenses are listed in dependencies.md.

---

This code and its documentation were partially written by a large
language model, then carefully planned and reviewed by a human.

[cosmo]: https://github.com/jart/cosmopolitan
[cosmocc]: https://cosmo.zip/pub/cosmocc/
