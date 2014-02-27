#ifndef __LOCIAPPS_HH__
#define __LOCIAPPS_HH__

#include <signal.h>
#include <arpa/inet.h>
#include <arpa/inet.h>
extern "C" {
#include <loci/loci.h>
}

#include "Controller.hh"
#include "of10.hh"

/* To make this example work, just move the LOCI generated code to the same 
level of this file and run make loxi_controller in the controller example
directory. */

class LociLearningSwitch : public BaseLearningSwitch {
    virtual void event_callback(ControllerEvent* ev) {
        if (ev->get_type() == EVENT_PACKET_IN) {
            L2TABLE* l2table = 
                get_l2table(ev->ofconn);
            if (l2table == NULL) {
                return;
            }
            
            int conn_id = ev->ofconn->get_id();

            PacketInEvent* pi = static_cast<PacketInEvent*>(ev);
            struct ofp_packet_in* ofpi = (struct ofp_packet_in*) pi->data;

            uint64_t dst = 0, src = 0;
            
            memcpy( ((uint8_t*) &dst) + 2, ofpi->data, 6);
            memcpy( ((uint8_t*) &src) + 2, ofpi->data + 6, 6);
            
            uint16_t in_port = ntohs(ofpi->in_port);

            // Learn the source
            (*l2table)[src] = in_port;

            // Try to find the destination
            L2TABLE::iterator it = l2table->find(dst);
            if (it == l2table->end()) {
                flood(pi);
                return;
            }
            install_flow_mod(pi, ev->ofconn, ofpi->header.xid, src, dst, it->second);
        }
        else {
            BaseLearningSwitch::event_callback(ev);
        }
    }

    void install_flow_mod(PacketInEvent* pi, OFConnection* ofconn, uint32_t xid, uint64_t src, uint64_t dst, uint16_t out_port) {
        struct ofp_packet_in* ofpi = (struct ofp_packet_in*) pi->data;
        
        // Flow mod
        of_flow_add_t *flow_add = of_flow_add_new(OF_VERSION_1_0);
        of_flow_add_cookie_set(flow_add, 123);
        of_flow_add_idle_timeout_set(flow_add, 5);
        of_flow_add_hard_timeout_set(flow_add, 10);
        of_flow_add_priority_set(flow_add, 100);
        of_flow_add_buffer_id_set(flow_add, ntohl(ofpi->buffer_id));
        
        // Match
        of_match_t match;
        memset(&match, 0, sizeof(match));
        uint8_t* srcb = ((uint8_t*) &src) + 2;
        uint8_t* dstb = ((uint8_t*) &dst) + 2;
        of_mac_addr_t srct = {{srcb[0], srcb[1], srcb[2], 
                               srcb[3], srcb[4], srcb[5]}};
        of_mac_addr_t dstt = {{dstb[0], dstb[1], dstb[2], 
                               dstb[3], dstb[4], dstb[5]}};
        match.fields.eth_src = srct;
        match.fields.eth_dst = dstt;
        OF_MATCH_MASK_ETH_SRC_EXACT_SET(&match);
        OF_MATCH_MASK_ETH_DST_EXACT_SET(&match);
        if (of_flow_add_match_set(flow_add, &match)) {
            fprintf(stderr, "Failed to set the match field\n");
            abort();
        }

        // Actions
        of_list_action_t actions;
        of_flow_add_actions_bind(flow_add, &actions);
        
        of_action_output_t action;
        of_action_output_init(&action, flow_add->version, -1, 1);
        of_list_action_append_bind(&actions, (of_action_t *)&action);
        of_action_output_port_set(&action, out_port);
        
        // Send
        void *raw = WBUF_BUF(OF_OBJECT_TO_WBUF(flow_add));
        uint16_t msg_len = flow_add->length;
        ofconn->send(raw, msg_len);
        of_flow_add_delete(flow_add);
    }

    void flood(PacketInEvent* pi) {
        struct ofp_packet_in* ofpi = (struct ofp_packet_in*) pi->data;
        
        // Packet out
        of_packet_out_t *po = of_packet_out_new(OF_VERSION_1_0);
        of_packet_out_buffer_id_set(po, ntohl(ofpi->buffer_id));
        of_packet_out_in_port_set(po, ntohs(ofpi->in_port));
        
        // Actions
        of_list_action_t actions;
        of_packet_out_actions_bind(po, &actions);
        
        of_action_output_t action;
        of_action_output_init(&action, po->version, -1, 1);
        of_list_action_append_bind(&actions, (of_action_t *)&action);
        of_action_output_port_set(&action, OFPP_FLOOD);

        // Send
        void *raw = WBUF_BUF(OF_OBJECT_TO_WBUF(po));
        uint16_t msg_len = po->length;
        // Loxigen isn't setting the actions_len field for us
        ((uint16_t*) raw)[7] = htons(8);
        pi->ofconn->send(raw, msg_len);
        of_packet_out_delete(po);
    }
};

#endif
