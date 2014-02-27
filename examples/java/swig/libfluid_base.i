%module(directors="1") libfluid_base
%{
#include <fluid/OFConnection.hh>
#include <fluid/OFServerSettings.hh>
#include <fluid/OFServer.hh>

#include <arpa/inet.h>
%}

%ignore fluid_base::OFServer::free_data(void*);
%ignore fluid_base::OFConnection::get_application_data();
%ignore fluid_base::OFConnection::set_application_data(void*);

%typemap("jni") void* "jbyteArray"
%typemap("jtype") void* "byte[]"
%typemap("jstype") void* "byte[]"
%typemap("javadirectorin") void* "$jniinput"
%typemap("javain") void* "$javainput"

%typemap(directorin, descriptor="[B") void* {
    jbyteArray jb = (jenv)->NewByteArray(len);
    (jenv)->SetByteArrayRegion(jb, 0, len, (jbyte*) data);
    $input = jb;
}

%typemap(in) void* {
    $1 = (void *) JCALL2(GetByteArrayElements, jenv, $input, 0);
}

%apply short { uint8_t }

%feature("director", assumeoverride=1) OFServer;

%include "fluid/OFConnection.hh"
%include "fluid/OFServerSettings.hh"
%include "fluid/OFServer.hh"
