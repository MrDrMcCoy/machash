
# Agent goals for this project

- Write all code in the C language, using Cosmopolitan C as its
  standard library and cosmocc for the compiler. The library can be
  found at https://github.com/jart/cosmopolitan, and compiler at
  https://cosmo.zip/pub/cosmocc/. Cosmopolitan libc is ISC-licensed and
  does not require special handling, but should be attributed.
- Resulting application should be a single file, static binary that
  can be executed on all targets that Cosmopolitan libc / cosmocc
  supports.
- Use BSD 3-clause License for this project.
- Binary name should be `machash`.
- Accept string input either on standard input or as option flags `-s`
  and `--string`. Multi-line inputs should be treated as a single
  input. Leading and trailing whitespace should be stripped.
- Perform 48-bit Bobcat hash on input string. Reference implementation
  is here:
https://github.com/padolph/StampInfosec/blob/master/Chapter5/bobcat/bobcat.c
- By default, format the output hash as a Unix-style, colon-delimeted
  string that can be used as a MAC address.
- Provide option flags `-p` and `--plain` to output hash without
  formatting or `0x` hexadecimal prefix.
- Provide option flags `-u` and `--unicast` to convert generated hash
  to nearest unicast MAC address. This mode should only clear the
  multicast bit.
- Provide option flag `--local` to force generated MAC addresses to
  appear as vendor-unassigned, locally-administered addresses. This
  shoult not suppress the unicast warning.
- Detect non-unicast MAC addresses. If neither plain mode nor unicast
  mode were specified, print the MAC address to stdout along with a
  warning to stderr that explains why this matters and what you can do.
  A similar warning for vendor-unassigned addressing is not necessary.
