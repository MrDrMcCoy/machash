# In-flight work

Records work in progress so another agent can resume interrupted work.

## Current

None. v0.1.0 is released (2026-08-22); see changelog.md.

## Notes for the next agent

- Toolchain: cosmocc 4.0.2 in ~/.local/bin; `make build`,
  `make test`, `make lint`.
- Lint tools: shellcheck and cppcheck in ~/.local/bin (cppcheck via a
  nix profile under ~/.local/nix).
- Expected hash values in tests/integration.sh come from the
  certified independent oracle (tests/ref/); regenerate with
  `printf '%s' STR | build/bobcat_ref` if the oracle changes.
- Keep all code and markdown within 80 columns (enforced by
  `make lint-width`).
- Workflows live in .github/workflows/ (Gitea scans that directory
  as well as .gitea/workflows/).
