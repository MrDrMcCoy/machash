# machash - hash strings into MAC addresses.
#
# The binary is a single-file static universal binary built with
# cosmocc 4.0.2, which is downloaded at build time. The cosmocc
# helper tools are Cosmopolitan (APE) binaries. A build host has
# no APE binfmt handler, so each helper is run through the APE
# loader for this architecture, which ships inside the cosmocc
# zip.
#
# The test suite runs in the upstream CI. It does not run in the
# build sandbox, which has no APE binfmt handler.
#
# Build with nixpkgs:
#   nix build --impure --expr \
#     '(import <nixpkgs> {}).callPackage (import ./default.nix) {}'
#
# On NixOS, prefer module.nix, which installs the package and
# registers the APE binfmt handler.

{ stdenvNoCC, fetchurl, make, curl, unzip, licenses, platforms }:

stdenvNoCC.mkDerivation rec {
  pname = "machash";
  version = "1.0.1";

  src = fetchurl {
    url = "https://github.com/MrDrMcCoy/machash/releases/"
      + "download/v${version}/${pname}-${version}.tar.gz";
    sha256 = "__SDIST_SHA256__";
  };

  nativeBuildInputs = [ make curl unzip ];

  dontConfigure = true;

  makeFlags = [ "CC=$(pwd)/cosmo/bin/cosmocc" ];

  preBuild = ''
    curl -fsSL -o cosmocc.zip \
      "https://cosmo.zip/pub/cosmocc/cosmocc-4.0.2.zip"
    mkdir -p cosmo
    unzip -q cosmocc.zip -d cosmo
    for t in apelink fixupobj pecheck mktemper \
      x86_64-linux-cosmo-gcc aarch64-linux-cosmo-gcc; do
      mv "cosmo/bin/$t" "cosmo/bin/$t.ape"
      {
        printf '#!/bin/sh\n'
        printf 'exec "$(dirname "$0")/ape-%s.elf" ' "$(uname -m)"
        printf '"$(dirname "$0")/%s.ape" "$@"\n' "$t"
      } > "cosmo/bin/$t"
      chmod +x "cosmo/bin/$t"
    done
  '';

  installPhase = ''
    install -Dm755 dist/machash $out/bin/machash
    install -Dm755 "cosmo/bin/ape-$(uname -m).elf" $out/lib/machash/ape
    install -Dm644 LICENSE $out/share/licenses/machash/LICENSE
    install -Dm644 readme.md changelog.md dependencies.md \
      $out/share/doc/machash/
    install -Dm644 docs/*.md $out/share/doc/machash/
  '';

  postInstall = ''
    installManPage man/machash.1
  '';

  doCheck = false;

  meta = {
    description = "Hash arbitrary strings into MAC addresses";
    longDescription = ''
      machash hashes input strings with the 48-bit Bobcat hash and
      prints the result as a colon-separated MAC address. The
      address is fake, but it has the form of a valid MAC address.
      This is useful for deriving a stable, deterministic MAC
      address from any string.
    '';
    license = licenses.bsd3;
    platforms = platforms.all;
    mainProgram = "machash";
  };
}
