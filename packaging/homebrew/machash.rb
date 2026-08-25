class Machash < Formula
  desc "Hash arbitrary strings into MAC addresses"
  homepage "https://github.com/MrDrMcCoy/machash"
  url "https://github.com/MrDrMcCoy/machash/releases/" \
      "download/v1.0.2/machash-1.0.2.tar.gz"
  sha256 "971429a3de578cb937920925bb4c7e8978abc9ae4d14c7b1340bc4712e3733b2"
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
