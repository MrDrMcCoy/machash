#!/bin/sh
# Validate the independent Bobcat oracle (build/bobcat_ref) against
# the four published reference vectors. This certifies the oracle
# before its output is used as expected values in integration.sh.
set -eu

CC=${CC:-cosmocc}
REF=build/bobcat_ref
fail=0

if [ ! -x "$REF" ]; then
  mkdir -p build
  $CC -O2 -std=gnu11 -o "$REF" tests/ref/bobcat_ref.c
fi

check() {
  name=$1
  want=$2
  shift 2
  # Chunks are hard-coded vector bytes (16 bytes each), not user input.
  fmt=$(printf '%s' "$@")
  # shellcheck disable=SC2059
  got=$(printf "$fmt" | "$REF")
  if [ "$got" = "$want" ]; then
    echo "ok: $name"
  else
    echo "FAIL: $name got=$got want=$want" >&2
    fail=1
  fi
}

# Little-endian byte strings of the published 16-bit word vectors.
check v1 0x2bce98fa6186 \
  '\040\131\314\307\064\142\021\261\220\060\267\212\163\363\335\106' \
  '\323\065\156\300\000\000\000\000\000\000\000\000\000\000\000\000'
check v2 0x75ebe721c8cd \
  '\103\276\306\203\172\001\161\076\245\104\237\012\111\063\212\054' \
  '\204\017\347\154\000\000\000\000\000\000\000\000\000\000\000\000'
check v3 0xf20aec83c477 \
  '\067\277\070\306\240\027\161\037\245\344\236\152\111\064\252\054' \
  '\204\017\347\154\170\065\375\140\200\247\250\201\300\147\237\170' \
  '\064\276\146\074\172\101\161\076\245\104\237\012\111\063\212\054' \
  '\103\017\347\014\041\103\064\022\043\376\012\013\000\000\000\000'
check v4 0x7e5abf5fad22 \
  '\001\244\054\353\215\230\244\350\047\277\233\030\061\262\252\057' \
  '\274\043\064\375\063\040\007\261\177\171\320\351\026\075\066\362' \
  '\161\270\377\205\133\331\041\207\362\164\107\325\221\117\014\157' \
  '\150\076\233\124\323\373\075\236\151\310\077\131\333\165\152\154'

if [ "$fail" -eq 0 ]; then
  echo "check_oracle: all vectors match"
fi
exit "$fail"
