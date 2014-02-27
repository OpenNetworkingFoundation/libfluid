#ifndef __CONTROLLER_HH__
#define __CONTROLLER_HH__

#include <stdio.h>
#include <string.h>

#include <unordered_map>
// #include <boost/unordered_map.hpp>
#include <list>

#include <fluid/OFServer.hh>

using namespace fluid_base;

#define EVENT_PACKET_IN 0
#define EVENT_SWITCH_DOWN 1
#define EVENT_SWITCH_UP 2

typedef std::unordered_map<uint64_t, uint16_t> L2TABLE;

#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

volatile sig_atomic_t quit = 0;

void sigint_handler(int s) {
    quit = 1;
}

void wait_for_sigint() {
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = sigint_handler;
    sigfillset(&sa.sa_mask);
    sigaction(SIGINT, &sa, NULL);

    while (1) {
        if (quit) break;
        sleep(1000);
    }
}

class ControllerEvent {
public:
    ControllerEvent(OFConnection* ofconn, int type) {
        this->ofconn = ofconn;
        this->type = type;
    }
    virtual ~ControllerEvent() {}

    virtual int get_type() {
        return this->type;
    }

    OFConnection* ofconn;

private:
    int type;
};

class PacketInEvent : public ControllerEvent {
public:
    PacketInEvent(OFConnection* ofconn, OFHandler* ofhandler, void* data, size_t len) :
    ControllerEvent(ofconn, EVENT_PACKET_IN) {
        this->data = (uint8_t*) data;
        this->len = len;
        this->ofhandler = ofhandler;
    }

    virtual ~PacketInEvent() {
        this->ofhandler->free_data(this->data);
    }

    OFHandler* ofhandler;
    uint8_t* data;
    size_t len;
};

class SwitchUpEvent : public ControllerEvent {
public:
    SwitchUpEvent(OFConnection* ofconn, OFHandler* ofhandler, void* data, size_t len) :
    ControllerEvent(ofconn, EVENT_SWITCH_UP) {
        this->data = (uint8_t*) data;
        this->len = len;
        this->ofhandler = ofhandler;
    }

    virtual ~SwitchUpEvent() {
        this->ofhandler->free_data(this->data);
    }
    
    OFHandler* ofhandler;
    uint8_t* data;
    size_t len;
};

class SwitchDownEvent : public ControllerEvent {
public:
    SwitchDownEvent(OFConnection* ofconn) :
    ControllerEvent(ofconn, EVENT_SWITCH_DOWN) {}
};


class Application {
public:
    virtual void event_callback(ControllerEvent* ev) {}
};

class Controller : public OFServer {
public:
    std::unordered_map<int, std::vector<Application*> > event_listeners;
    // We assume that after we start the controller no event listeners will be
    // added. This way, we don't need to lock this map.
    bool running;

    Controller(const char* address = "0.0.0.0",
               const int port = 6653,
               const int n_workers = 4,
               bool secure = false) : 
    OFServer(address, port, n_workers, secure, OFServerSettings().supported_version(1).
                        supported_version(4).keep_data_ownership(false)) {
        this->running = true;
    }

    void register_for_event(Application* app, int event_type) {
        event_listeners[event_type].push_back(app);
    }

    void stop() {
        this->running = false;
        OFServer::stop();
    }

    virtual void message_callback(OFConnection* ofconn, uint8_t type, void* data, size_t len) {
        if (type == 10) { // OFPT_PACKET_IN
            dispatch_event(new PacketInEvent(ofconn, this, data, len));
        }
        else if (type == 6) { // OFPT_FEATURES_REPLY
            dispatch_event(new SwitchUpEvent(ofconn, this, data, len));
        }
    }

    virtual void connection_callback(OFConnection* ofconn, OFConnection::Event type) {
        if (type == OFConnection::EVENT_STARTED)
            printf("Connection id=%d started\n", ofconn->get_id());

        else if (type == OFConnection::EVENT_ESTABLISHED) {
            printf("Connection id=%d established\n", ofconn->get_id());
        }

        else if (type == OFConnection::EVENT_FAILED_NEGOTIATION)
            printf("Connection id=%d failed version negotiation\n", ofconn->get_id());

        else if (type == OFConnection::EVENT_CLOSED) {
            printf("Connection id=%d closed by the user\n", ofconn->get_id());
            dispatch_event(new SwitchDownEvent(ofconn));
        }

        else if (type == OFConnection::EVENT_DEAD) {
            printf("Connection id=%d closed due to inactivity\n", ofconn->get_id());
            dispatch_event(new SwitchDownEvent(ofconn));
        }
    }

    inline void dispatch_event(ControllerEvent* ev) {
        if (not this->running) {
            delete ev;
            return;
        }
        for (std::vector<Application*>::iterator it =
                event_listeners[ev->get_type()].begin();
             it != event_listeners[ev->get_type()].end();
             it++) {
            ((Application*) (*it))->event_callback(ev);
        }
        delete ev;
    }
};


class BaseLearningSwitch : public Application {
    std::list<L2TABLE*> l2tables;
    pthread_mutex_t l2tables_lock;
    
public:
    BaseLearningSwitch() {
        pthread_mutex_init(&l2tables_lock, NULL);
    }

    ~BaseLearningSwitch() {
        pthread_mutex_lock(&l2tables_lock);
        while (!l2tables.empty()) {
            L2TABLE* l2table = *l2tables.begin();
            l2tables.erase(l2tables.begin());
            delete l2table;
        }
        pthread_mutex_unlock(&l2tables_lock);
    }

    L2TABLE* get_l2table(OFConnection* ofconn) {
            L2TABLE* l2table = 
            (L2TABLE*) 
            ofconn->get_application_data();
        
        if (l2table == NULL) {
            fprintf(stderr, "l2table for connection id=%d not initialized. "
                            "Make sure your application is listening to "
                            "EVENT_SWITCH_UP.\n", ofconn->get_id());
        }
        
        return l2table;
    }
            
    virtual void event_callback(ControllerEvent* ev) {
        if (ev->get_type() == EVENT_SWITCH_UP) {                
            L2TABLE* l2table = 
                new L2TABLE();
            
            ev->ofconn->set_application_data(l2table);
            
            pthread_mutex_lock(&l2tables_lock);
            l2tables.push_back(l2table);
            pthread_mutex_unlock(&l2tables_lock);
        }
        
        else if (ev->get_type() == EVENT_SWITCH_DOWN) {
            L2TABLE* l2table = 
                (L2TABLE*) 
                ev->ofconn->get_application_data();
            if (l2table != NULL) {
                l2table->clear();
                printf("Deleting L2 entries for connection id=%d\n", ev->ofconn->get_id());
            }
        }
    }

};

#endif
