# machash build rules.
#
# Targets:
#   build   - build dist/machash (single-file static universal binary)
#   test    - build and run unit tests, then the integration suite
#   lint    - static analysis of C sources and shell scripts
#   install - copy the binary into $(PREFIX)/bin
#   clean   - remove build outputs

# Default to cosmocc (overridable on the command line).
CC      = cosmocc
CFLAGS  = -O2 -std=gnu11 -Wall -Wextra -Werror -Wshadow
PREFIX  ?= $(HOME)/.local

PROG    := machash
DIST    := dist
BUILD   := build

SRC     := src/bobcat.c src/log.c src/args.c src/machash.c
HDR     := src/bobcat.h src/log.h src/args.h
UNIT    := $(BUILD)/test_bobcat $(BUILD)/test_log $(BUILD)/test_args

all: build

$(DIST)/$(PROG): $(SRC) $(HDR)
	@mkdir -p $(DIST)
	$(CC) $(CFLAGS) -o $@ $(SRC)

build: $(DIST)/$(PROG)

# Unit tests: each driver links the sources under test.
$(BUILD)/test_bobcat: tests/unit_bobcat.c tests/test.h src/bobcat.c src/bobcat.h
	@mkdir -p $(BUILD)
	$(CC) $(CFLAGS) -Isrc -o $@ tests/unit_bobcat.c src/bobcat.c

$(BUILD)/test_log: tests/unit_log.c tests/test.h src/log.c src/log.h
	@mkdir -p $(BUILD)
	$(CC) $(CFLAGS) -Isrc -o $@ tests/unit_log.c src/log.c

$(BUILD)/test_args: tests/unit_args.c tests/test.h src/args.c src/args.h src/log.c src/log.h
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

lint: lint-c lint-sh

lint-c:
	cppcheck -Isrc --enable=all --std=c11 -q --inline-suppr \
		--suppress=missingIncludeSystem \
		--suppress=checkersReport \
		--suppress=normalCheckLevelMaxBranches src/ tests/
	$(CC) $(CFLAGS) -fsyntax-only $(SRC)

lint-sh:
	shellcheck tests/integration.sh tests/ref/check_oracle.sh

install: build
	mkdir -p $(PREFIX)/bin
	cp $(DIST)/$(PROG) $(PREFIX)/bin/$(PROG)

clean:
	rm -rf $(DIST) $(BUILD)

.PHONY: all build test lint lint-c lint-sh install clean
