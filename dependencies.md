# Dependencies

Build tools and third-party code used by machash.

| Name | Version | License | Purpose |
|---|---|---|---|
| [Cosmopolitan Libc](https://github.com/jart/cosmopolitan) | bundled in cosmocc 4.0.2 | ISC | C standard library; yields the single-file static universal binary |
| [cosmocc](https://cosmo.zip/pub/cosmocc/) | 4.0.2 | ISC, with LLVM-derived components under Apache-2.0 WITH LLVM-exception | Compiler for Cosmopolitan (toolchain only) |
| [Bobcat reference](https://github.com/padolph/StampInfosec/blob/master/Chapter5/bobcat/bobcat.c) | StampInfosec master | see note | Algorithm description and published test vectors for the 48-bit Bobcat hash |

Note: the StampInfosec repository carries no license file. machash does not
copy code from it; the Bobcat algorithm (an educational hash modeled on
Tiger) is re-implemented in src/bobcat.c and validated against the vectors
published in its comments. See docs/hash.md.
