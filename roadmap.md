# Roadmap

Planned work for machash. Completed items move to changelog.md.

# Done

- 1.0.1 - packages for NixOS and Void Linux, and Makefile targets
  for the OS package builds (make packages). See changelog.md.
- 1.0.0 - man page, packages for Homebrew, Alpine, Debian, OpenSuSE,
  Fedora, and Arch, a reproducible source tarball (make dist), and
  the release workflow that publishes the assets. See changelog.md.
- 0.5.0 - deterministic fuzz harness for the argument parser and
  hash, UBSan in abort mode. Found and fixed an undefined left
  shift in the Bobcat key schedule. See changelog.md.
- 0.4.0 - hostname input mode (-n, --hostname) and per-interface
  MACs (-i, --interface). Test suite fix for expected failures.
  See changelog.md.
- 0.3.1 - fixed and simplified CI workflow. The test job was still
  broken; the cause was fixed in 0.4.0.
- 0.3.0 - line-oriented input mode (-l, -f), --0x and -S output
  formats, --check mode, build-essential in CI. See changelog.md.
- 0.2.0 - compliance release: enum parameters, named spec constants,
  null-terminated error text, simple-English docs, wider tests. See
  changelog.md.
- 0.1.0 - initial release: Bobcat hash, MAC/plain output, bit control
  flags, stdin and flag inputs, help and logging, tests, CI. See
  changelog.md.
