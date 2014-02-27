# Quick start

This page shows the absolute minimum to get **libfluid** running.

> We are assuming Ubuntu 12.04 for the steps below. Detailed instructions for 
> other distributions (Fedora only for now) are available in other pages: 
> [A quick intro to libfluid_base](md_libfluid_base_doc_Intro.html) and
> [A quick intro to libfluid_msg](md_libfluid_msg_doc_Intro.html).

Install the dependencies:
~~~{.sh}
$ sudo apt-get install autoconf libtool build-essential pkg-config
$ sudo apt-get install libevent-dev libssl-dev
~~~

Clone the **libfluid** repository:
~~~{.sh}
$ git clone https://github.com/OpenNetworkingFoundation/libfluid.git
$ cd libfluid
$ ./bootstrap.sh
~~~

The bootstrap script will clone both libraries that form **libfluid**: 
**libfluid_base**, which deals with OpenFlow control channel, and 
**libfluid_msg**, which provides classes for building and parsing OpenFlow 
messages. The bootstraping process will also checkout the repositories to 
stable versions of both libraries.

Build and install **libfluid_base**:
~~~{.sh}
$ cd libfluid_base
$ ./configure --prefix=/usr
$ make
$ sudo make install
~~~
> For more information, see [A quick intro to libfluid_base](md_libfluid_base_doc_Intro.html).

Build and install **libfluid_msg**:
~~~{.sh}
$ cd libfluid_msg
$ ./configure --prefix=/usr
$ make
$ sudo make install
~~~
> For more information, see [A quick intro to libfluid_msg](md_libfluid_msg_doc_Intro.html).

Build and run the sample controller:
~~~{.sh}
$ cd examples/controller
$ make msg_controller
$ ./msg_controller l2
~~~
> For more information, see [Example controller implementations](md_examples_controller_README.html).

Start [Mininet](http://mininet.org/) and run:
> Change [IP] to the IP address on which you are running `msg_controller`.
~~~{.sh}
$ sudo mn --topo tree,depth=2,fanout=2 --controller=remote,ip=[IP],port=6653
mininet> pingall
mininet> dpctl dump-flows
~~~

That's it. Now check out the [Documentation](pages.html) page to learn more
about **libfluid** and the other examples.

If you are building an application that uses or extends **libfluid**, check the 
[API reference](annotated.html).
