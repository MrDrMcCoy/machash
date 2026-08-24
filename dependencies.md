# Dependencies

Build tools and third-party code used by machash.

- [Cosmopolitan Libc][cosmo] - bundled in cosmocc 4.0.2. ISC
  licensed. It is the C standard library. It produces the
  single-file static universal binary.
- [cosmocc][cosmocc] 4.0.2 - ISC, with LLVM-derived components under
  Apache-2.0 WITH LLVM-exception. Compiler for Cosmopolitan
  (toolchain only).
- Bobcat reference (StampInfosec master, Chapter 5) - algorithm
  description and published test vectors for the 48-bit Bobcat hash.
  There is no license file. See the note below.
  Reference:
https://github.com/padolph/StampInfosec/blob/master/Chapter5/bobcat/bobcat.c

Note: the StampInfosec repository carries no license file. machash
does not copy code from it. The Bobcat algorithm is an educational
hash modeled on Tiger. machash re-implements it in src/bobcat.c and
validates it against the vectors published in the comments of the
reference. See docs/hash.md.

[cosmo]: https://github.com/jart/cosmopolitan
[cosmocc]: https://cosmo.zip/pub/cosmocc/
