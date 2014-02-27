#include <signal.h>

#include "fluid/TLS.hh"
#include "Controller.hh"
#include "RawApps.hh"



// See: http://git.openvswitch.org/cgi-bin/gitweb.cgi?p=openvswitch;a=blob_plain;f=INSTALL.SSL;hb=HEAD
#define CERT "/home/allanv/Desktop/certs/ctl-cert.pem"
#define PKEY "/home/allanv/Desktop/certs/ctl-privkey.pem"
#define SCRT "/usr/share/openvswitch/pki/switchca/cacert.pem"

int main(int argc, char **argv) {
    if (argc > 2) {
        printf("Only \"l2\" is available for secure_controller.\n");
    }
    
    Controller* normal = NULL;
    Controller* safe = NULL;
    
    libfluid_tls_init(CERT, PKEY, SCRT);
                      
    // Listen for insecure connections on port 6654
    normal = new Controller("127.0.0.1", 6654, 2);
    RawLearningSwitch app;
    normal->register_for_event(&app, EVENT_PACKET_IN);
    normal->register_for_event(&app, EVENT_SWITCH_UP);    
    normal->register_for_event(&app, EVENT_SWITCH_DOWN);
    printf("l2 application (RawLearningSwitch) started for normal controller\n");
    normal->start();

    // Listen for secure connections on port 6653
    safe = new Controller("127.0.0.1", 6653, 1, true);
    RawLearningSwitch app2;
    safe->register_for_event(&app2, EVENT_PACKET_IN);
    safe->register_for_event(&app2, EVENT_SWITCH_UP);        
    safe->register_for_event(&app2, EVENT_SWITCH_DOWN);
    printf("l2 application (RawLearningSwitch) started for safe controller\n");
    safe->start();

    wait_for_sigint();
    normal->stop();
    safe->stop();
    libfluid_tls_clear();

    delete normal;
    delete safe;

    return 0;
}
