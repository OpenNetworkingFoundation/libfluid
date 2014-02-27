#ifndef PORT_H
#define PORT_H 1

#include <pcap.h>
#include <string>
#include "datapath.hh"

struct pcap_datapath{
	Datapath **dp;
	uint16_t port_no;
	pcap_t* pcap_;	
};

class SwPort{
	private:
		int port_no_;
		std::string name_;
		pcap_t* pcap_;		
	public:
			
		SwPort(int port_no, std::string name);						
		void* open();
		void close();		
		void port_output_packet(uint8_t *pkt, size_t pkt_len);		
		static void* pcap_capture(void* arg);					
		static void packet_handler(u_char *user, const struct pcap_pkthdr* pkthdr,
				const u_char* packet);

		int port_no(){return this->port_no_;}		
		std::string name(){return this->name_;}
		pcap_t* pcap(){return this->pcap_;}
		void port_no(int port_no){this->port_no_ = port_no;}
		void name(std::string name){this->name_ = name;}	 
		void pcap(pcap_t *pcap){this->pcap_ = pcap;}

};

#endif