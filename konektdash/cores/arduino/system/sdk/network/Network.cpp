#include "Network.h"
#include <cstring>

void Network::init(NetworkEventHandler &handler) {
    eventHandler = &handler;
}

bool Network::write(int socket, const char* content) {
    return write(socket, (const uint8_t*)content, strlen(content));
}
