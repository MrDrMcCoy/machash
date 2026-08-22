# Roadmap

Planned work for machash. Completed items move to changelog.md.

## 0.2.0

- [ ] Line-oriented input mode: hash one line at a time from a file or stdin
- [ ] More output formats (0x prefix, byte-swapped octet order)
- [ ] `--check` mode to verify an existing MAC against a string

## Later

- [ ] Fuzz harness for the argument parser and hash
- [ ] Per-architecture release assets (cosmocc already emits a universal binary)
- [ ] Man page

## Done

- 0.1.0 - initial release: Bobcat hash, MAC/plain output, bit control flags,
  stdin and flag inputs, help and logging, tests, CI. See changelog.md.
