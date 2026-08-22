# machash build rules.
#
# Targets:
#   build   - build dist/machash (single-file static universal binary)
#   test    - build and run unit tests, then the integration suite
#   lint    - static analysis of C sources, shell scripts, line widths
#   install - copy the binary into $(PREFIX)/bin
#   clean   - remove build outputs

# Default to cosmocc (overridable on the command line).
CC      = cosmocc
CFLAGS  = -O2 -std=gnu11 -Wall -Wextra -Werror -Wshadow
PREFIX  = $(HOME)/.local

# Bake build and version info into the binary (see --version).
VERSION := 0.1.0

PROG    := machash
DIST    := dist
BUILD   := build

SRC     := src/bobcat.c src/log.c src/args.c src/machash.c
HDR     := src/bobcat.h src/log.h src/args.h
UNIT    := $(BUILD)/test_bobcat $(BUILD)/test_log $(BUILD)/test_args

all: build

# Generated header carrying the baked-in version, build info, commit
# hash, and build number. PHONY so every build gets a new number.
.PHONY: $(BUILD)/version.h
$(BUILD)/version.h:
	@mkdir -p $(BUILD)
	@{ \
	if [ -f $(BUILD)/.build-number ]; then \
	  n=$$(( $$(cat $(BUILD)/.build-number) + 1 )); \
	else \
	  n=1; \
	fi; \
	echo $$n > $(BUILD)/.build-number; \
	commit=unknown; \
	if git rev-parse HEAD >/dev/null 2>&1; then \
	  commit=$$(git rev-parse HEAD | cut -c1-12); \
	  git diff-index --quiet HEAD >/dev/null 2>&1 \
	    || commit="$${commit}-dirty"; \
	fi; \
	printf '#define MACHASH_VERSION "%s"\n' "$(VERSION)"; \
	printf '#define MACHASH_BUILD "%s"\n' \
	  "$$( $(CC) --version | head -1 )"; \
	printf '#define MACHASH_COMMIT "%s"\n' "$$commit"; \
	printf '#define MACHASH_BUILD_NUMBER "%s"\n' "$$n"; \
	} > $@

$(DIST)/$(PROG): $(SRC) $(HDR) $(BUILD)/version.h
	@mkdir -p $(DIST)
	$(CC) $(CFLAGS) -I$(BUILD) -o $@ $(SRC)
	@rm -f $@.aarch64.elf $@.x86_64.elf $@.com.dbg

build: $(DIST)/$(PROG)

# Unit tests: each driver links the sources under test.
$(BUILD)/test_bobcat: tests/unit_bobcat.c tests/test.h src/bobcat.c src/bobcat.h
	@mkdir -p $(BUILD)
	$(CC) $(CFLAGS) -Isrc -o $@ tests/unit_bobcat.c src/bobcat.c

$(BUILD)/test_log: tests/unit_log.c tests/test.h src/log.c src/log.h
	@mkdir -p $(BUILD)
	$(CC) $(CFLAGS) -Isrc -o $@ tests/unit_log.c src/log.c

$(BUILD)/test_args: tests/unit_args.c tests/test.h src/args.c \
                    src/args.h src/log.c src/log.h
	@mkdir -p $(BUILD)
	$(CC) $(CFLAGS) -Isrc -o $@ tests/unit_args.c src/args.c src/log.c

# Independent Bobcat oracle, used to generate and check expected output.
$(BUILD)/bobcat_ref: tests/ref/bobcat_ref.c
	@mkdir -p $(BUILD)
	$(CC) $(CFLAGS) -o $@ tests/ref/bobcat_ref.c

test: build $(UNIT)
	$(BUILD)/test_bobcat
	$(BUILD)/test_log
	$(BUILD)/test_args
	./tests/ref/check_oracle.sh
	./tests/integration.sh

lint: lint-c lint-sh lint-width

# The strict -Werror build doubles as a compile-time lint.
lint-c: build
	cppcheck -Isrc --enable=all --std=c11 -q --inline-suppr \
		--suppress=missingIncludeSystem \
		--suppress=checkersReport \
		--suppress=normalCheckLevelMaxBranches src/ tests/

lint-sh:
	shellcheck tests/integration.sh tests/ref/check_oracle.sh

# Keep sources and docs aligned to 80 columns (see agents.md).
lint-width:
	@bad=0; \
	for f in $$(git ls-files '*.c' '*.h' '*.md' '*.sh' Makefile); do \
	  if awk 'length > 80 { bad = 1; exit } END { exit bad }' "$$f"; then \
	    :; \
	  else \
	    echo "lint-width: $$f has lines longer than 80 columns" >&2; \
	    bad=1; \
	  fi; \
	done; \
	exit $$bad

install: build
	mkdir -p $(PREFIX)/bin
	cp $(DIST)/$(PROG) $(PREFIX)/bin/$(PROG)

clean:
	rm -rf $(DIST) $(BUILD)

.PHONY: all build test lint lint-c lint-sh lint-width install clean
