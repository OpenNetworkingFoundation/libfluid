#ifndef __MSGAPPS_HH__
#define __MSGAPPS_HH__

#include "Controller.hh"
#include <fluid/of10msg.hh>
#include <fluid/of13msg.hh>

using namespace fluid_msg;

class CBench: public Application {
    virtual void event_callback(ControllerEvent* ev) {
        if (ev->get_type() == EVENT_PACKET_IN) {
            of10::FlowMod fm;
            fm.command(of10::OFPFC_ADD);
            uint8_t* buffer;
            buffer = fm.pack();
            ev->ofconn->send(buffer, fm.length());
            OFMsg::free_buffer(buffer);
        }
    }
};

class MultiLearningSwitch: public BaseLearningSwitch {
public:
    virtual void event_callback(ControllerEvent* ev) {
        uint8_t ofversion = ev->ofconn->get_version();

        if (ev->get_type() == EVENT_PACKET_IN) {
            L2TABLE* l2table = get_l2table(ev->ofconn);
            if (l2table == NULL) {
                return;
            }
            uint64_t dst = 0, src = 0;
            PacketInEvent* pi = static_cast<PacketInEvent*>(ev);

            void* ofpip;
            uint8_t* data;
            uint16_t in_port;
            if (ofversion == of10::OFP_VERSION) {
                of10::PacketIn *ofpi = new of10::PacketIn();
                ofpip = ofpi;
                ofpi->unpack(pi->data);
                data = (uint8_t*) ofpi->data();
                memcpy(((uint8_t*) &dst) + 2, (uint8_t*) ofpi->data(), 6);
                memcpy(((uint8_t*) &src) + 2, (uint8_t*) ofpi->data() + 6, 6);
                in_port = ofpi->in_port();
            }
            else if (ofversion == of13::OFP_VERSION) {
                of13::PacketIn *ofpi = new of13::PacketIn();
                ;
                ofpip = ofpi;
                ofpi->unpack(pi->data);
                data = (uint8_t*) ofpi->data();
                memcpy(((uint8_t*) &dst) + 2, (uint8_t*) ofpi->data(), 6);
                memcpy(((uint8_t*) &src) + 2, (uint8_t*) ofpi->data() + 6, 6);
                if (ofpi->match().in_port() == NULL) {
                    return;
                }
                in_port = ofpi->match().in_port()->value();
            }

            // Learn the source
            (*l2table)[src] = in_port;

            // Try to find the destination
            L2TABLE::iterator it = l2table->find(dst);
            if (it == l2table->end()) {
                if (ofversion == of10::OFP_VERSION) {
                    flood10(*((of10::PacketIn*) ofpip), ev->ofconn);
                    delete (of10::PacketIn*) ofpip;
                }
                else if (ofversion == of13::OFP_VERSION) {
                    flood13(*((of13::PacketIn*) ofpip), ev->ofconn, in_port);
                    delete (of13::PacketIn*) ofpip;
                }
                return;
            }

            if (ofversion == of10::OFP_VERSION) {
                install_flow_mod10(*((of10::PacketIn*) ofpip), ev->ofconn, src,
                    dst, it->second);
                delete (of10::PacketIn*) ofpip;
            }
            else if (ofversion == of13::OFP_VERSION) {
                install_flow_mod13(*((of13::PacketIn*) ofpip), ev->ofconn, src,
                    dst, it->second);
                delete (of13::PacketIn*) ofpip;
            }
        }
        else if (ev->get_type() == EVENT_SWITCH_UP) {
            BaseLearningSwitch::event_callback(ev);
            if (ofversion == of13::OFP_VERSION) {
                install_default_flow13(ev->ofconn);
            }
        }

        else {
            BaseLearningSwitch::event_callback(ev);
        }
    }

    void install_flow_mod10(of10::PacketIn &pi, OFConnection* ofconn,
        uint64_t src, uint64_t dst, uint16_t out_port) {
        // Flow mod message
        uint8_t* buffer;
        /* Messages constructors allow to add all 
         values in a row. The fields order follows
         the specification */
        of10::FlowMod fm(pi.xid(),  //xid 
            123, // cookie
            of10::OFPFC_ADD, // command
            5, // idle timeout
            10, // hard timeout
            100, // priority
            pi.buffer_id(), //buffer id
            0, // outport
            0); // flags
        of10::Match m;
        m.dl_src(((uint8_t*) &src) + 2);
        m.dl_dst(((uint8_t*) &dst) + 2);
        fm.match(m);
        of10::OutputAction act(out_port, 1024);
        fm.add_action(act);
        buffer = fm.pack();
        ofconn->send(buffer, fm.length());
        OFMsg::free_buffer(buffer);
    }

    void flood10(of10::PacketIn &pi, OFConnection* ofconn) {
        uint8_t* buf;
        of10::PacketOut po(pi.xid(), pi.buffer_id(), pi.in_port());
        /*Add Packet in data if the packet was not buffered*/
        if (pi.buffer_id() == -1) {
            po.data(pi.data(), pi.data_len());
        }
        of10::OutputAction act(of10::OFPP_FLOOD, 1024);
        po.add_action(act);
        buf = po.pack();
        ofconn->send(buf, po.length());
        OFMsg::free_buffer(buf);
    }

    void install_default_flow13(OFConnection* ofconn) {
        uint8_t* buffer;
        of13::FlowMod fm(42, 0, 0xffffffffffffffff, 0, of13::OFPFC_ADD, 0, 0, 0,
            0xffffffff, 0, 0, 0);
        of13::OutputAction *act = new of13::OutputAction(of13::OFPP_CONTROLLER,
            of13::OFPCML_NO_BUFFER);
        of13::ApplyActions *inst = new of13::ApplyActions();
        inst->add_action(act);
        fm.add_instruction(inst);
        buffer = fm.pack();
        ofconn->send(buffer, fm.length());
        OFMsg::free_buffer(buffer);
    }

    void install_flow_mod13(of13::PacketIn &pi, OFConnection* ofconn,
        uint64_t src, uint64_t dst, uint32_t out_port) {
        // Flow mod message
        uint8_t* buffer;
        /*You can also set the message field using
         class methods which have the same names from
         the field present on the specification*/
        of13::FlowMod fm;
        fm.xid(pi.xid());
        fm.cookie(123);
        fm.cookie_mask(0xffffffffffffffff);
        fm.table_id(0);
        fm.command(of13::OFPFC_ADD);
        fm.idle_timeout(5);
        fm.hard_timeout(10);
        fm.priority(100);
        fm.buffer_id(pi.buffer_id());
        fm.out_port(0);
        fm.out_group(0);
        fm.flags(0);
        of13::EthSrc fsrc(((uint8_t*) &src) + 2);
        of13::EthDst fdst(((uint8_t*) &dst) + 2);
        fm.add_oxm_field(fsrc);
        fm.add_oxm_field(fdst);
        of13::OutputAction act(out_port, 1024);
        of13::ApplyActions inst;
        inst.add_action(act);
        fm.add_instruction(inst);
        buffer = fm.pack();
        ofconn->send(buffer, fm.length());
        OFMsg::free_buffer(buffer);
        of13::Match m;
        of13::MultipartRequestFlow rf(2, 0x0, 0, of13::OFPP_ANY, of13::OFPG_ANY,
            0x0, 0x0, m);
        buffer = rf.pack();
        ofconn->send(buffer, rf.length());
        OFMsg::free_buffer(buffer);
    }

    void flood13(of13::PacketIn &pi, OFConnection* ofconn, uint32_t in_port) {
        uint8_t* buf;
        of13::PacketOut po(pi.xid(), pi.buffer_id(), in_port);
        /*Add Packet in data if the packet was not buffered*/
        if (pi.buffer_id() == -1) {
            po.data(pi.data(), pi.data_len());
        }
        of13::OutputAction act(of13::OFPP_FLOOD, 1024); // = new of13::OutputAction();
        po.add_action(act);
        buf = po.pack();
        ofconn->send(buf, po.length());
        OFMsg::free_buffer(buf);
    }
};

#endif
