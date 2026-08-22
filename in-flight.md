# In-flight work

Records work in progress so another agent can resume interrupted work.

## Current

- Implementing the core tool: logging (src/log.c), argument parser
  (src/args.c), and the main program (src/machash.c).

## Notes for the next agent

- Toolchain: cosmocc 4.0.2 in ~/.local/bin, build with `make build`.
- Lint tools (shellcheck, cppcheck) are in ~/.local/bin; CI installs its own.
- Bobcat hash (src/bobcat.c) is done and validated against the four
  published reference vectors plus the independent oracle
  (tests/ref/, certified by tests/ref/check_oracle.sh).
- Expected hash values for common test strings are generated with
  `printf '%s' STR | build/bobcat_ref` (e.g. "hello" -> 0x0fc2c1584259,
  "" -> 0xe6be9fc25f39, "baz" -> 0x8ebeff920c64 which is multicast).
- Design: input strings are hashed with 48-bit Bobcat; output is a MAC
  address by default. See goals.md for the authoritative feature list.
