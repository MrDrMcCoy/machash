# The Bobcat hash

machash uses Bobcat, a 48-bit hash modeled on the Tiger hash. The
reference description and test vectors are in padolph's StampInfosec
repository (Chapter 5):
https://github.com/padolph/StampInfosec/blob/master/Chapter5/bobcat/bobcat.c

## How it works

- The message is split into 128-bit blocks of eight 16-bit words.
  Input bytes are packed little-endian (byte 0 is the low byte of the
  word), and the final block is padded with zero words. An empty
  input hashes one zero block, so the hash of the empty string is
  well-defined.
- The state is three 16-bit words (a, b, c), initialized to 0xface,
  0xe961, 0x041d.
- Each block passes through three outer rounds with multipliers 5, 7,
  and 9. An outer round applies eight inner rounds, each mixing one
  message word into the state through two S-boxes (each mapping 4
  bits to 16 bits); between outer rounds a key schedule re-derives
  the message words.
- After the final round the pre-round state is folded back in
  (feedforward).
- The 48-bit digest is (a << 32) | (b << 16) | c.

## Validation

- src/bobcat.c is tested against the four vectors published with the
  reference (tests/unit_bobcat.c).
- tests/ref/bobcat_ref.c is an independently structured second
  implementation; tests/ref/check_oracle.sh checks it against the
  same four vectors before its output is used as expected values in
  the integration suite.
- The empty-input digest (0xe6be9fc25f39) was taken from the
  certified oracle.

For background on the Tiger construction, see Wikipedia's article on
[Tiger (hash function)][tiger].

[tiger]: https://en.wikipedia.org/wiki/Tiger_(hash_function)
