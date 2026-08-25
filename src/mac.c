// Parse 48-bit MAC addresses.
#include "mac.h"

// Returns the value of a hex digit, or -1.
static int hexval(char c) {
  if (c >= '0' && c <= '9') {
    return c - '0';
  }
  if (c >= 'a' && c <= 'f') {
    return c - 'a' + 10;
  }
  if (c >= 'A' && c <= 'F') {
    return c - 'A' + 10;
  }
  return -1;
}

int mac_parse(const char *s, unsigned long long *out) {
  unsigned long long v = 0;
  const char *p = s;
  for (int i = 0; i < MAC_OCTETS; i++) {
    int hi = hexval(p[0]);
    int lo = hexval(p[1]);
    if (hi < 0 || lo < 0) {
      return -1;
    }
    v = (v << 8) | (unsigned long long)(hi << 4) | (unsigned long long)lo;
    p += 2;
    if (i < MAC_OCTETS - 1) {
      if (*p != ':') {
        return -1;
      }
      p++;
    }
  }
  if (*p != 0) {
    return -1;
  }
  *out = v;
  return 0;
}
