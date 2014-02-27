#include <stdlib.h>  
#include "flow.hh"


bool Flow::pkt_match(const Flow& flow, const of10::Match& pkt_match){
	return match_std_pkt(flow.match, pkt_match, 
					flow.nw_src_mask, flow.nw_dst_mask);
}

bool Flow::strict_match(const Flow& flow_a, const Flow& flow_b){
    return (flow_a.priority_ == flow_b.priority_) && match_std_strict(flow_a.match, flow_b.match, 
			flow_a.nw_src_mask, flow_b.nw_src_mask, flow_a.nw_dst_mask, 
			flow_b.nw_dst_mask);
}

bool Flow::non_strict_match(const Flow& flow_a, const Flow& flow_b){
	return match_std_nonstrict(flow_a.match, flow_b.match, 
			flow_a.nw_src_mask, flow_b.nw_src_mask, flow_a.nw_dst_mask, 
			flow_b.nw_dst_mask);
}

void Flow::extract_flow_fields(of10::Match &match, uint8_t *pkt, size_t len){
	size_t offset = 0;
    uint8_t next_proto = 0;


    struct protocols_std *proto = (struct protocols_std *) malloc(sizeof(struct protocols_std));

    /* Ethernet */
    if (len < offset + sizeof(struct eth_header)) {
        free(proto);
        return;
    }

    proto->eth = (struct eth_header *)(pkt + offset);
    offset += sizeof(struct eth_header);

    if (ntoh16(proto->eth->eth_type) >= ETH_TYPE_II_START) {
            /* Ethernet II */
		match.dl_src(proto->eth->eth_src);
		match.dl_dst(proto->eth->eth_dst);
		match.dl_type(ntoh16(proto->eth->eth_type));
    } else {
        /* Ethernet 802.3 */
        struct llc_header *llc;

        if (len < offset + sizeof(struct llc_header)) {
                free(proto);
                return;
            }

            llc = (struct llc_header *)(pkt + offset);
            offset += sizeof(struct llc_header);

            if (!(llc->llc_dsap == LLC_DSAP_SNAP &&
                  llc->llc_ssap == LLC_SSAP_SNAP &&
                  llc->llc_cntl == LLC_CNTL_SNAP)) {
                free(proto);
                return;
            }

            if (len < offset + sizeof(struct snap_header)) {
                free(proto);
                return;
            }

            proto->eth_snap = (struct snap_header *)(pkt + offset);
            offset += sizeof(struct snap_header);

            if (memcmp(proto->eth_snap->snap_org, SNAP_ORG_ETHERNET,
                                            sizeof(SNAP_ORG_ETHERNET)) != 0) {
                free(proto);
                return;
            }
            match.dl_src(proto->eth->eth_src);
			match.dl_src(proto->eth->eth_dst);
			match.dl_type(ntoh16(proto->eth->eth_type));
    }
    /* VLAN */
    if (ntoh16(proto->eth->eth_type) == ETH_TYPE_VLAN) {

        uint16_t vlan_id;
        uint8_t vlan_pcp;
        if (len < offset + sizeof(struct vlan_header)) {
            free(proto);
            return;
        }
        proto->vlan = (struct vlan_header *)(pkt + offset);
        proto->vlan_last = proto->vlan;
        offset += sizeof(struct vlan_header);
        vlan_id  = (ntoh16(proto->vlan->vlan_tci) &
                                        VLAN_VID_MASK) >> VLAN_VID_SHIFT;
        vlan_pcp = (ntoh16(proto->vlan->vlan_tci) &
                                        VLAN_PCP_MASK) >> VLAN_PCP_SHIFT;
        match.dl_vlan(vlan_id);
		match.dl_vlan_pcp(vlan_pcp);
		match.dl_type( ntoh16(proto->vlan->vlan_next_type));
    }
    else {
         //Vlan None
         match.dl_vlan(0xffff);  
    }

    /* skip through rest of VLAN tags */
    while (ntoh16(proto->eth->eth_type) == ETH_TYPE_VLAN) {

        if (len < offset + sizeof(struct vlan_header)) {
            free(proto);
            return;
        }
        proto->vlan_last = (struct vlan_header *)(pkt + offset);
        offset += sizeof(struct vlan_header);
        match.dl_type(ntoh16(proto->vlan->vlan_next_type));
    }
    /*ARP */
    if (ntoh16(proto->eth->eth_type) == ETH_TYPE_ARP) {
        if (len < offset + sizeof(struct arp_eth_header)) {
            free(proto);
            return;
        }
        proto->arp = (struct arp_eth_header *)(pkt + offset);
        offset += sizeof(struct arp_eth_header);
        match.nw_src(proto->arp->ar_spa);
        match.nw_dst(proto->arp->ar_tpa);
        match.nw_proto((uint8_t)ntoh16(proto->arp->ar_op));
        free(proto);
        return;
    }

     /* Network Layer */
    if (ntoh16(proto->eth->eth_type) == ETH_TYPE_IP) {
        if (len < offset + sizeof(struct ip_header)) {
            free(proto);
            return;
         }

        proto->ipv4 = (struct ip_header *)(pkt + offset);
        offset += sizeof(struct ip_header);
        match.nw_src(proto->ipv4->ip_src);
        match.nw_dst(proto->ipv4->ip_dst);
        match.nw_proto(proto->ipv4->ip_proto);
        match.nw_tos(proto->ipv4->ip_tos >> 2);
       
        if (IP_IS_FRAGMENT(proto->ipv4->ip_frag_off)) {
            /* No further processing for fragmented IPv4 */
            free(proto);
            return;
        }
        next_proto = proto->ipv4->ip_proto;
    }
    
    /*ICMP*/
    if (next_proto== IP_TYPE_ICMP) {
        if (len < offset + sizeof(struct icmp_header)) {
            free(proto);
            return;
        }
        proto->icmp = (struct icmp_header *)(pkt + offset);
        offset += sizeof(struct icmp_header);
        match.tp_src(proto->icmp->icmp_type);
        match.tp_dst(proto->icmp->icmp_code);
        return;
        free(proto);
    }

    /* Transport */
    if (next_proto== IP_TYPE_TCP) {
        if (len < offset + sizeof(struct tcp_header)) {
            free(proto);
            return;
        }
        proto->tcp = (struct tcp_header *)(pkt + offset);
        offset += sizeof(struct tcp_header);
        match.tp_src(ntoh16(proto->tcp->tcp_src));
        match.tp_dst(ntoh16(proto->tcp->tcp_dst));
        free(proto);
        return;
    }
    else if (next_proto == IP_TYPE_UDP) {
        if (len < offset + sizeof(struct udp_header)) {
            free(proto);
            return;
        }
        proto->udp = (struct udp_header *)(pkt + offset);
        offset += sizeof(struct udp_header);

        match.tp_src(ntoh16(proto->udp->udp_src));
        match.tp_dst(ntoh16(proto->udp->udp_dst));
        free(proto);
        return;

    }
    free(proto);
}     




        