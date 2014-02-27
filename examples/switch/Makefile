CXX=c++

switch:
	$(CXX) -g client/OFClient.cc client/base/BaseOFClient.cc packets.h *.hh *.cc -Iclient/base -lfluid_base -lfluid_msg -levent -levent_pthreads -lpcap -o switch

clean:
	rm -f switch

install:
	install -m 0755 switch /usr/local/bin
    
.PHONY : clean switch install
