# Output formats

## MAC address (default)

machash prints the 48-bit digest as six colon-separated lowercase hex
octets, most significant octet first:

    0f:c2:c1:58:42:59

This is the Unix-style form. See the Wikipedia article on
[MAC addresses][mac] for the octet and bit layout.

## Plain

`-p/--plain` prints the digest as 12 lowercase hex digits with no
separators and no `0x` prefix:

    0fc2c1584259

## 0x prefix

`--0x` prints the digest as 0x followed by 12 lowercase hex digits:

    0x0fc2c1584259

## Swapped octets

`-S/--swap` prints the six octets in reversed order:

    59:42:58:c1:c2:0f

The output is a MAC address form, so the multicast warning applies
as for the default form.

## Format selection

Only one output format can be selected: the default MAC form,
`-p/--plain`, `--0x`, or `-S/--swap`. `--check` is a mode, not a
format.

## Address bits

The first octet carries two semantic bits (see the Wikipedia article
on [MAC addresses][mac]):

- Bit 0 (0x80 in the first octet): the multicast bit. When set, the
  address is a group address rather than a device address.
- Bit 1 (0x40 in the first octet): the locally-administered bit. When
  set, the address is not a vendor-assigned OUI.

machash controls these bits as follows:

- `-u/--unicast` clears only the multicast bit. The result becomes
  the nearest unicast address.
- `--local` sets the locally-administered bit. It does not touch the
  multicast bit.
- Neither flag changes the rest of the digest.

## Multicast warning

The warning applies to the address-like output forms (the default
MAC form and `-S/--swap`). If the result is non-unicast and you
give neither `-u` nor a raw format, machash prints the address to
stdout. It also prints a warning to stderr. The warning explains
that multicast addresses are group addresses. It shows how to get
a unicast address (`-u`) or the raw hash (`-p` or `--0x`).
`--local` does not suppress this warning. machash emits no warning
for locally-administered (vendor-unassigned) addresses.

[mac]: https://en.wikipedia.org/wiki/MAC_address
