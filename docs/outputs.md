# Output formats

## MAC address (default)

The 48-bit digest is printed as six colon-separated lowercase hex
octets, most significant octet first:

    0f:c2:c1:58:42:59

This is the Unix-style form; see the Wikipedia article on
[MAC addresses][mac] for the octet and bit layout.

## Plain

`-p/--plain` prints the digest as 12 lowercase hex digits with no
separators and no `0x` prefix:

    0fc2c1584259

## Address bits

The first octet carries two semantic bits (see the Wikipedia article
on [MAC addresses][mac]):

- Bit 0 (0x80 in the first octet): the multicast bit. When set, the
  address is a group address rather than a device address.
- Bit 1 (0x40 in the first octet): the locally-administered bit. When
  set, the address is not a vendor-assigned OUI.

machash controls these bits as follows:

- `-u/--unicast` clears only the multicast bit, converting the result
  to the nearest unicast address.
- `--local` sets the locally-administered bit; it does not touch the
  multicast bit.
- Neither flag changes the rest of the digest.

## Multicast warning

When the resulting MAC address is non-unicast and neither `-p` nor
`-u` was given, machash prints the address to stdout and a warning to
stderr explaining that multicast addresses are group addresses, and
how to get a unicast one (`-u`) or just the raw hash (`-p`).
`--local` does not suppress this warning. No warning is emitted for
locally-administered (vendor-unassigned) addresses.

[mac]: https://en.wikipedia.org/wiki/MAC_address
