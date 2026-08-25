// Parse 48-bit MAC addresses.
#ifndef MAC_H
#define MAC_H

// Number of octets in a MAC address.
#define MAC_OCTETS 6

// Parse a MAC address of the form aa:bb:cc:dd:ee:ff. Returns 0 and
// stores the 48-bit value in out, or returns -1.
int mac_parse(const char *s, unsigned long long *out);

#endif
