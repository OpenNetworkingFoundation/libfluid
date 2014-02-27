# Android port of libfluid
This port proves that **libfluid** and its dependencies can be cross-compiled
to another architecture (ARM), and also showcases a neat (but rather useless)
OpenFlow controller running as an Android app.

It uses the application set of the **msg** controller from the `controller`
example and adds a controller (`AndroidController.cc`) that establishes a JNI
binding for Android.

## Building
To build it, you need the
[Android SDK](http://developer.android.com/sdk/index.html) and the
[Android NDK](http://developer.android.com/tools/sdk/ndk/index.html).

You will need the `android-8` platform (Android 2.2).

Then grab the sources for **libfluid_base**, **libfluid_msg** and
[libevent 2.0](http://libevent.org/) and place them in the `jni` folder.

Adjust the variables `LIBEVENT_DIR`, `LIBFLUID_BASE_DIR`, `LIBFLUID_MSG_DIR`
and `NDK_ROOT` in `build_libs.sh` if you need.

To build the libraries, just run:
~~~{.sh}
$ ./build_libs.sh
~~~

> We're building without TLS support. We've successfully built with SSL support 
> in our tests, but we didn't include it in the default configuration to make 
> it easier and faster. You can find commented code in `build_libs.sh` that 
> builds OpenSSL for Android.

Import the project in `android` to the Eclipse workspace (File, Import...,
General, Existing Projects into Workspace). If you copy the sources, make sure
you place and build the libraries in the copied location.

## Running

> This example uses the new, IANA-defined OpenFlow port: **6653** instead of 
> the previous, non-standard default (6633).

Just build and run the project as usual.

If you are running the Android SDK emulator, forward port 6653 in the virtual
device to your host so that OpenFlow switches can connect to it:
~~~{.sh}
$ telnet localhost 5554
redir add tcp:6653:6653
~~~

The app will display the log output for the controller on the screen.

## Known issues
**libfluid_base** and **libfluid_msg** are both compiled using static STL
implementations. We should be using a shared version for both to avoid
potential issues. But getting it to compile and run is troublesome this
way (that is, I don't know how to make it work).
