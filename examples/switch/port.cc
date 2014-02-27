#include "port.hh"

SwPort::SwPort(int port_no, std::string name):port_no_(port_no), 
				name_(name), pcap_(NULL){
}

void* SwPort::open(){
	char errbuf[PCAP_ERRBUF_SIZE];
		/* open device for reading. NOTE: defaulting to
	 	* promiscuous mode*/
	this->pcap_ = pcap_open_live(this->name_.c_str(), BUFSIZ, true, 100, errbuf);
	if (this->pcap_ == NULL) { 
		fprintf(stderr, "pcap_open_live(): %s\n", errbuf); exit(1);
	}	
    
    if (pcap_setdirection(this->pcap_ , PCAP_D_IN) != 0) {
        fprintf(stderr, "pcap_setdirection %s", pcap_geterr(this->pcap_));
        return NULL;
    }
}

void SwPort::close(){
	pcap_close(this->pcap_);
}

void SwPort::port_output_packet(uint8_t *pkt, size_t pkt_len){						
	pcap_inject(this->pcap_, pkt, pkt_len);			
}

void* SwPort::pcap_capture(void* arg){
	struct pcap_datapath *pcap = (struct pcap_datapath*)arg;
	pcap_loop(pcap->pcap_, -1, packet_handler, (u_char *) pcap); 		
	return arg;
			
}

void SwPort::packet_handler(u_char *user, const struct pcap_pkthdr* pkthdr,const u_char*
					packet){						
		struct pcap_datapath *pcap_dp = (struct pcap_datapath*) user;
		struct packet *pkt = new struct packet();
		pkt->in_port = pcap_dp->port_no;
		pkt->data = (uint8_t*) packet;
		pkt->len = pkthdr->caplen;		
		//parse the packet
		of10::Match pkt_match;
		pkt_match.in_port(pkt->in_port);
		Flow::extract_flow_fields(pkt_match, pkt->data, pkthdr->caplen);							
		//send it to flow table match
		bool match = false;		
		for(std::set<Flow>::iterator it = (*(pcap_dp->dp))->flow_table.begin();
			it != (*(pcap_dp->dp))->flow_table.end(); ++it){			
			match = Flow::pkt_match(*it, pkt_match);				
			if(match){
				//Apply the actions and leave
				ActionList l = it->actions;
			    std::list<Action*> acts = l.action_list();			    
				for(std::list<Action*>::iterator act_it = acts.begin();
 					act_it != acts.end(); ++act_it){					
					(*(pcap_dp->dp))->action_handler(*act_it, pkt);
 				}
 				return;
			}
		}	
		// // Save the packet into the buffer
		++Datapath::buffer_id;
		(*(pcap_dp->dp))->pkt_buffer.insert(std::pair<uint32_t,struct packet*>(Datapath::buffer_id,pkt));
		//No -> Send to controller
		if(*(*(pcap_dp->dp))->conn != NULL){					
			// /*TODO: generate xid function*/
			of10::PacketIn pi(21,  Datapath::buffer_id, pkthdr->len, pkt->in_port, of10::OFPR_NO_MATCH);
	 		pi.data(pkt->data, pkt->len);
	 		uint8_t* buffer  = pi.pack();
	 		(*(*(pcap_dp->dp))->conn)->send(buffer, pi.length());
	 		OFMsg::free_buffer(buffer);
	 	}
}
