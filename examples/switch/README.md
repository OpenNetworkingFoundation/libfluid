# Example switch implementation
This is an implementation of a very simple switch using **libfluid** . 

It is built on top of `BaseOFClient` (which is analogous to
`fluid_base::BaseOFServer`). This code (which is in the `client` directory) was 
not included in **libfluid_base** because it was not considered mature enough, 
but it will be, eventually.

There's a `Makefile` in the `client` folder for compiling a client that just 
establishes and keeps a connection to an OpenFlow controller. You need to 
extend it to do anything useful.

## Building

Along with the **libfluid** library requisites you will also need to install 
the pcap library with development headers.

On Ubuntu 12.04:
~~~{.sh}
$ sudo apt-get install libcap-dev
~~~

After installing all dependencies, build the switch:
~~~{.sh}
$ make
~~~

## Running

> This example uses the new, IANA-defined OpenFlow port: **6653** instead of 
> the previous, non-standard default (6633).

To run the switch you need to list the interfaces that will be used as OpenFlow
ports. As the switch does not create interfaces, the names given to the command
should be those of existent interfaces that can be put on promiscuous mode.

Start the switch with the interfaces for the ports. As interfaces will be set 
to promiscuous the command needs to be run as root:
~~~{.sh}
$ sudo ./switch -i veth0 veth2 veth4 veth6
~~~

Optionally, it is possible to specify the switch datapath ID. If you don't, a 
random one will be generated for you.
~~~{.sh}
$ sudo ./switch -i veth0 veth2 veth4 veth6 -d 0x0000000000000001
~~~

## Testing with OFTest

The switch passes on a few  basic OFTest cases, demonstrating basic 
functionality.

To start the environment, run the script run_switch.py. It will create the 
virtual interfaces expected by OFTest and run the switch with the created 
ports:
~~~{.sh}
$ sudo ./run_switch
~~~

Then go to the OFTest directory and run the basic tests:
~~~{.sh}
$ sudo ./oft basic.Echo
$ sudo ./oft basic.PacketIn
$ sudo ./oft basic.PacketOut
$ sudo ./oft basic.FlowMod
~~~

## Testing with Mininet

It is possible to use this switch in Mininet. Replace the Mininet file 
`mininet/bin/mn` and `mininet/mininet/node.py` with the ones in the 
`MininetFiles`.

Install Mininet again:
~~~{.sh}
$ sudo ./util/install.sh -n
~~~

Now, build and install the switch so that Mininet can find it:
~~~{.sh}
$ sudo make
$ sudo make install
~~~

And start Mininet:
~~~{.sh}
$ sudo mn --topo single,3 --mac --switch fluid --controller=remote,ip=127.0.0.1,port=6653
~~~

The switch functionality is enough to run a simple learning switch application, 
like the one present in the `controller` example or the POX l2_forwarding 
application. 
