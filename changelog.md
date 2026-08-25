# Changelog

All notable changes to machash are documented in this file.

The format is based on [Keep a Changelog][kacl]. This project adheres
to [Semantic Versioning][semver].

## [Unreleased]

## [0.5.0] - 2026-08-25

### Added

- Fuzz harness for the argument parser and the Bobcat hash
  (tests/fuzz/). cosmocc has no libFuzzer mode, so the harness is a
  small deterministic, splitmix64-driven generator with libFuzzer
  style -runs/-seed flags. Both targets build with UBSan in abort
  mode. `make fuzz` runs 100000 inputs per target, and the CI test
  job runs a 20000-input session.

### Changed

- MAC address parsing moved from src/machash.c to a small module,
  src/mac.c (mac_parse), so the fuzz target exercises the real
  validator.

### Fixed

- Undefined behavior in the Bobcat key schedule: (~x[1]) << 5 and
  (~x[7]) << 5 left-shift a negative int. The new fuzz harness found
  it on its first run. The words are now masked to 16 bits before
  the shift; the digest is unchanged (all published vectors and all
  integration expectations still match).

## [0.4.0] - 2026-08-25

### Added

- Hostname input mode: `-n/--hostname` uses the name of this host
  (gethostname) as the single input. `-i/--interface IFACE` uses the
  host name and IFACE, joined by a colon, as the single input, so
  one command gives one stable MAC per host and per interface.
  Hostname mode is exclusive: it cannot be combined with -s,
  positional arguments, or line mode, and stdin is ignored.
- docs/hostname.md documents the new mode.

### Fixed

- The test suite aborts at the first expected failure. The 0.3.1
  switch to set -Eeuo pipefail made the capture helpers fatal on
  nonzero exit codes, and set -E is not valid under a #!/bin/sh
  shebang. Both scripts now use bash, and the capture helpers
  record exit codes instead of aborting. The CI test job failed on
  this for the same reason.
- The --version expectation in the integration suite still said
  0.3.0 after the 0.3.1 release.

## [0.3.1] - 2026-08-24

### Changed

- Updated CI/CD pipeline with consolidation and fixes. Still broken in Github.

## [0.3.0] - 2026-08-24

### Added

- Line-oriented input mode: -l/--lines reads each line of stdin as
  one input. -f/--file reads each line of a file. Each line loses
  its newline and is stripped like other inputs.
- Output formats: --0x prints the hash with a 0x prefix.
  -S/--swap prints the six octets in reversed order.
- --check MAC mode: compares each input to MAC (after applying
  -u/--local) and prints match or no match per input. The exit
  status is 0 if all inputs match, 1 otherwise.
- CI workflows install build-essential in the action environment.
  The runners failed on make before this fix.

### Changed

- The multicast warning no longer names the first octet, so it
  stays correct for swapped output. It applies to both address-like
  formats (MAC and swapped) and is suppressed for the raw formats
  (-p, --0x) as before.

## [0.2.0] - 2026-08-24

### Changed

- `hash_input()` takes an output-format enum and a bit-operation
  bitmask instead of three boolean parameters.
- Spec values and recurring values are named constants in
  src/bobcat.c and src/machash.c.
- Integration tests cover inputs past the initial buffer capacity
  and whitespace-padded flag values.
- agents.md is wrapped to 80 columns. Before this fix, `make lint`
  failed on the file.
- The markdown documentation follows simple-English rules: sentences
  under 25 words, no semicolons, no passive voice with a known
  agent.

### Fixed

- The error text for an unknown short option printed a stray stack
  byte after the option name in some builds. The same defect
  affected the missing-value message. The option-name buffer is now
  null-terminated.

## [0.1.0] - 2026-08-22

### Added

- 48-bit Bobcat hash of input strings (src/bobcat.c), validated
  against the four published reference vectors and an independent
  oracle (tests/ref/).
- MAC address output by default. Plain 12-hex-digit output with
  `-p/--plain`.
- `-u/--unicast` clears the multicast bit. `--local` sets the
  locally-administered bit.
- Warning on stderr when the result is a multicast address and
  neither plain nor unicast mode was requested.
- Inputs from stdin (the whole stream as one input), `-s/--string`,
  and positional arguments, in command order. machash strips leading
  and trailing whitespace.
- Option parser with clustered short flags, long flags, and `=` or
  whitespace value separators. Flags accept boolean values. Dashed
  arguments after the first positional are inputs. A bare `--` ends
  option parsing.
- Leveled logging (`-L/--loglevel` by name or number). Help with
  documented defaults and per-feature detail. `--version` shows the
  baked-in version, build toolchain, commit hash, and build number.
- Unit, oracle, and integration test suites. Makefile targets for
  build, test, lint, and install, with an 80-column width check.
- Gitea-compatible CI: lint and test on every pull request, release
  with the binary as an asset on each `v*` tag.

[kacl]: https://keepachangelog.com/en/1.1.0/
[semver]: https://semver.org/spec/v2.0.0.html
