#include "OFClient.hh"

int main() {
    OFClient client(0, "127.0.0.1", 6653);
    client.start(true);
}
