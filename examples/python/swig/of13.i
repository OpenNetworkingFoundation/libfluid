%module(directors="1") of13
#pragma SWIG nowarn=362,503,509
%{
/* Includes the header in the wrapper code */
typedef uint32_t of_error;
#include <fluid/of13msg.hh>
%}
%include "stdint.i"
%include "cdata.i"

%apply SWIGTYPE *DISOWN { fluid_msg::Action*};
%apply SWIGTYPE *DISOWN { fluid_msg::of13::OXMField*};
%apply SWIGTYPE *DISOWN { fluid_msg::TableFeatureProp*};
%apply SWIGTYPE *DISOWN { fluid_msg::QueueProperty*}
%apply SWIGTYPE *DISOWN { fluid_msg::of13::Instruction*}

%typemap(out) (uint8_t* buffer) %{
   $result = (void*) $input;
%}

%typemap(out) uint8_t* {
  Py_ssize_t size = htons(*((uint16_t*) $1 + 1));
  $result = PyBytes_FromStringAndSize((char*) $1, size);
}

%typemap(in) uint8_t* {
   char* buffer;
   Py_ssize_t length = PyBytes_Size($input);
   PyBytes_AsStringAndSize($input, &buffer, &length);
   $1 = (uint8_t*) buffer;
}

%pythoncode %{
btom = lambda b : ":".join([c.encode("hex") for c in b])
%}

%apply char* {uint8_t *data};
%include "fluid/util/ethaddr.hh"
%include "fluid/util/ipaddr.hh"
%clear uint8_t *data;

%include "fluid/ofcommon/action.hh"
%include "fluid/ofcommon/common.hh"
%include "fluid/ofcommon/msg.hh"

%include "fluid/of13/of13match.hh"
%include "fluid/of13/of13instruction.hh"
%include "fluid/of13/of13action.hh"
%include "fluid/of13/of13common.hh"
%include "fluid/of13/of13meter.hh"

%include "fluid/of13msg.hh"

typedef uint32_t of_error;
