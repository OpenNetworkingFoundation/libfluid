%module(directors="1") libfluid_base
%{
/* Includes the header in the wrapper code */
#include <fluid/OFConnection.hh>
#include <fluid/OFServerSettings.hh>
#include <fluid/OFServer.hh>
#include <fluid/OFServerSettings.hh>
#include <arpa/inet.h>
%}

%ignore fluid_base::OFServer::free_data(void*);
%ignore fluid_base::OFConnection::get_application_data();
%ignore fluid_base::OFConnection::set_application_data(void*);

/* Map void* data to Python bytes for OFServer::message_callback data */
%typemap(directorin) (void* data) {
  Py_ssize_t size = htons(*((uint16_t*) data + 1));
  $input = PyBytes_FromStringAndSize((char*) data, size);
}

%typecheck(SWIG_TYPECHECK_VOIDPTR) void* {
   $1 = PyBytes_Check($input) ? 1 : 0;
}

%feature("director", assumeoverride=1) OFServer;

/* Map uint8_t to a Python int for OFServer::message_callback type */
%apply int { uint8_t }
/* Accept str/bytes in OFConnection::send */
%apply char* { void* }

%include "fluid/OFConnection.hh"
%include "fluid/OFServerSettings.hh"
%include "fluid/OFServer.hh"
%include "fluid/OFServerSettings.hh"
