# Check mode

`--check MAC` compares the inputs to an existing MAC address.

## MAC syntax

MAC is six colon-separated hex octets, for example
0f:c2:c1:58:42:59. Upper and lower case are both accepted. An
invalid MAC address is a usage error with a descriptive message.

## Comparison

machash hashes each input and applies -u/--local to the result, as
in normal output mode. It then compares the 48-bit result to MAC.
A match means the input hashes to exactly this address with these
flags.

## Output and exit status

machash prints one line per input, in input order: match or no
match. The exit status is 0 if all inputs match, 1 if any input
does not match. The multicast warning is not printed in check
mode.

## Examples

    machash --check 0f:c2:c1:58:42:59 -s hello
    match

    echo hello | machash --check 0f:c2:c1:58:42:59
    match

    machash --check 0e:be:ff:92:0c:64 -u -s baz
    match
