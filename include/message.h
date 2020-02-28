#include "pre.h"

class Message{
public:
    int32_t SOCKADDR_IN_size;
    int32_t flags = 0;
    SOCKADDR_IN address;
    int address_size;
    int32_t bufferLength;
    int bytesReceived = SOCKET_ERROR;
    char buffer[];
    void SetAddress(SOCKADDR_IN);
    void PrintAddress();
};