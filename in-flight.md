# In-flight work

Records work in progress so another agent can resume interrupted work.

## Current

None. v1.0.1 is released (2026-08-25). See changelog.md.

## Notes for the next agent

- Toolchain: cosmocc 4.0.2 in ~/.local/bin. `make toolchain`
  installs it (tools/install-cosmocc.sh). Run `make build`,
  `make test`, `make lint`, `make fuzz`.
- Lint tools: shellcheck and cppcheck in ~/.local/bin (cppcheck is a
  pip wheel installed in ~/venv, symlinked into ~/.local/bin).
- The fuzz harness is deterministic (splitmix64). For a deeper pass
  run `make fuzz FUZZ_ARGS="-runs=1000000 -seed=<n>"`. A nonzero
  exit is a UBSan abort or a crash.
- Expected hash values in tests/integration.sh come from the
  certified independent oracle (tests/ref/). Regenerate them with
  `printf '%s' STR | build/bobcat_ref` if the oracle changes.
- Keep all code and markdown within 80 columns (enforced by
  `make lint-width`).
- Workflows live in .github/workflows/ (Gitea scans that directory
  as well as .gitea/workflows/).
