/* A little and dumb OpenFlow 1.0 switch implemented only for demonstration purposes.
The switch is made to work with a */

#define __STDC_FORMAT_MACROS
#include <net/ethernet.h>
#include <signal.h>
#include "flow.hh"
#include "port.hh"

#define TABLE_NUM 32


using namespace fluid_base;
using namespace fluid_msg;

uint64_t generate_dp_id() {
	srand (time(NULL));
	return (((uint64_t) rand() <<  0) & 0x000000000000FFFFull) ^ 
		  (((uint64_t) rand() << 16) & 0x00000000FFFF0000ull) ^ 
		  (((uint64_t) rand() << 32) & 0x0000FFFF00000000ull) ^
		  (((uint64_t) rand() << 48) & 0xFFFF000000000000ull);
}

volatile sig_atomic_t quit = false;

void sigint_handler(int) {
   quit = true;
}

void wait_for_sigint() {
   struct sigaction sa;
   memset(&sa, 0, sizeof(sa));
   sa.sa_handler = sigint_handler;
   sigfillset(&sa.sa_mask);
   sigaction(SIGINT, &sa, NULL);

   while (1) {

       if (quit) {
       	break;
       }
       sleep(1000);
   }
}

class Switch: public OFClient{
	private:
		Datapath *dp;
		pthread_t *threads;	
	public:		
		std::vector<SwPort> ports;						

		~Switch(){
			for(std::vector<SwPort*>::iterator it = this->dp->ports.begin();
		  		it != this->dp->ports.end(); ++it){				  					  					  					  	
				  	delete (*it);				  	  								
			}	
			delete dp;
			delete [] this->threads; 
		}

		Switch(const int id = 0, std::string address = "127.0.0.1", 
			  const int port = 6653, std::vector<std::string> ports = 
			  std::vector<std::string>(),  uint64_t dp_id = generate_dp_id()):
			OFClient(id, address, port, dp_id, OFServerSettings().supported_version(0x01)) {
				/*Create ports threads*/						
				this->dp = new Datapath();
				this->dp->conn = &this->conn;				
				for(int i = 0; i < ports.size(); i++){					
					SwPort *port = new SwPort(i+1, ports[i]);										
					this->dp->ports.push_back(port);
				}				
				this->threads = new pthread_t[this->dp->ports.size()];
				/*Open interfaces to capture packets*/						
				for(std::vector<SwPort*>::iterator it = this->dp->ports.begin();
	  				it != this->dp->ports.end(); ++it){			  	
			  		(*it)->open();
				  	struct pcap_datapath *pcap_dp =  new pcap_datapath();
				  	pcap_dp->dp =  &this->dp;
				  	pcap_dp->port_no = (*it)->port_no();			  	
				  	pcap_dp->pcap_ = (*it)->pcap();
				  	int err = pthread_create(&threads[(*it)->port_no()-1], NULL, 
				  	SwPort::pcap_capture, (void*)pcap_dp);						
				}
		}

		bool start(bool block = false) {								
			return OFClient::start(block);
    	}

    	void stop() {
    		int i = 0;
			for(std::vector<SwPort*>::iterator it = this->dp->ports.begin();
		  		it != this->dp->ports.end(); ++it){				  					  					  					  	
				  	(*it)->close();				  	
				  	void *pcap_dp;				  	
				  	pthread_join(threads[(*it)->port_no()-1], &pcap_dp);
				  	delete (struct pcap_datapath*) pcap_dp;				  					  					  					  	    		  
			}						     				
        	OFClient::stop();        	
    	}

	void connection_callback(OFConnection* conn, OFConnection::Event event_type) {		
		if(event_type == OFConnection::EVENT_CLOSED){
			OFClient::stop_conn();
			OFClient::start_conn();						
		}
	}

 	void message_callback(OFConnection* conn, uint8_t type, void* data, size_t len) {
 		//Message handlers
		if(type == of10::OFPT_BARRIER_REQUEST){			
			this->dp->handle_barrier_request((uint8_t*)data);
		}
		if (type == of10::OFPT_PACKET_OUT){			
			this->dp->handle_packet_out((uint8_t*)data);
		}
		if (type == of10::OFPT_FLOW_MOD){
			this->dp->handle_flow_mod((uint8_t*)data);
		}
 	}
	
};
