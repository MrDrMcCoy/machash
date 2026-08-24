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
  and 9. An outer round applies eight inner rounds. Each inner round
  mixes one message word into the state through two S-boxes. Each
  S-box maps 4 bits to 16 bits. Between outer rounds, a key schedule
  re-derives the message words.
- After the final round the pre-round state is folded back in
  (feedforward).
- The 48-bit digest is (a << 32) | (b << 16) | c.

## Validation

- tests/unit_bobcat.c tests src/bobcat.c against the four vectors
  published with the reference.
- tests/ref/bobcat_ref.c is a second implementation with a different
  structure. tests/ref/check_oracle.sh checks it against the same
  four vectors. The integration suite uses the certified output as
  expected values.
- The empty-input digest (0xe6be9fc25f39) comes from the certified
  oracle.

For background on the Tiger construction, see Wikipedia's article on
[Tiger (hash function)][tiger].

[tiger]: https://en.wikipedia.org/wiki/Tiger_(hash_function)
