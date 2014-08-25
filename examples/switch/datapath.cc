#include "datapath.hh"
#include "port.hh"

uint32_t Datapath::buffer_id = 0;

Datapath::~Datapath(){
	for(std::map<uint32_t, struct packet*>::iterator it = this->pkt_buffer.begin();
		  				it != this->pkt_buffer.end(); ++it){
		delete it->second;
	}
}

void Datapath::handle_barrier_request(uint8_t* data){	
	OFMsg msg(data);
	of10::BarrierReply br(msg.xid());
	uint8_t* buffer = br.pack();
	(*(this->conn))->send(buffer, br.length());
	OFMsg::free_buffer(buffer);	
}

void Datapath::action_handler(Action *act, struct packet *pkt){ 		
 		switch(act->type()){ 			
 			case of10::OFPAT_OUTPUT:{ 				
 				of10::OutputAction *oa = static_cast<of10::OutputAction*>(act);
 				//send the packet through the same port 				 				
 				if(oa->port() == of10::OFPP_IN_PORT){
 					SwPort *port = this->ports[pkt->in_port - 1];
 					port->port_output_packet(pkt->data, pkt->len);
 				}
 				/*Flood or All. As we are not dealing with STP the actions are the same*/
 				else if(oa->port() ==  of10::OFPP_FLOOD || oa->port() ==  of10::OFPP_ALL){ 					
 					for(std::vector<SwPort*>::iterator it = this->ports.begin();
		  				it != this->ports.end(); ++it){
 						if((*it)->port_no() == pkt->in_port)
 							continue;
 						(*it)->port_output_packet(pkt->data, pkt->len);
 					}
 				}
 				else if (oa->port() == of10::OFPP_CONTROLLER){
 					Datapath::buffer_id++;
 					this->pkt_buffer.insert(std::pair<uint32_t,struct packet*>(Datapath::buffer_id,pkt));
 					uint16_t max_len = pkt->len > oa->max_len()? oa->max_len():pkt->len;
 					of10::PacketIn pi(21,  -1, pkt->in_port, pkt->len, of10::OFPR_ACTION);
			 		pi.data(pkt->data, max_len);
			 		uint8_t* buffer  = pi.pack();
			 		(*(this->conn))->send(buffer, pi.length());
			 		OFMsg::free_buffer(buffer);
 				}
 				else {
 					SwPort *port = this->ports[oa->port()-1];
 					port->port_output_packet(pkt->data, pkt->len);
 				}
 				break;
 			}

 		}
 	}

void Datapath::handle_flow_mod(uint8_t *data){
	of10::FlowMod fm;
	fm.unpack((uint8_t*)data);
	switch(fm.command()){
		case of10::OFPFC_ADD:{
			Flow flow;
			flow.priority_ = fm.priority();
			flow.match = fm.match();
			flow.actions = fm.actions();					
			for(std::set<Flow>::iterator it = this->flow_table.begin();
				it != this->flow_table.end(); ++it){			
				if (Flow::strict_match(*it, flow)){
					this->flow_table.erase(it);
					break;							
				}							
			}
			this->flow_table.insert(flow);
			if(fm.buffer_id() != -1) {
				struct packet *pkt =  this->pkt_buffer[fm.buffer_id()];	
				of10::Match pkt_match;
				pkt_match.in_port(pkt->in_port);
				Flow::extract_flow_fields(pkt_match, pkt->data, pkt->len);
				if(Flow::pkt_match(flow, pkt_match)){
					//Apply the actions and leave
					ActionList l = flow.actions;
				    std::list<Action*> acts = l.action_list();			    
					for(std::list<Action*>::iterator act_it = acts.begin();
	 					act_it != acts.end(); ++act_it){					
						action_handler(*act_it, pkt);
	 				}
	 			delete pkt;	
				}								
			}
			break;					
		}
		//Delete all matching flows
		case of10::OFPFC_DELETE:{				
				Flow flow;
				flow.match = fm.match();
				std::set<Flow>::iterator it; 
				for (it = this->flow_table.begin(); it != 
					this->flow_table.end(); ) {
				    if (Flow::non_strict_match(flow, *it)) {
				        this->flow_table.erase(it++);
				    }
				    else {
				        ++it;
				    }
				}
			}			
			break;
		}
}

void Datapath::handle_packet_out(uint8_t* data){
	of10::PacketOut po;
	po.unpack((uint8_t*)data);
	ActionList l = po.actions();
	std::list<Action*> acts = l.action_list();
	struct packet *pkt;
	if(po.buffer_id() == -1){
		pkt = new struct packet();
		pkt->in_port = po.in_port();
		pkt->data = (uint8_t*) po.data();
		pkt->len = po.data_len();
	}
	else {
		pkt = this->pkt_buffer[po.buffer_id()];				
	}
	for(std::list<Action*>::const_iterator it = acts.begin();
			it != acts.end(); ++it){ 								
		action_handler(*it, pkt);
	}
	delete pkt;
}
