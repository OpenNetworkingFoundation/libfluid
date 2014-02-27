#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <pthread.h>
#include <arpa/inet.h>

#include <event2/thread.h>

#include "BaseOFClient.hh"

using namespace fluid_base;

BaseOFClient::BaseOFClient(int id, std::string address, int port) {
    // Prepare libevent for threads
    // This will leave a small, insignificant leak for us.
    // See: http://archives.seul.org/libevent/users/Jul-2011/msg00028.html
    evthread_use_pthreads();
    // Ignore SIGPIPE so it becomes an EPIPE
    signal(SIGPIPE, SIG_IGN);

    this->id = id;
    this->address = address;
    this->port = port;
    this->evloop = new EventLoop(0);
}

BaseOFClient::~BaseOFClient() {    
    delete this->evloop;
}

void* BaseOFClient::try_connect(void *arg){
    int sock;
    struct sockaddr_in echoserver;
    int received = 0;

    BaseOFClient *boc = (BaseOFClient*) arg;

    /* Create the TCP socket */
    if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
        fprintf(stderr, "Error creating socket");
        return NULL;
    }
    memset(&echoserver, 0, sizeof(echoserver));
    echoserver.sin_family = AF_INET;
    echoserver.sin_addr.s_addr = inet_addr(boc->address.c_str());
    echoserver.sin_port = htons(boc->port);
    while (connect(sock, (struct sockaddr *) &echoserver, sizeof(echoserver)) < 0) {
        fprintf(stderr, "Retrying in 5 seconds...\n");
        sleep(5);
    }
    BaseOFConnection* c = new BaseOFConnection(0,
                                               boc,
                                               boc->evloop,
                                               sock,
                                               false);
    return NULL;                                                    
}

void BaseOFClient::start_conn(){
    pthread_create(&conn_t, NULL,
                      try_connect,
                       this);    
}

bool BaseOFClient::start(bool block) {    
    this->blocking = block;    
    start_conn();
    if (not this->blocking) {        
        pthread_create(&t,
                       NULL,
                       EventLoop::thread_adapter,
                       evloop);
    }
    else {        
        evloop->run();
    }
    return true;
}

void BaseOFClient::free_data(void* data) {
    BaseOFConnection::free_data(data);
}

void BaseOFClient::stop() {
    pthread_cancel(this->conn_t);
    evloop->stop();    
    if (not this->blocking) {
        pthread_join(t, NULL);
    }    
}

void BaseOFClient::base_connection_callback(BaseOFConnection* conn, BaseOFConnection::Event event_type) {
    if (event_type == BaseOFConnection::EVENT_CLOSED) {
        delete conn;
    }
}

