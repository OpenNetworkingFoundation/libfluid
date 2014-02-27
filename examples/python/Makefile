CXX=c++
SWIG=swig

all: libfluid_base libfluid_msg_of10 libfluid_msg_of13

libfluid_base:
	mkdir -p fluid/base
	touch fluid/__init__.py
	cd swig;\
	$(SWIG) -c++ -python -threads -I/usr/include libfluid_base.i;\
	$(CXX) -c libfluid_base_wrap.cxx -I/usr/include/python2.7/ -fPIC;\
	$(CXX) libfluid_base_wrap.o -o _libfluid_base.so -shared -lfluid_base -lpython2.7 -fPIC;\
	mv _libfluid_base.so ../fluid/base;\
	mv libfluid_base.py ../fluid/base/__init__.py;

libfluid_msg_of10:
	mkdir -p fluid/msg
	touch fluid/__init__.py
	touch fluid/msg/__init__.py
	cd swig;\
	$(SWIG) -c++ -python -I/usr/include of10.i;\
	$(CXX) -c of10_wrap.cxx -I/usr/include/python2.7/ -fPIC;\
	$(CXX) of10_wrap.o -o _of10.so -shared -lfluid_msg -lpython2.7 -fPIC;\
	mv _of10.so ../fluid/msg;\
	mv of10.py ../fluid/msg;

libfluid_msg_of13:
	mkdir -p fluid/msg
	touch fluid/__init__.py
	touch fluid/msg/__init__.py
	cd swig;\
	$(SWIG) -c++ -python -I/usr/include of13.i;\
	$(CXX) -c of13_wrap.cxx -I/usr/include/python2.7/ -fPIC;\
	$(CXX) of13_wrap.o -o _of13.so -shared -lfluid_msg -lpython2.7 -fPIC;\
	mv _of13.so ../fluid/msg;\
	mv of13.py ../fluid/msg;

clean:
	cd swig;\
	ls | grep -v "\.i$$" | xargs rm -f;
	rm -r fluid

.PHONY : libfluid_base libfluid_msg_of10 libfluid_msg_of13 clean
