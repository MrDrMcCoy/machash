class Machash < Formula
  desc "Hash arbitrary strings into MAC addresses"
  homepage "https://github.com/MrDrMcCoy/machash"
  url "https://github.com/MrDrMcCoy/machash/releases/" \
      "download/v1.0.0/machash-1.0.0.tar.gz"
  sha256 "144ff28570b0e999dca1c88d7f2cacafe84d0e4e855bc3343b190a0d87616234"
  license "BSD-3-Clause"
  depends_on "curl"
  depends_on "unzip"

  # The binary is a single-file static universal binary. It is built
  # with cosmocc, which is fetched at build time. macOS runs
  # Cosmopolitan (APE) binaries natively, so no loader is needed.
  def install
    system "curl", "-fsSL", "-o", "cosmocc.zip",
      "https://cosmo.zip/pub/cosmocc/cosmocc-4.0.2.zip"
    mkdir "cosmo" do
      system "unzip", "-q", "../cosmocc.zip"
    end
    system "make", "build", "CC=#{pwd}/cosmo/bin/cosmocc"
    bin.install "dist/machash"
    (share/"man/man1").install "man/machash.1"
  end
end
