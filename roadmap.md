# Roadmap

Planned work for machash. Completed items move to changelog.md.

## Later

- [ ] Add `-n`, `--hostname` arguments to generate a MAC address
  from the hostname
- [ ] Add `-i`, `--interface` arguments to generate a MAC address
  from hostname + interface name
- [ ] Fuzz harness for the argument parser and hash
- [ ] Man page
- [ ] Packages for Homebrew, Debian, OpenSuSE, Fedora, and Arch
- [ ] Binary as Single-file, multi-arch OCI container, pushed to
  Github Packages as part of automated builds

## Done

- 0.3.0 - line-oriented input mode (-l, -f), --0x and -S output
  formats, --check mode, build-essential in CI. See changelog.md.
- 0.2.0 - compliance release: enum parameters, named spec constants,
  null-terminated error text, simple-English docs, wider tests. See
  changelog.md.
- 0.1.0 - initial release: Bobcat hash, MAC/plain output, bit control
  flags, stdin and flag inputs, help and logging, tests, CI. See
  changelog.md.
