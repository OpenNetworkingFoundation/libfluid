# Introduction
> If you want to jump straight into the action, see the 
> [Quick start guide](md_doc_QuickStart.html).

**libfluid** is a library bundle that provides the basic features to implement 
an OpenFlow controller. It is composed of two separate libraries:

* **libfluid_base**: classes for creating an OpenFlow server that listens to 
connections and handles events

* **libfluid_msg**: classes for easily building and parsing OpenFlow 
wire format messages

These pages contain the documentation and API reference to the **libfluid** 
bundle. Extra documentation is available for **libfluid_msg** in the form of 
the 
[OpenFlow 1.0 and 1.3 specifications](https://www.opennetworking.org/sdn-resources/onf-specifications/openflow).

Both libraries are built in C++, and can be compiled and used independently.
You can use **libfluid_base** with any other OpenFlow messaging library, or you 
can also choose to integrate **libfluid_msg** alone into an existing controller 
(or driver). Everything is licensed under the Apache license, Version 2.0.

**libfluid** was selected as the winner of the 
[OpenFlow Driver Competition](https://www.opennetworking.org/competition) 
sponsored by ONF!

# Overview
> This is a very simple overview of **libfluid**. For more information and 
> examples, see the [Documentation](pages.html) page.

**libfluid_base** defines a client-server architecture, in which a controller 
is a server and a switch is a client. It provides a base class upon which you 
can build your controller (by inheritance): fluid_base::OFServer. You need to 
implement the callbacks for connection and message events, which will be called 
automatically.

Connections will be represented by fluid_base::OFConnection objects, which 
provide the abstraction for the most basic functionalities of an OpenFlow 
connection.

**libfluid_msg** (namespace fluid_msg) greatly simplifies message building and 
parsing in your controller. It provides classes to build OpenFlow messages with 
marshalling (pack) and unmarshalling (unpack) methods. Packing an object 
results in an OpenFlow message in the network byte order (wire format), ready 
to be sent through the OpenFlow connection. For unpacking, the library parses 
OpenFlow wire format data and sets the object attributes to the loaded values 
(dealing with byte ordering automatically).

Every message from **libfluid_msg** inherits from fluid_msg::OFMsg, which 
contains basic OpenFlow header information, allowing the user to create new 
OpenFlow messages by extending it. For example, for OpenFlow 1.0, vendor and 
experimenter messages can be easily implemented inheriting from 
fluid_msg::of10::Vendor and fluid_msg::of10::Experimenter respectively.

The image below illustrates how your implementation can use **libfluid**.

![Using libfluid](img/libfluid_simple.png)

# Getting started
For a quick overview on how to install both libraries and start using them, see 
the [Quick start guide](md_doc_QuickStart.html).

For a more detailed overview of **libfluid_base**, see 
[A quick intro to libfluid_base](md_libfluid_base_doc_Intro.html).

For a more detailed overview of **libfluid_msg**, see 
[A quick intro to libfluid_msg](md_libfluid_msg_doc_Intro.html).

The [Documentation](pages.html) page will help you with more documentation, 
sample code for building a controller, porting to other architectures, binding 
for other languages (Python and Java) and some extra information.

If you are building an application that uses or extends **libfluid**, check the 
[API reference](annotated.html).

The [FAQ](md_doc_FAQ.html) contains some trivia, explanations and known 
limitations.

If you're interested in contributing or diving into **libfluid**, checkout our 
repositories in GitHub:

* [libfluid_base](https://github.com/OpenNetworkingFoundation/libfluid_base)
* [libfluid_msg](https://github.com/OpenNetworkingFoundation/libfluid_msg)
* [libfluid](https://github.com/OpenNetworkingFoundation/libfluid)

The last repository, 
[libfluid](https://github.com/OpenNetworkingFoundation/libfluid), 
is the bundle that aggregates **libfluid_base** 
and **libfluid_msg**, providing extra documentation, examples and useful 
scripts for building. It is the starting point for **libfluid**.

The [repository guidelines](md_doc_RepositoryGuidelines.html) describe our 
development and versioning practices.

A [mailing list](https://groups.google.com/forum/#!forum/libfluid) is available 
for any questions and discussions involving **libfluid**. Issues and bugs 
should be reported directly in the GitHub repositories mentioned above.
