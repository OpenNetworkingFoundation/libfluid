#include "OFClient.hh"
#include <fluid/of10msg.hh>

using namespace fluid_base;
using namespace fluid_msg;

void OFClient::base_message_callback(BaseOFConnection* c, void* data, size_t len) {
    uint8_t type = ((uint8_t*) data)[1];
    OFConnection* cc = (OFConnection*) c->get_manager();

    // We trust that the other end is using the negotiated protocol
    // version. Should we?

    if (ofsc.liveness_check() and type == of10::OFPT_ECHO_REQUEST) {
        uint8_t msg[8];
        memset((void*) msg, 0, 8);
        msg[0] = ((uint8_t*) data)[0];
        msg[1] = of10::OFPT_ECHO_REPLY;
        ((uint16_t*) msg)[1] = htons(8);
        ((uint32_t*) msg)[1] = ((uint32_t*) data)[1];
        // TODO: copy echo data
        c->send(msg, 8);

        if (ofsc.dispatch_all_messages()) goto dispatch; else goto done;
    }

    if (ofsc.handshake() and type == of10::OFPT_HELLO) {
        uint8_t version = ((uint8_t*) data)[0];
        if (not this->ofsc.supported_versions() & (1 << (version - 1))) {
            uint8_t msg[12];
            memset((void*) msg, 0, 8);
            msg[0] = version;
            msg[1] = of10::OFPT_ERROR;
            ((uint16_t*) msg)[1] = htons(12);
            ((uint32_t*) msg)[1] = ((uint32_t*) data)[1];
            ((uint16_t*) msg)[4] = htons(of10::OFPET_HELLO_FAILED);
            ((uint16_t*) msg)[5] = htons(of10::OFPHFC_INCOMPATIBLE);
            cc->send(msg, 12);
            cc->close();
            cc->set_state(OFConnection::STATE_FAILED);
            connection_callback(cc, OFConnection::EVENT_FAILED_NEGOTIATION);
        }

        if (ofsc.dispatch_all_messages()) goto dispatch; else goto done;
    }

    if (ofsc.liveness_check() and type == of10::OFPT_ECHO_REPLY) {
        if (ntohl(((uint32_t*) data)[1]) == ECHO_XID) {
            cc->set_alive(true);
        }

        if (ofsc.dispatch_all_messages()) goto dispatch; else goto done;
    }

    if (ofsc.handshake() and type == of10::OFPT_FEATURES_REQUEST) {
        cc->set_version(((uint8_t*) data)[0]);
        cc->set_state(OFConnection::STATE_RUNNING);        
        of10::FeaturesRequest freq;
        freq.unpack((uint8_t*) data);
        of10::FeaturesReply fr(freq.xid(), this->datapath_id, 1, 1, 0x0, 0x0);
        uint8_t *buffer =  fr.pack();
        c->send(buffer, fr.length());
        OFMsg::free_buffer(buffer);

        if (ofsc.liveness_check())
            c->add_timed_callback(send_echo, ofsc.echo_interval() * 1000, cc);
        connection_callback(cc, OFConnection::EVENT_ESTABLISHED);

        goto dispatch;
    }

    goto dispatch;

    // Dispatch a message and goto done
    dispatch:
        message_callback(cc, type, data, len);
        goto done;
    // Free the message and return
    done:
        c->free_data(data);
        return;
}

void OFClient::base_connection_callback(BaseOFConnection* c, BaseOFConnection::Event event_type) {
    /* If the connection was closed, destroy it.
    There's no need to notify the user, since a DOWN event already
    means a CLOSED event will happen and nothing should be expected from
    the connection. */
    if (event_type == BaseOFConnection::EVENT_CLOSED) {
        BaseOFClient::base_connection_callback(c, event_type);        
        // TODO: delete the OFConnection?
        return;
    }

    int conn_id = c->get_id();
    if (event_type == BaseOFConnection::EVENT_UP) {
        if (ofsc.handshake()) {
            struct of10::ofp_hello msg;
            msg.header.version = this->ofsc.max_supported_version();
            msg.header.type = of10::OFPT_HELLO;
            msg.header.length = htons(8);
            msg.header.xid = HELLO_XID;
            c->send(&msg, 8);
        }

		this->conn = new OFConnection(c, this);
        connection_callback(this->conn, OFConnection::EVENT_STARTED);
    }
    else if (event_type == BaseOFConnection::EVENT_DOWN) {
        connection_callback(this->conn, OFConnection::EVENT_CLOSED);
    }
}

void OFClient::free_data(void* data) {
    BaseOFClient::free_data(data);
}

void* OFClient::send_echo(void* arg) {
    OFConnection* cc = static_cast<OFConnection*>(arg);

    if (!cc->is_alive()) {
        cc->close();
        cc->get_ofhandler()->connection_callback(cc, OFConnection::EVENT_DEAD);
        return NULL;
    }

    uint8_t msg[8];
    memset((void*) msg, 0, 8);
    msg[0] = (uint8_t) cc->get_version();
    msg[1] = of10::OFPT_ECHO_REQUEST;
    ((uint16_t*) msg)[1] = htons(8);
    ((uint32_t*) msg)[1] = htonl(ECHO_XID);

    cc->set_alive(false);
    cc->send(msg, 8);

    return NULL;
}



