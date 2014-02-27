/** @file */
#ifndef __BASEOFCLIENT_HH__
#define __BASEOFCLIENT_HH__

#include <pthread.h>

#include <string>

#include <fluid/base/EventLoop.hh>
#include <fluid/base/BaseOFConnection.hh>

using namespace fluid_base;
/**
A BaseOFClient manages the very basic functions of an OpenFlow client. It
connects to a server and wait for messages and connection events. It is an
abstract class that should be overriden by another class to provide OpenFlow
features.
*/
class BaseOFClient : public BaseOFHandler {
public:
    /**
    Create a BaseOFClient.

    @param port listen port
    @param nevloops number of event loops to run. Connections will be
                    attributed to event loops running on threads on a
                    round-robin fashion. The first event loop will listen for
                    new connections.
    */
    BaseOFClient(int id, std::string address, int port);
    ~BaseOFClient();

    /**
    Start the client. It will connect at the address and port declared in the
    constructor and wait for events, optionally blocking the calling thread
    until BaseOFClient::stop is called.

    @param block block the calling thread while the client is running
    */
    virtual bool start(bool block = false);

    virtual void start_conn();

    static void* try_connect(void* arg);

    /**
    Stop the client. It will close the connection and signal the event loop to
    stop running.

    It will eventually unblock BaseOFClient::start if it is blocking.
    */
    virtual void stop();


    virtual void base_connection_callback(BaseOFConnection* conn,
                                          BaseOFConnection::Event event_type);
    virtual void base_message_callback(BaseOFConnection* conn,
                                       void* data,
                                       size_t len) = 0;
    virtual void free_data(void* data);

private:
    int id;
    bool blocking;
    EventLoop* evloop;
    pthread_t t;
    pthread_t conn_t;

    std::string address;
    int port;

};

#endif
