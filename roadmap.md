# Roadmap

Planned work for machash. Completed items move to changelog.md.

## 0.3.0

- [ ] Line-oriented input mode: hash one line at a time from a file or stdin
- [ ] More output formats (0x prefix, byte-swapped octet order)
- [ ] `--check` mode to verify an existing MAC against a string

## Later

- [ ] Add `-n`, `--hostname` arguments to generate a MAC address
  from the hostname
- [ ] Add `-i`, `--interface` arguments to generate a MAC address
  from hostname + interface name
- [ ] Fuzz harness for the argument parser and hash
- [ ] Man page
- [ ] Packages for Homebrew, Debian, OpenSuSE, Fedora, and Arch

## Done

- 0.2.0 - compliance release: enum parameters, named spec constants,
  null-terminated error text, simple-English docs, wider tests. See
  changelog.md.
- 0.1.0 - initial release: Bobcat hash, MAC/plain output, bit control
  flags, stdin and flag inputs, help and logging, tests, CI. See
  changelog.md.
