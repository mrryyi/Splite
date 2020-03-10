#pragma once

#include "pre.h"

namespace Network
{

class Message {
public:
    uint8 buffer[SOCKET_BUFFER_SIZE];
    int32 SOCKADDR_IN_size;
    int32 flags = 0;
    SOCKADDR_IN address;
    int address_size;
    int32 bufferLength;
    int bytesReceived = SOCKET_ERROR;

    void SetAddress(SOCKADDR_IN address) {
        this->address = address;
        this->address_size = sizeof(address);
    };

    void PrintAddress() {
        this->buffer[this->bytesReceived] = 0;
        printf( "%d.%d.%d.%d:%d", 
        this->address.sin_addr.S_un.S_un_b.s_b1, 
        this->address.sin_addr.S_un.S_un_b.s_b2, 
        this->address.sin_addr.S_un.S_un_b.s_b3, 
        this->address.sin_addr.S_un.S_un_b.s_b4, 
        this->address.sin_port);
    };
};

void send(SOCKET* sock, Message& s_Msg){
    uint8 message_type;
    memcpy( &message_type, &s_Msg.buffer[0], sizeof( message_type ) );
    
    if (sendto( *sock,
                (const char*) s_Msg.buffer,
                s_Msg.bufferLength,
                s_Msg.flags,
                (SOCKADDR*)&s_Msg.address,
                s_Msg.address_size) == SOCKET_ERROR) {
        printf( "sendto failed: %d", WSAGetLastError() );
    }
}

static bool8 set_sock_opt(SOCKET sock, int opt, int val)
{
	int len = sizeof(int);
	if (setsockopt(sock, SOL_SOCKET, opt, (char*)&val, len) == SOCKET_ERROR)
	{
		return false;
	}

	int actual;
	if (getsockopt(sock, SOL_SOCKET, opt, (char*)&actual, &len) == SOCKET_ERROR)
	{
		return false;
	}

	return val == actual;
}

bool8 make_socket(SOCKET* out_socket)
{
	int address_family = AF_INET;
	int type = SOCK_DGRAM;
	int protocol = IPPROTO_UDP;
    SOCKET sock;
	sock = socket(address_family, type, protocol);

	if (!set_sock_opt(sock, SO_RCVBUF, SOCKET_BUFFER_SIZE))
	{
		printf("failed to set rcvbuf size");
	}
	if (!set_sock_opt(sock, SO_SNDBUF, SOCKET_BUFFER_SIZE))
	{
		printf("failed to set sndbuf size");
	}

	if (sock == INVALID_SOCKET)
	{
		printf("socket() failed: %d\n", WSAGetLastError());
		return false;
	}

	// put socket in non-blocking mode
	ULONG enabled = 1;

    // You may be wondering why we pass 0x8004667E as the second argument.
    // We actually want to pass FIONBIO, but it doesn't work, so we're using the value FIONBIO
    // would have if it worked. This results in the correct behaviour.
    // This issue took a million years to solve, including giving up on non-blocking sockets,
    // trying to settle for a billion threads to solve concurrency requirements,
    // encountering a quadrillion segfaults and WSA errors, refactoring here and there to accomodate
    // threading, then giving up to find this:
    //
    // https://stackoverflow.com/a/16185001
    //
    // :)

	int result = ioctlsocket(sock, 0x8004667E, &enabled);
	if (result == SOCKET_ERROR)
	{
		printf("ioctlsocket() failed: %d\n", WSAGetLastError());
		return false;
	}

    *out_socket = sock;

	return true;
}

}