# Changelog

All notable changes to machash are documented in this file.

The format is based on [Keep a Changelog][kacl]. This project adheres
to [Semantic Versioning][semver].

## [Unreleased]

## [0.1.0] - 2026-08-22

### Added

- 48-bit Bobcat hash of input strings (src/bobcat.c), validated
  against the four published reference vectors and an independent
  oracle (tests/ref/).
- MAC address output by default; plain 12-hex-digit output with
  `-p/--plain`.
- `-u/--unicast` clears the multicast bit; `--local` sets the
  locally-administered bit.
- Warning on stderr when the result is a multicast address and
  neither plain nor unicast mode was requested.
- Inputs from stdin (the whole stream as one input), `-s/--string`,
  and positional arguments, in command order; leading/trailing
  whitespace stripped.
- Robust option parser: clustered short flags, long flags with `=` or
  whitespace values, boolean values for flags, dashed arguments after
  the first positional treated as inputs, `--` terminator.
- Leveled logging (`-L/--loglevel` by name or number), help system
  with documented defaults and per-feature detail, `--version` with
  baked-in version, build toolchain, commit hash, and build number.
- Unit, oracle, and integration test suites; Makefile build/test/
  lint/install targets with an 80-column width check.
- Gitea-compatible CI: lint and test on every pull request, release
  with the binary as an asset on each `v*` tag.

[kacl]: https://keepachangelog.com/en/1.1.0/
[semver]: https://semver.org/spec/v2.0.0.html
