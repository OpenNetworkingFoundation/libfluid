#include <signal.h>

#include <android/log.h>
#include <jni.h>

#include "Controller.hh"
#include "msg/MsgApps.hh"

class AndroidController : public Controller {
public:
    AndroidController(const char* address, int port, int nthreads) : 
        Controller(address, port, nthreads) {}
    
    virtual void connection_callback(OFConnection* ofconn, OFConnection::Event type) {
        if (type == OFConnection::EVENT_STARTED)
            __android_log_print(ANDROID_LOG_DEBUG, "OFCONTROLLER", 
                "Connection id=%d started\n", ofconn->get_id());

        else if (type == OFConnection::EVENT_ESTABLISHED)
            __android_log_print(ANDROID_LOG_DEBUG, "OFCONTROLLER", 
                "Connection id=%d established\n", ofconn->get_id());

        else if (type == OFConnection::EVENT_FAILED_NEGOTIATION)
            __android_log_print(ANDROID_LOG_DEBUG, "OFCONTROLLER", 
                "Connection id=%d failed version negotiation\n", ofconn->get_id());

        else if (type == OFConnection::EVENT_CLOSED)
            __android_log_print(ANDROID_LOG_DEBUG, "OFCONTROLLER", 
                "Connection id=%d closed by the user\n", ofconn->get_id());

        else if (type == OFConnection::EVENT_DEAD)
            __android_log_print(ANDROID_LOG_DEBUG, "OFCONTROLLER", 
                "Connection id=%d closed due to inactivity\n", ofconn->get_id());
        
        Controller::connection_callback(ofconn, type);
    }
};

extern "C" void
Java_com_cpqd_ofcontroller_Controller_startController(JNIEnv*  env,
                                      jobject  self,
                                      jint port,
                                      jint nthreads) {
    AndroidController ctrl("0.0.0.0", port, nthreads);
    MultiLearningSwitch app;
    ctrl.register_for_event(&app, EVENT_PACKET_IN);
    ctrl.register_for_event(&app, EVENT_SWITCH_UP);
    ctrl.register_for_event(&app, EVENT_SWITCH_DOWN);
    __android_log_print(ANDROID_LOG_DEBUG, "OFCONTROLLER", "Server listening at port %d\n", port);
    ctrl.start(true);
    __android_log_print(ANDROID_LOG_DEBUG, "OFCONTROLLER", "Server stopped", port);
}
