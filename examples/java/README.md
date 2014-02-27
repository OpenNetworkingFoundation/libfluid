# Java bindings for libfluid_base
This is an implementation of a Java binding for **libfluid**.

The `Makefile` for this example assumes you have **libfluid_base** and 
**libfluid_msg** installed in your system with the `/usr` prefix. It also needs 
Swig 2.0 and the JDK (`openjdk-7-jdk` package on Ubuntu 12.04). 

So do what's needed first, and then run:
~~~{.sh}
$ make
$ LD_LIBRARY_PATH=./fluid:$LD_LIBRARY_PATH java LearningSwitch
~~~

It is an OpenFlow 1.0 learning switch test application.

The Java code is in `LearningSwitch.java`.

> This example uses the new, IANA-defined OpenFlow port: **6653** instead of 
> the previous, non-standard default (6633).
