# Example controller implementations
This is an implementation of a very simple OpenFlow controller using 
**libfluid**.
It builds on top of `fluid_base::OFServer` to provide controller-like
functionalities (it needs a lot more work to become a real controller).

Note that these controllers will run each event to completion. That is: once a
message arrives, its response will be created and sent during the callback 
execution. This means other connections running in the same thread will have
to wait. This provides a general use case that is fine for an example and good
for general use and benchmarking. When designing a real controller, make sure 
that long operations don't block the callbacks, and be aware of the guidelines 
in the API documentation (see fluid_base::OFServer::message_callback).

Four controller variations are available:
* **raw**: OpenFlow 1.0 learning switch (`l2`) and `cbench` fast reply 
applications that build the messages on their own, using raw OpenFlow structs.

* **secure**: uses the **raw** learning switch application and provides an 
example of a controller that deals with secure and non-secure connections in 
different ports at the same time. It has been tested with Open vSwitch.

* **msg**: an implementation showcasing **libfluid_msg**. It provides a 
`cbench` fast reply application and a learning switch that works simultaneously 
with OpenFlow 1.0 and 1.3 switches (`l2`).

* **loci**: based on the **raw** learning switch application, but using LOCI 
for message generation. To make it compile and work, move the LOCI generated 
sources (`loxi_output/loci`) to a folder named **loci**.

> These examples use the new, IANA-defined OpenFlow port: **6653** instead of 
> the previous, non-standard default (6633).

> Note that two applications are available for the **raw** and **msg** 
> controllers. You should indicate which one you want (`l2` or `cbench`).

To build and run any of the variations:
~~~{.sh}
$ make controller
$ make raw_controller
$ make secure_controller
$ make loci_controller

$ ./controller l2
$ ./raw_controller l2
$ ./secure_controller
$ ./loci_controller
~~~

After you start the controller, point your switches to it. You can use Mininet 
or OpenFlow switches to test functionality.

> The learning switch flows last for only 10 seconds (5 if idle) to make tests
> more interesting.
 
In order to test and compare performance, you can use `cbench`:
~~~{.sh}
$ cbench -c localhost -p 6653 -m 1000 -l 10 -s 16 -M 1000000 -t
~~~

To see how fast **libfluid** can go, run the **raw** controller with the 
`cbench` fast reply application. It replies to packet-in messages with dummy 
flow mod messages in order to measure IO alone. Linking with TCMalloc further 
improves performance.

To make a fairer comparison with other controllers running a learning switch 
application, run the **msg** controller with the `l2` app. This adds the 
overhead for message creation and parsing, as well as hash map lookup. Similar 
applications are built into pretty much every OpenFlow controller.
