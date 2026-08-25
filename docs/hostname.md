# Hostname input

`-n/--hostname` and `-i/--interface` take the input from the machine
instead of the command line. They are useful for deriving one stable
MAC address per host, or one per host and interface.

## -n/--hostname

machash gets the name of this host with gethostname(2) and uses it as
the single input:

    $ machash -n
    7f:b8:8e:31:f2:d8

The example value is the hash of lappy486, the host name of the
machine that wrote this file. Your host gives your own name.

## -i/--interface IFACE

The input is the host name and IFACE, joined by a colon:

    $ machash -i eth0
    9e:02:d0:e6:1e:64

The example value is the hash of lappy486:eth0.

-i already includes the host name, so -n adds nothing when -i is
given. The interface name is used as given, with surrounding
whitespace removed. machash does not check that the interface exists.

## Combining with other options

Hostname mode gives exactly one input. It cannot be combined with
-s, positional arguments, or line mode (-l, -f). Data piped on stdin
is ignored.

The output format flags (-p, --0x, -S), the bit flags (-u, --local),
and --check MAC all work with hostname mode, as with any input.
