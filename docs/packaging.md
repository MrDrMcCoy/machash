# Packaging

machash ships packages for Homebrew, Alpine, Debian, OpenSuSE,
Fedora, Arch, NixOS, and Void Linux. The packaging files live in
packaging/.

## How a package builds

Each package builds from the source tarball that the release
workflow publishes for the tag, for example
machash-1.0.0.tar.gz. The tarball comes from `make dist`. It has a
fixed file list and normalized timestamps, so a given tree gives the
same bytes.

The build downloads cosmocc 4.0.2 and compiles the binary with it.
The result is a single-file static universal binary. It runs on all
platforms that Cosmopolitan supports, so one source tree serves all
architectures.

The build needs network access. It fetches the cosmocc toolchain and
nothing else.

## The APE loader

A Cosmopolitan binary is an APE binary. macOS runs it natively.
Linux runs it through the APE binfmt handler.

The cosmocc helper tools (the gcc backends, apelink, fixupobj,
pecheck, and mktemper) are APE binaries as well. A package build
host does not have the APE binfmt handler, so each package runs the
helpers through the APE loader for the build architecture, which
ships inside the cosmocc zip. The build replaces each helper with a
two-line shell script that calls the loader.

At install time, the package ships the APE loader for the host
architecture at /usr/lib/machash/ape and registers the binfmt
handler, unless one is already registered. On removal, the package
unregisters the handler only if it registered it. If the handler
cannot be registered, the package prints a warning and the binary
will not run until you register it yourself:

    echo ':APE:M::MZqFpD::/usr/lib/machash/ape:' \
      > /proc/sys/fs/binfmt_misc/register

## Package files

| Ecosystem | File |
|---|---|
| Homebrew | packaging/homebrew/machash.rb |
| Alpine | packaging/alpine/APKBUILD |
| Debian | packaging/debian/ |
| OpenSuSE | packaging/opensuse/machash.spec |
| Fedora | packaging/fedora/machash.spec |
| Arch | packaging/arch/PKGBUILD and machash.install |
| NixOS | packaging/nixos/default.nix and module.nix |
| Void Linux | packaging/void/machash.template |

Each file is self-contained. None of them reference files outside
the source tarball.

## Building the packages

The build hosts need the tools of each ecosystem plus curl and
unzip. Each package downloads the release source tarball for its
version, so build the packages after the tag is published. The
Makefile has a target for each package (`make package-nixos`,
`make package-void`, and so on) and `make packages` for all of
them.

Homebrew:

    brew install --build-from-source \
      packaging/homebrew/machash.rb

Alpine:

    cd packaging/alpine
    abuild

Debian. The release tarball must be renamed to
machash_1.0.0.orig.tar.gz first:

    dpkg-source -x machash_1.0.0.orig.tar.gz
    cp -r packaging/debian machash-1.0.0/debian
    cd machash-1.0.0
    dpkg-buildpackage -us -uc -b

OpenSuSE and Fedora:

    rpmbuild -bb packaging/opensuse/machash.spec
    rpmbuild -bb packaging/fedora/machash.spec

Arch:

    makepkg -s -C packaging/arch

NixOS. With nixpkgs in the NIX_PATH:

    nix build --impure --expr \
      '(import <nixpkgs> {}).callPackage \
       (import packaging/nixos/default.nix) {}'

On a NixOS system, import packaging/nixos/module.nix and set
machash.enable = true. The module installs the package and
registers the APE binfmt handler.

Void Linux, from a void-packages checkout:

    mkdir -p srcpkgs/machash
    cp packaging/void/machash.template srcpkgs/machash/template
    ./xbps-src setup
    ./xbps-src pkgbuild machash

## Testing

The test suite runs in the upstream CI on every pull request and on
each tag. It does not run inside a package build. The build chroots
have no APE binfmt handler, and the test binaries are APE binaries.
The package installs the same binary that CI tested.
