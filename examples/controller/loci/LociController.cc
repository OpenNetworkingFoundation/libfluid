#include "LociApps.hh"

int main(int argc, char **argv) {
    Controller ctrl("0.0.0.0", 6653, 4);
    
    if (argc > 2) {
        printf("Only \"l2\" is available for loci_controller.\n");
    }
    
    LociLearningSwitch app;
    ctrl.register_for_event(&app, EVENT_PACKET_IN);
    ctrl.register_for_event(&app, EVENT_SWITCH_UP);
    ctrl.register_for_event(&app, EVENT_SWITCH_DOWN);
    printf("l2 application (LociLearningSwitch) started\n");

    ctrl.start();
    wait_for_sigint();
    ctrl.stop();
    
    return 0;
}
