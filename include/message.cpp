#include "message.h"
void Message::SetAddress(SOCKADDR_IN address) {
        this->address = address;
        this->address_size = sizeof(address);
};

void Message::PrintAddress() {
    this->buffer[this->bytesReceived] = 0;
    printf( "%d.%d.%d.%d:%d", 
    this->address.sin_addr.S_un.S_un_b.s_b1, 
    this->address.sin_addr.S_un.S_un_b.s_b2, 
    this->address.sin_addr.S_un.S_un_b.s_b3, 
    this->address.sin_addr.S_un.S_un_b.s_b4, 
    this->address.sin_port);
};