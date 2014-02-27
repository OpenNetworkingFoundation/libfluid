# FAQ

## Where did the name come from?
Failing to come up with a good name, we decided to call it "fluid" during 
development, since it could or could not be the final name. But it didn't 
change, and it relates nicely to the idea of flows and OpenFlow, so it stuck.

## Why was libfluid created?
**libfluid** was coded by two programmers at CPqD (Allan Vidal and Eder 
Fernandes) to be submitted to the ONF driver competition.

## Why is libfluid divided in two?
The division in functionality came naturally during development, and we decided 
to treat it as a feature. Each sub-library can be used independently, though 
they share common examples and documentation. Together they can be used to 
simplify the process of creating an OpenFlow controller.

## Which versions of the OpenFlow protocol does libfluid implement?
**libfluid_msg** implements OpenFlow 1.0 and 1.3 because that's what the ONF 
driver competition asked for. But it can (and will) be extended to deal with 
newer or older versions.

**libfluid_base** works independently of OpenFlow protocol versions, assuming 
only basic, unchanging message headers and handshake steps.

## Why was *feature X* done that way?
The library was developed with a few things always in mind:
* Keeping it short: common use cases require as few lines of code as possible

* Keeping it simple: the API should be simple to understand and extend; it 
should also feel pretty and be **very** well documented

* Keeping it fast: it should be fast, but respecting the other two principles 
first

## Why is libfluid using port 6653 in its examples?
OpenFlow 1.4 defines a new port number for the OpenFlow control channel: 
**6653**.

The previous port number (6633) was not registered in the IANA port database, 
so its use should be deprecated.

We've decided to adopt this change retroactively: even though we still only 
implement versions 1.0 and 1.3 of the OpenFlow protocol, our examples use the
new port number. However, you're free to use any other value. Neither 
**libfluid_base** nor **libfluid_msg** assume any values regarding port 
numbers.

## What are the limitations of libfluid?
This is a rough list of known issues. For details, see the source code.
* No support for UDP/DTLS
* Logging is done to stderr (will be fixed soon)
* libfluid_tls_clear leaves some bytes on shutdown
* libevent_pthreads leaves a few bytes on shutdown
* **libfluid_msg** is missing message printing
* The bindings are presented as a proof of concept, they need more work
