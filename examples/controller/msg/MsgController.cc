#include <signal.h>

#include "MsgApps.hh"

Controller ctrl("0.0.0.0", 6653, 2);

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("Choose an application to run (\"l2\" or \"cbench\"):\n");
        printf("  ./msg_controller l2|cbench\n");
        return 0;
    }

    MultiLearningSwitch l2;
    CBench cbench;
    if (!strcmp(argv[1], "l2")) {
        ctrl.register_for_event(&l2, EVENT_PACKET_IN);
        ctrl.register_for_event(&l2, EVENT_SWITCH_UP);
        ctrl.register_for_event(&l2, EVENT_SWITCH_DOWN);
        printf("l2 application (MultiLearningSwitch) started\n");
    }
    else if (!strcmp(argv[1], "cbench")) {
        ctrl.register_for_event(&cbench, EVENT_PACKET_IN);
        printf("cbench (CBench) application started\n");
    }
    else {
        printf("Invalid application. Must be either \"l2\" or \"cbench\".\n");
        return 0;
    }

    ctrl.start();
    wait_for_sigint();
    ctrl.stop();

    return 0;
}
