#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <stdio.h>

#pragma comment(lib, "Ws2_32.lib")

#define SOCKET_BUFFER_SIZE 1024



int main() {
    
    // We create a WSADATA object called wsaData.
    WSADATA wsaData;

    // We call WSAStartup and return its value as an integer and check for errors.
    int iResult;
    // WSAStartup definition:
    // WSAStartup(
    // _In_ WORD wVersionRequested,
    // _Out_ LPWSADATA lpWSAData
    // );
    // MAKEWORD(2, 2) requests version 2.2 ow winsock.
    // WSAstartup makes a request for version 2.2 of Winsock on the system,
    // and sets the passed version as the highest version of Windows Sockets support
    // that the caller can use.
    iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed: %d\n", iResult);
        return 1;
    }

    int address_family = AF_INET;
    int type = SOCK_DGRAM;
    int protocol = IPPROTO_UDP;

    SOCKET sock = socket( address_family, type, protocol );

    if( sock == INVALID_SOCKET )
    {
        printf( "socket failed: %d", WSAGetLastError() );
        return 1;
    }

    SOCKADDR_IN local_address;
    local_address.sin_family = AF_INET;
    local_address.sin_port = htons( 1234 );
    local_address.sin_addr.s_addr = INADDR_ANY;
    if( bind( sock, (SOCKADDR*)&local_address, sizeof( local_address ) ) == SOCKET_ERROR )
    {
        printf( "bind failed: %d", WSAGetLastError() );
        return 1;
    }

    char buffer[SOCKET_BUFFER_SIZE];
    int flags = 0;
    SOCKADDR_IN from;
    int from_size = sizeof( from );
    int bytes_received = recvfrom( sock, buffer, SOCKET_BUFFER_SIZE, flags, (SOCKADDR*)&from, &from_size );

    printf("Waiting for receive...\n");
    if( bytes_received == SOCKET_ERROR )
    {
        printf( "recvfrom returned SOCKET_ERROR, WSAGetLastError() %d", WSAGetLastError() );
    }
    else
    {
        buffer[bytes_received] = 0;
        printf( "%d.%d.%d.%d:%d - %s", 
        from.sin_addr.S_un.S_un_b.s_b1, 
        from.sin_addr.S_un.S_un_b.s_b2, 
        from.sin_addr.S_un.S_un_b.s_b3, 
        from.sin_addr.S_un.S_un_b.s_b4, 
        from.sin_port, 
        buffer );
    }

    /*
    SOCKADDR_IN server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons( PORT );
    server_address.sin_addr.S_un.S_addr = inet_addr( "127.0.0.1" );

    char message[SOCKET_BUFFER_SIZE];
    gets_s( message, SOCKET_BUFFER_SIZE );

    int flags = 0;
    if( sendto( sock, message, strlen( message ), flags, (SOCKADDR*)&server_address, sizeof( server_address ) ) == SOCKET_ERROR )
    {
        printf( "sendto failed: %d", WSAGetLastError() );
        return;
    }
    */
    return 0;
}