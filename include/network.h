#pragma once

#include "pre.h"
#include "def.h"
#include <stdio.h>
#include <stdlib.h>

#define DELIM "."

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

    int64 timestamp_received_ms;

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

void send_msg(SOCKET* sock, Message& s_Msg, uint32 buffer_length, SOCKADDR_IN& address) {
    if (sendto( *sock,
                (const char*) s_Msg.buffer,
                buffer_length,
                s_Msg.flags,
                (SOCKADDR*)&address,
                sizeof( address )) == SOCKET_ERROR) {
        printf( "sendto failed: %d", WSAGetLastError() );
    }
}

void send_msg(SOCKET* sock, Message& s_Msg) {
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
};


// Below code stolen from:
// https://www.includehelp.com/c-programs/check-string-is-valid-ipv4-address-or-not.aspx

/* return 1 if string contain only digits, else return 0 */
int valid_digit(char *ip_str)
{
    while (*ip_str) {
        if (*ip_str >= '0' && *ip_str <= '9')
            ++ip_str;
        else
            return 0;
    }
    return 1;
}
 
/* return 1 if IP string is valid, else return 0 */
int is_valid_ip(char *ip_str)
{
    int i, num, dots = 0;
    char *ptr;
 
    if (ip_str == NULL)
        return 0;
 
    ptr = strtok(ip_str, DELIM);
 
    if (ptr == NULL)
        return 0;
 
    while (ptr) {
 
        /* after parsing string, it must contain only digits */
        if (!valid_digit(ptr))
            return 0;
 
        num = atoi(ptr);
 
        /* check for valid IP */
        if (num >= 0 && num <= 255) {
            /* parse remaining string */
            ptr = strtok(NULL, DELIM);
            if (ptr != NULL)
                ++dots;
        } else
            return 0;
    }
 
    /* valid IP string must contain 3 dots */
    if (dots != 3)
        return 0;
    return 1;
}

}