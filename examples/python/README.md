# Python bindings for libfluid
This is an implementation of the Python bindings for **libfluid**.

The `Makefile` for this example assumes you have **libfluid_base** and 
**libfluid_msg** installed in your system with the `/usr` prefix. It also needs 
Swig 2.0 and the Python development headers (`python-dev` package on Ubuntu 
12.04). 

So do what's needed first, and then run:
~~~{.sh}
$ make
$ python of10switch.py
~~~

A module named `libfluid` is generated, with submodules `base` and `msg` (which
is divided in the submodules `of10` and `of13`).

`ofswitch10.py` has a learning switch test application for OpenFlow 1.0. The 
OpenFlow 1.3 version is in the file `of13switch.py`.

> This example uses the new, IANA-defined OpenFlow port: **6653** instead of 
> the previous, non-standard default (6633).
