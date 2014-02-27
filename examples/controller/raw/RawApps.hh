#ifndef __RAWAPPS_HH__
#define __RAWAPPS_HH__

#include <arpa/inet.h>

#include "of10.hh"
#include "Controller.hh"

class RawCBench : public Application {
    virtual void event_callback(ControllerEvent* ev) {
        if (ev->get_type() == EVENT_PACKET_IN) {
            struct ofp_flow_mod fm;
            memset((void*) &fm, 0, sizeof(struct ofp_flow_mod));
            fm.header.version = 0x01;
            fm.header.type = OFPT_FLOW_MOD;
            fm.header.length = htons(sizeof(struct ofp_flow_mod));
            fm.header.xid = 0;
            fm.command = OFPFC_ADD;
            ev->ofconn->send((uint8_t*) &fm, sizeof(struct ofp_flow_mod));
        }
    }
};

void print_bin(uint8_t* data, int len) {
    for (int i = 0; i < len; i++) {
        printf("%x ", data[i]);
    }
    printf("\n");
}

class RawLearningSwitch : public BaseLearningSwitch {
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

        uint16_t msg_len = 72 + 8;
        uint8_t data[msg_len];
        uint8_t* raw = (uint8_t*) &data;
        memset(raw, 0, msg_len);

        // Flow mod message
        struct ofp_flow_mod fm;
        fm.header.version = 0x01;
        fm.header.type = OFPT_FLOW_MOD;
        fm.header.length = htons(msg_len);
        fm.header.xid = xid;

        // Match
        memset(&fm.match, 0, sizeof(struct ofp_match));
        fm.match.wildcards = htonl(OFPFW_ALL & ~OFPFW_DL_SRC & ~OFPFW_DL_DST);
        memcpy((uint8_t*) &fm.match.dl_src, ((uint8_t*) &src) + 2, 6);
        memcpy((uint8_t*) &fm.match.dl_dst, ((uint8_t*) &dst) + 2, 6);

        // Flow mod body
        fm.cookie = 123;
        fm.command = OFPFC_ADD;
        fm.idle_timeout = htons(5);
        fm.hard_timeout = htons(10);
        fm.priority  = htons(100);
        fm.buffer_id = ofpi->buffer_id;
        fm.out_port = 0;
        fm.flags = 0;
        memcpy(raw, &fm, 72);

        // Output action
        struct ofp_action_output act;
        act.type = OFPAT_OUTPUT;
        act.len = htons(8);
        act.port = htons(out_port);
        act.max_len = 0;
        memcpy(raw + 72, &act, 8);

        ofconn->send(raw, msg_len);
    }

    void flood(PacketInEvent* pi) {
        struct ofp_packet_in* ofpi = (struct ofp_packet_in*) pi->data;

        uint16_t msg_len = 16 + 8 + ntohs(ofpi->total_len);
        uint8_t data[msg_len];
        uint8_t* raw = (uint8_t*) &data;
        memset(raw, 0, msg_len);

        // Packet out message
        struct ofp_packet_out po;
        po.header.version = 0x01;
        po.header.type = OFPT_PACKET_OUT;
        po.header.length = htons(msg_len);
        po.header.xid = ((uint32_t*) pi->data)[1];
        po.buffer_id = ofpi->buffer_id;
        po.in_port = ofpi->in_port;
        po.actions_len = htons(8);
        memcpy(raw, &po, 16);

        // Flood action
        struct ofp_action_output act;
        act.type = OFPAT_OUTPUT;
        act.len = htons(8);
        act.port = htons(OFPP_FLOOD);
        act.max_len = 0;
        memcpy(raw + 16, &act, 8);

        // Packet content
        memcpy(raw + 16 + 8, pi->data, ntohs(ofpi->total_len));

        pi->ofconn->send(raw, msg_len);
    }
};

#endif
