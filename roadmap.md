# Roadmap

Planned work for machash. Completed items move to changelog.md.

## 0.4.0

- [ ] Add `-n`, `--hostname` arguments to generate a MAC address
  from the hostname
- [ ] Add `-i`, `--interface` arguments to generate a MAC address
  from hostname + interface name

## 0.5.0

- [ ] Fuzz harness for the argument parser and hash

## 1.0.0

- [ ] Man page
- [ ] Packages for Homebrew, Alpine, Debian, OpenSuSE, Fedora, and Arch

# Done

- 0.3.1 - fixed and simplified CI workflow. Still broken.
- 0.3.0 - line-oriented input mode (-l, -f), --0x and -S output
  formats, --check mode, build-essential in CI. See changelog.md.
- 0.2.0 - compliance release: enum parameters, named spec constants,
  null-terminated error text, simple-English docs, wider tests. See
  changelog.md.
- 0.1.0 - initial release: Bobcat hash, MAC/plain output, bit control
  flags, stdin and flag inputs, help and logging, tests, CI. See
  changelog.md.
