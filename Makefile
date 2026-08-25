# machash build rules.
#
# Targets:
#   build   - build dist/machash (single-file static universal binary)
#   test    - build and run unit tests, then the integration suite
#   fuzz    - build and run the deterministic fuzz harness
#   lint    - static analysis of C sources, shell scripts, line widths
#   man     - render the man page to check it is well-formed groff
#   dist      - build the versioned source tarball (release asset)
#   toolchain - install the cosmocc toolchain under ~/.local
#   packages  - build all the OS packages (see the package-* targets)
#   package-* - build one OS package from the release source tarball
#   install   - copy the binary and man page into $(PREFIX)
#   clean     - remove build outputs

# Default to cosmocc (overridable on the command line).
CC      = cosmocc
CFLAGS  = -O2 -std=gnu11 -Wall -Wextra -Werror -Wshadow
PREFIX  = $(HOME)/.local

# Bake build and version info into the binary (see --version).
VERSION := 1.0.1

PROG    := machash
DIST    := dist
BUILD   := build

SRC     := src/bobcat.c src/log.c src/args.c src/mac.c src/machash.c
HDR     := src/bobcat.h src/log.h src/args.h src/mac.h
UNIT    := $(BUILD)/test_bobcat $(BUILD)/test_log $(BUILD)/test_args
FUZZ    := $(BUILD)/fuzz_bobcat $(BUILD)/fuzz_args

# Fuzz targets: UBSan aborts the run on the first report, so a
# nonzero exit code is a finding. cosmocc has no libFuzzer mode, so
# the harness is the deterministic driver in tests/fuzz/fuzz.c.
FUZZ_CFLAGS = $(CFLAGS) -fsanitize=undefined -fno-sanitize-recover=all
# libFuzzer-style flags, accepted by both fuzz targets.
FUZZ_ARGS ?= -runs=100000 -seed=1

all: build

# Generated header carrying the baked-in version, build info, commit
# hash, and build number. PHONY so every build gets a new number.
.PHONY: $(BUILD)/version.h
$(BUILD)/version.h: toolchain-check
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

# Fuzz targets: each links the deterministic driver with the code
# under test. For a longer session pass FUZZ_ARGS="-runs=<big>
# -seed=<n>" to make.
$(BUILD)/fuzz_bobcat: toolchain-check tests/fuzz/fuzz_bobcat.c \
                      tests/fuzz/fuzz.c tests/fuzz/fuzz.h \
                      src/bobcat.c src/bobcat.h
	@mkdir -p $(BUILD)
	$(CC) $(FUZZ_CFLAGS) -Isrc -Itests/fuzz -o $@ \
		tests/fuzz/fuzz_bobcat.c tests/fuzz/fuzz.c src/bobcat.c

$(BUILD)/fuzz_args: toolchain-check tests/fuzz/fuzz_args.c \
                    tests/fuzz/fuzz.c tests/fuzz/fuzz.h src/args.c \
                    src/args.h src/log.c src/log.h src/mac.c src/mac.h
	@mkdir -p $(BUILD)
	$(CC) $(FUZZ_CFLAGS) -Isrc -Itests/fuzz -o $@ \
		tests/fuzz/fuzz_args.c tests/fuzz/fuzz.c src/args.c \
		src/log.c src/mac.c

fuzz: $(FUZZ)
	$(BUILD)/fuzz_bobcat $(FUZZ_ARGS)
	@$(BUILD)/fuzz_args $(FUZZ_ARGS) \
		|| { cat $(BUILD)/fuzz_args.stderr; exit 1; }

# The man target render-checks the man page (groff must be present).
test: build $(UNIT) man
	$(BUILD)/test_bobcat
	$(BUILD)/test_log
	$(BUILD)/test_args
	./tests/ref/check_oracle.sh
	./tests/integration.sh

# Render the man page to make sure it is well-formed groff.
man:
	groff -Tascii -man man/machash.1 > /dev/null

lint: lint-c lint-sh lint-width

# The strict -Werror build doubles as a compile-time lint.
lint-c: build
	cppcheck -Isrc --enable=all --std=c11 -q --inline-suppr \
		--suppress=missingIncludeSystem \
		--suppress=checkersReport \
		--suppress=normalCheckLevelMaxBranches \
		--suppress=unmatchedSuppression src/ tests/

lint-sh:
	shellcheck tests/integration.sh tests/ref/check_oracle.sh \
	    tools/install-cosmocc.sh

# Keep sources and docs aligned to 80 columns (see agents.md). The
# Debian changelog maintainer line is exempt: dpkg-parsechangelog
# requires it on one line.
lint-width:
	@bad=0; \
	for f in $$(git ls-files '*.c' '*.h' '*.md' '*.sh' '*.1' \
	              'packaging/*' Makefile); do \
	  if [ "$$f" = packaging/debian/changelog ]; then \
	    continue; \
	  fi; \
	  if awk 'length > 80 { bad = 1; exit } END { exit bad }' "$$f"; then \
	    :; \
	  else \
	    echo "lint-width: $$f has lines longer than 80 columns" >&2; \
	    bad=1; \
	  fi; \
	done; \
	exit $$bad

# Source tarball for the release and the package builds. The file
# list is fixed and the mtimes are normalized, so the tarball is
# reproducible for a given tree.
dist:
	@rm -rf $(DIST)/$(PROG)-$(VERSION).tar.gz $(BUILD)/sdist
	@mkdir -p $(BUILD)/sdist/$(PROG)-$(VERSION)
	@cp -R src tests man docs tools $(BUILD)/sdist/$(PROG)-$(VERSION)/
	@cp LICENSE Makefile readme.md changelog.md dependencies.md \
	  $(BUILD)/sdist/$(PROG)-$(VERSION)/
	@cd $(BUILD)/sdist && \
	find $(PROG)-$(VERSION) -exec touch -t 2601010000.00 {} + && \
	find $(PROG)-$(VERSION) | sort | \
	tar cf - --no-recursion -T - --owner=0 --group=0 --numeric-owner | \
	gzip -9n > $(CURDIR)/$(DIST)/$(PROG)-$(VERSION).tar.gz
	@rm -rf $(BUILD)/sdist
	@echo "wrote $(DIST)/$(PROG)-$(VERSION).tar.gz"

# Install the cosmocc toolchain under ~/.local, where the build
# looks for it (see the script for the download and the APE
# handler handling on Linux).
toolchain:
	sh tools/install-cosmocc.sh

# The compile step needs $(CC) on the PATH.
.PHONY: toolchain-check
toolchain-check:
	@command -v $(CC) >/dev/null 2>&1 || { \
	  echo "error: $(CC) is not on the PATH" >&2; \
	  echo "run: make toolchain, or set CC to a compiler" >&2; \
	  exit 1; \
	}

# OS package builds. Each target builds one package from the
# release source tarball for $(VERSION), which the tag workflow
# publishes, so run them after the tag is published. The build
# host needs the tools of each ecosystem; see docs/packaging.md.
packages: package-nixos package-void package-alpine package-debian \
          package-opensuse package-fedora package-arch package-homebrew

package-nixos:
	nix build --impure --expr \
	  '(import <nixpkgs> {}).callPackage \
	   (import packaging/nixos/default.nix) {}'

package-void:
	@if [ ! -d $(BUILD)/void-packages/.git ]; then \
	  git clone --depth 1 \
	    https://github.com/void-linux/void-packages \
	    $(BUILD)/void-packages; \
	fi
	$(BUILD)/void-packages/xbps-src setup
	mkdir -p $(BUILD)/void-packages/srcpkgs/$(PROG)
	cp packaging/void/machash.template \
	  $(BUILD)/void-packages/srcpkgs/$(PROG)/template
	$(BUILD)/void-packages/xbps-src pkgbuild $(PROG)

package-alpine:
	cd packaging/alpine && abuild

package-debian: dist
	rm -rf $(BUILD)/debian
	mkdir -p $(BUILD)/debian
	cp $(DIST)/$(PROG)-$(VERSION).tar.gz \
	  $(BUILD)/debian/$(PROG)_$(VERSION).orig.tar.gz
	dpkg-source -x $(BUILD)/debian/$(PROG)_$(VERSION).orig.tar.gz \
	  $(BUILD)/debian
	cp -r packaging/debian \
	  $(BUILD)/debian/$(PROG)-$(VERSION)/debian
	dpkg-buildpackage -us -uc -b -F \
	  $(BUILD)/debian/$(PROG)-$(VERSION)

package-opensuse:
	rpmbuild -bb packaging/opensuse/machash.spec

package-fedora:
	rpmbuild -bb packaging/fedora/machash.spec

package-arch:
	makepkg -s -C packaging/arch

package-homebrew:
	brew install --build-from-source packaging/homebrew/machash.rb

install: build
	mkdir -p $(PREFIX)/bin $(PREFIX)/man/man1
	cp $(DIST)/$(PROG) $(PREFIX)/bin/$(PROG)
	cp man/machash.1 $(PREFIX)/man/man1/machash.1

clean:
	rm -rf $(DIST) $(BUILD)

.PHONY: all build test fuzz lint lint-c lint-sh lint-width man dist \
        toolchain toolchain-check packages package-nixos package-void \
        package-alpine package-debian package-opensuse package-fedora \
        package-arch package-homebrew install clean
