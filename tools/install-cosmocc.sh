#!/bin/sh
# Install the cosmocc toolchain for the machash build.
#
# The compile step runs CC=cosmocc from the PATH, and this project
# installs missing tools under ~/.local (see agents.md), so the
# default prefix is ~/.local. The toolchain is the pinned cosmocc
# 4.0.2 release zip (about 420 MB), verified by sha256 before use.
#
# Usage:
#   sh tools/install-cosmocc.sh [PREFIX]
#
# Minimal dependencies: sh, a downloader (curl or wget), a zip
# extractor (unzip, bsdtar, or python3), and a sha256 tool
# (sha256sum, shasum, openssl, or python3). On Linux, registering
# the APE binfmt handler needs root.

set -eu

VERSION=4.0.2
URL="https://cosmo.zip/pub/cosmocc/cosmocc-${VERSION}.zip"
SHA256="85b8c37a406d862e656ad4ec14be9f6ce474c1b436b9615e91a55208aced3f44"
PREFIX="${1:-${HOME}/.local}"

die() {
    printf 'install-cosmocc: %s\n' "$1" >&2
    exit 1
}

have() {
    command -v "$1" >/dev/null 2>&1
}

if have curl; then
    fetch() { curl -fsSL -o "$2" "$1"; }
elif have wget; then
    fetch() { wget -q -O "$2" "$1"; }
else
    die "need curl or wget to download the toolchain"
fi

if have unzip; then
    # -o overwrites without prompting, so a re-run updates in place.
    extract() { unzip -qo "$1" -d "$2"; }
elif have bsdtar; then
    extract() { bsdtar -xf "$1" -C "$2"; }
elif have python3; then
    # zipfile -e would write the zip symlinks as plain files, which
    # breaks the cross targets. Extract by hand instead.
    extract() {
        python3 - "$1" "$2" <<'PY'
import os, sys, zipfile
src, dest = sys.argv[1], sys.argv[2]
with zipfile.ZipFile(src) as z:
    for info in z.infolist():
        path = os.path.join(dest, info.filename)
        if info.is_dir():
            os.makedirs(path, exist_ok=True)
            continue
        os.makedirs(os.path.dirname(path), exist_ok=True)
        if (info.external_attr >> 16) & 0o170000 == 0o120000:
            if os.path.lexists(path):
                os.remove(path)
            os.symlink(z.read(info).decode(), path)
        else:
            with z.open(info) as f, open(path, "wb") as out:
                out.write(f.read())
PY
    }
else
    die "need unzip, bsdtar, or python3 to unpack the toolchain"
fi

if have sha256sum; then
    digest() { sha256sum "$1" | cut -d ' ' -f 1; }
elif have shasum; then
    digest() { shasum -a 256 "$1" | cut -d ' ' -f 1; }
elif have openssl; then
    digest() { openssl dgst -sha256 "$1" | awk '{print $NF}'; }
elif have python3; then
    digest() {
        python3 -c 'import hashlib, sys
print(hashlib.sha256(open(sys.argv[1], "rb").read()).hexdigest())' "$1"
    }
else
    die "need sha256sum, shasum, openssl, or python3 to verify"
fi

tmp=$(mktemp -d)
trap 'rm -rf "$tmp"' EXIT INT TERM

printf 'install-cosmocc: fetching %s\n' "$URL"
fetch "$URL" "$tmp/cosmocc.zip"

printf 'install-cosmocc: verifying the download\n'
got=$(digest "$tmp/cosmocc.zip")
if [ "$got" != "$SHA256" ]; then
    die "sha256 mismatch: got ${got}, want ${SHA256}"
fi

printf 'install-cosmocc: installing to %s\n' "$PREFIX"
mkdir -p "$PREFIX"
extract "$tmp/cosmocc.zip" "$PREFIX"
# The zip stores the modes, but not every extractor honors them
# (python3 -m zipfile drops them), so set the tool bits by hand.
chmod +x "$PREFIX"/bin/* "$PREFIX"/libexec/clang \
    "$PREFIX"/libexec/gcc/*/*/*

case ":${PATH}:" in
    *":${PREFIX}/bin:"*) ;;
    *)
        printf 'install-cosmocc: add %s to the PATH\n' "${PREFIX}/bin"
        ;;
esac

if [ "$(uname)" = "Linux" ]; then
    # The cosmocc helper tools are APE binaries. Linux runs APE
    # binaries through the binfmt handler, which needs root to
    # register. Skip the step if a handler is already registered.
    loader="${PREFIX}/bin/ape-$(uname -m).elf"
    if [ ! -e /proc/sys/fs/binfmt_misc/APE ]; then
        if [ ! -d /proc/sys/fs/binfmt_misc ]; then
            mount -t binfmt_misc none /proc/sys/fs/binfmt_misc \
                2>/dev/null || true
        fi
        if [ -d /proc/sys/fs/binfmt_misc ]; then
            if printf ':APE:M::MZqFpD::%s:\n' "$loader" \
                > /proc/sys/fs/binfmt_misc/register 2>/dev/null; then
                printf 'install-cosmocc: registered the APE handler\n'
            fi
        fi
    fi
    if [ ! -e /proc/sys/fs/binfmt_misc/APE ]; then
        {
            printf 'warning: the APE binfmt handler is not registered.\n'
            printf 'The build needs it to run. Register it as root:\n'
            printf '  echo %s > /proc/sys/fs/binfmt_misc/register\n' \
                "':APE:M::MZqFpD::${loader}:'"
        } >&2
    fi
fi

# Verify the install where APE binaries can run. On Linux without
# the handler the check would fail for lack of the handler, not
# for lack of the toolchain.
if [ "$(uname)" != "Linux" ] || [ -e /proc/sys/fs/binfmt_misc/APE ]; then
    if ! ver=$("$PREFIX/bin/cosmocc" --version 2>&1); then
        die "the installed cosmocc does not run: ${ver}"
    fi
    printf 'install-cosmocc: installed %s\n' \
        "$(printf '%s\n' "$ver" | head -n 1)"
else
    printf 'install-cosmocc: installed (unverified: no APE handler)\n'
fi
