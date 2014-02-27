# Submission Requirements
Submissions to the competition should adhere to the following guidelines:

## COMPLIANCE

**The driver must be compliant with the ONF core specifications for OpenFlow v1.3.1, and demonstrate backward compatibility to 1.0, ensuring interoperability with existing equipment and controllers in the market.**

The driver works simultaneously with OpenFlow 1.0 and 1.3 and is compatible with existing tools and any other software or hardware that properly supports these versions.

**All optional capabilities described in the OpenFlow protocol should be implemented, unless an explanation is provided as to why a specific optional capability is not implemented.**

All capabilities are implemented (as far as a driver's responsibilities go). The only missing feature is UDP/DTLS support. It was not implemented because it was not clear to us (by reading the specification) what should be done in case of datagram loss, and also because no proper testing structure was available (software or hardware switches that support UDP). Libevent, which is used by **libfluid_base** currently doesn't support UDP (but there are talks to implement this feature).

## DEMONSTRATED FUNCTIONALITY
**The submission must include a sample controller demonstrating the integration of the driver.**

**The submission must provide learning switch functionality (when connected to an OpenFlow switch), either integrated with the sample controller, or provided as a separate application.**

Several variations of a simplified controller implementation are presented:

* **raw**: `cbench` and OpenFlow 1.0 learning switch (`l2`) applications that 
build the messages on their own, using C structs.

* **secure**: uses the **raw** learning switch application and provides an 
example of a controller that deals with secure and non-secure connections in 
different ports at the same time. It has been tested with Open vSwitch.

* **msg**: an implementation showcasing **libfluid_msg**. It provides a 
`cbench` application and a learning switch that works simultaneously with 
OpenFlow 1.0 and 1.3 switches (`l2`).

* **loci**: based on the **raw** learning switch application, but using Loxigen
generated sources to build the messages.

* **android**: the `msg` controller compiled to run in Android (ARM), 
demonstrating architecture portability. An APK is available (Android 2.3+).

**The submission must demonstrate the ability to integrate the driver into the OpenFlow switch agent for a physical switch.**

**The submission must demonstrate the ability to integrate the driver into the OpenFlow switch agent for a virtual switch.**

An example (`switch`) is provided. It simulates a switch and passes some of the basic OFTest tests (not all because switch implementation was not the focus of this competition).

The `switch` example uses a client implementation that adapts **libfluid_base** to act as an OpenFlow client rather than as a server (controller). It was not included in **libfluid_base** itself because we felt more testing and development were required.

This example could be adapted to run in either physical or software switches.


## CAPABILITIES
**The driver must be able to operate in a mixed environment against OpenFlow 1.3- and 1.0-compliant devices simultaneously.**

Implemented in fluid_base::OFServer, and possible to extend up to the maximum supported OpenFlow version (255). See the `controller` example (`Controller.hh` and `msg/MsgApps.cc`).

Version negotiation is done automatically for the user (but it can be optionally disable).

**The driver must be able to support multiple simultaneous connections.**

Intrisic to the architecture. The driver itself is agnostic to the connection peer. It could be another switch or a second connection from a switch that is already connected. Distinction between these cases is supposed to be done by the controller implementations through the normal OpenFlow mechanisms (features request).

## OWNERSHIP AND RIGHTS
**The winning entrant must assign to ONF rights to the code submitted. ONF will distribute the code under free and open-source licenses.**

It will be done if **libfluid** wins the competition.

**All entrants must be able to show proof that they can legally assign rights according to the above provision.**

Our code does not include any third party code that is not licensed accordingly.

**If the submission depends on third party libraries, the license(s) on those libraries must not prevent or limit ONF from distributing the entrant’s code.**

The dependencies (libevent and OpenSSL) are BSD/Apache licensed.

## TESTING – PROTOCOL AND APPLICATION
**The submission should demonstrate interoperability with existing OpenFlow tools such as Mininet and OFTest.**

**The entrant is responsible for providing a test harness that exercises the driver including the application of frames in the data plane and the resulting events impinging upon the controller. There is a strong preference for the use of Mininet or a similar simulation environment.**

**The submission will be evaluated by test cases (using network testing equipment) as well as integration into example physical and/or virtual network deployments.**

**libfluid** was tested in several ways:

In **Mininet**, several topologies were tested with the `controller` examples (e.g.: `single,3`, `linear,4`, `tree,depth=4,fanout=2`).

**Open vSwitch**, which is used by Mininet and some hardware switch implementations, was used directly as a software switch during development, especially for testing the TLS support.

**cbench** was used to measure performance during development.

**OFTest** compatibility is exercised in the `switch` example. Some of the tests of the `basic` set pass, though not all (they'd require too much effort on developing software switch features, which is not the direct goal of the competition).

**Physical switches** were used to run tests with the **msg** controller (that uses **libfluid_base** and **libfluid_msg**): Pronto 3290, Datacom DM4000 and NetFPGAs with the reference OpenFlow 1.0 implementation.

The **OpenFlow 1.3 software switch** implemented by CPqD was used to test OpenFlow 1.3 features. This version wasn't tested on hardware because we don't have switches that supports it.

**Test cases** (we're using [googletest](http://code.google.com/p/googletest/)) that exercise most parts of **libfluid_msg** are provided.

Simulated **network events** such as disconnections due to link failure, user action or version negotiation failure were used to test the expected behavior.


## INTEGRATION AND PORTING
**The submission must demonstrate the ability to be integrated into multiple language environments such as C/C++, Python, Java, and/or Ruby. The submitted code should provide two example bindings using a non-native language (e.g. if the code is written in C++, then examples showing bindings for Python and Ruby need to be provided).**

Bindings for Java and Python are provided as examples (with learning switches implemented). The bindings are generated using SWIG.

**The driver must run on a standard Linux or Windows operating system.**

It runs on Linux (tested on Ubuntu 12.04, Fedora 19 and Android). It should be easy to port to Windows and other OS, since its dependencies are available and work well in various platforms.

**The submission should demonstrate the ability of the driver to be compiled for a non-x86 CPU such as an ARM, MIPS, or PowerPC.**

Compiles in ARM (`android` example).

**ONF has a preference for code written in C/C++.**

The code is written in C++, making heavy use of object orientation features.

## BASIC EVALUATION CRITERIA
**The driver will be evaluated on performance, scalability, clarity of code, portability of code, extensibility, architecture, and lack of defects.**

**Performance**
It's up to par with existing controllers. **libfluid** is faster in some cases, but there's room for improvement in other parts. See the [benchmarks](md_doc_Benchmarks.html) for more details.

**Scalability**
Event-loop libraries (such as libevent and libev) are used by real-world networking applications, such as memcached, Tor and Google Chrome, and are known to scale very well. We use [libevent](http://libevent.org/), and in some tests (using `cbench`) we've been able to run with 1000 simulated switches without significant performance loss.

**Clarity of code**
We believe the code is very simple and clear (though not perfect). A good amount of documentation is also provided.

**Portability of code**
The code has been tested on Linux (x86 and ARM), but there's no use of features or dependencies that would cause trouble when porting to other platforms and architectures.

**Extensibility**
The API is modular and extensible through inheritance (the `client` example shows this).

**Architecture**
Can't be justified, but we believe it is documented well enough to assist in the evaluation.

**Lack of defects**
Knowing it's impossible to make bug-free software, tests, examples and memory checking were regularly exercised during development in order to detect and eliminate problems.

**The entrant may demonstrate the code in a laboratory other example deployment.**
The `msg` controller example was tested on three OpenFlow physical switches, as well as several simulated Mininet topologies. Other tests are mentioned in the Testing section.

**Specific criteria and evaluation mechanisms will be at the discretion of the judges.**
.
