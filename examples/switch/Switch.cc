#include "Switch.hh"
#include <iostream>

uint64_t str_to_uint64(char *str){
	return strtoull (str, NULL, 16);
}


int main(int argc, char **argv) {
	std::vector<std::string> ports;
	if(argc < 2){
		std::cout << "Usage: switch -i [interface1 interface2 ...] -d [DPId]" << std::endl;
		exit(0);
	}    
	if(strcmp(argv[1],"-i")){
		std::cout << "Invalid argument " << argv[1] << std::endl;
		exit(0);
	}
	int i = 0;
	bool with_dp = false;
	for(i = 2; i < argc; ++i){
		if(!strcmp(argv[i], "-d")){			
			with_dp = true;
			break;
		}
		ports.push_back(argv[i]);
	}
	Switch *sw;
	if(with_dp){
		uint64_t dpid;
		if( argv[i+1] == NULL || !(dpid = str_to_uint64(argv[i+1]))){
			std::cout << "Invalid datapath id. Valid example: 0x0000000000000001" << std::endl;			
			ports.clear();
			exit(0);
		}
		sw = new Switch(0, "127.0.0.1", 6653, ports, dpid);
	}
	else {
    	sw = new Switch(0, "127.0.0.1", 6653, ports);
    }
    
    sw->start();    
    wait_for_sigint();
    sw->stop();
    delete sw;
    return 0;
}

