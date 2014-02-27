# Secure connections with libfluid
It is possible to use TLS on top of **libfluid**. The implementation was tested
with Open vSwitch 1.4.0 (default version for Ubuntu 1.4.0). The
`controller/secure` example shows how to setup TLS in **libfluid**.

**libfluid** must be compiled with TLS support for this feature to work. This
is the default; unless you have explicitly passed the `--disable-tls` flag to
`configure`, you don't need to worry.

To get started, follow the [OVS tutorial for TLS](http://git.openvswitch.org/cgi-bin/gitweb.cgi?p=openvswitch;a=blob_plain;f=INSTALL.SSL;hb=HEAD).

You will need three files created in the tutorial above:

* `ctl-cert.pem`: this is the controller certificate, signed by the
controllerca. This will be sent to the switches so that they can verify the
controller can be trusted and establish communication.

* `ctl-privkey.pem`: this is the controller private key, used to encrypt data.

* `switchca/cacert.pem`: this is the file created by Open vSwitch for the CA
signing switch certificates. We will need it to verify that the switches
connecting to us are trusted. On Ubuntu 12.04, they will tipically be at
`/usr/share/openvswitch/pki/switchca/cacert.pem`.

> Don't use the self-signed certificates scheme. **libfluid** needs a full PKI.

> There's a script in this example (`controller/secure/switch.sh`) to start OVS 
> using the right parameters. You just need to create the PKI and the keys.

Once you have created these three files, starting your secure 
fluid_base::OFServer instance is really easy:
~~~{.cc}
using namespace fluid_base;

libfluid_tls_init("/path/to/ctl-cert.pem",
                  "/path/to/ctl-privkey.pem",
                  "/path/to/ovs/pki/switchca/cacert.pem");

OFServer srv("localhost", 6653, 2, true);

[...]

libfluid_tls_clear();
~~~

The only thing that changes is the fourth argument to fluid_base::OFServer:
`secure`.

This parameter will make all connection on this fluid_base::OFServer instance 
be secured with SSL, as long as you call fluid_base::libfluid_tls_init first.

The parameters to fluid_base::libfluid_tls_init are valid for **all**
fluid_base::OFServer instances in the program that use the `secure` option.

Calling fluid_base::libfluid_tls_clear at the end of the execution frees some 
data allocated internally by the SSL library.

If you see this error:
~~~
Error loading private key.
~~~

You probably need to run the controller with the right privileges for reading
the private key.

**libfluid** uses OpenSSL for providing TLS.
