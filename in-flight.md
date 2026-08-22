# In-flight work

Records work in progress so another agent can resume interrupted work.

## Current

- Scaffolding the repository: license, build system, progress docs.

## Notes for the next agent

- Toolchain: cosmocc 4.0.2 in ~/.local/bin, build with `make build`.
- Lint tools (shellcheck, cppcheck) are in ~/.local/bin; CI installs its own.
- Design: input strings are hashed with 48-bit Bobcat; output is a MAC
  address by default. See goals.md for the authoritative feature list.
