CXX=c++
SWIG=swig
JAVA_INCLUDE_PATH=/usr/lib/jvm/java-7-openjdk-amd64/include

LearningSwitch: libfluid_base libfluid_msg_of10
	javac LearningSwitch.java

libfluid_base:
	mkdir -p fluid/base
	cd swig;\
	$(SWIG) -c++ -java -package fluid.base -I/usr/include libfluid_base.i;\
	$(CXX) -c libfluid_base_wrap.cxx -I$(JAVA_INCLUDE_PATH) -fPIC;\
	$(CXX) libfluid_base_wrap.o -o libfluid_base.so -shared -lfluid_base -fPIC;\
	mv *.java ../fluid/base;\
	mv *.so ../fluid;

libfluid_msg_of10:
	mkdir -p fluid/msg
	cd swig;\
	$(SWIG) -c++ -java -package fluid.msg -I/usr/include libfluid_msg_of10.i;\
	$(CXX) -c libfluid_msg_of10_wrap.cxx -I$(JAVA_INCLUDE_PATH) -fPIC;\
	$(CXX) libfluid_msg_of10_wrap.o -o libfluid_msg_of10.so -shared -lfluid_msg -fPIC;\
	mv *.java ../fluid/msg;\
	mv *.so ../fluid;

clean:
	cd swig;\
	ls | grep -v "\.i$$" | xargs rm -f;
	rm -rf fluid
	rm -rf *.class

.PHONY : jbase jmsg libfluid_msg_of10 libfluid_msg_of13 clean
