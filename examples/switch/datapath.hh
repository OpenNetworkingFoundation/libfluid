#ifndef DATAPATH_H
#define DATAPATH_H 1

#include <fluid/of10msg.hh>
#include "client/OFClient.hh"
#include "flow.hh"
#include <map>

using namespace fluid_base;
using namespace fluid_msg;

struct cmp_priority {
	bool operator() (const Flow &lhs, const Flow &rhs) const{  	  
	  return lhs.priority_ <= rhs.priority_;
	}
};

class SwPort;

class Datapath{
public:
	std::set<Flow, cmp_priority> flow_table;
	std::map<uint32_t, struct packet*> pkt_buffer;
	std::vector<SwPort*> ports;
	OFConnection **conn;
	/*current packet buffer id*/
	static uint32_t buffer_id;

	~Datapath();

	void handle_barrier_request(uint8_t* data);
	void action_handler(Action *act, struct packet *pkt);
	void handle_flow_mod(uint8_t* data);
	void handle_packet_out(uint8_t* data);

	//static void send_packet_in();

};

#endif

