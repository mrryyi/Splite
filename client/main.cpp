#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <stdio.h>
#include <ncurses.h> // getch

#pragma comment(lib, "Ws2_32.lib")

#define SOCKET_BUFFER_SIZE 1024
#define PORT_HERE 1500
#define PORT_SERVER 1234

typedef struct ThreadData {
    SOCKET* sock;
    int32_t is_running = 1;
} ;

DWORD WINAPI receiveThread( _Inout_ LPVOID lpParam) {
    
    ThreadData& threadData = *((ThreadData*)lpParam);

    char recvbuffer[SOCKET_BUFFER_SIZE];
    int flags = 0;
    SOCKADDR_IN from;
    int from_size = sizeof( from );
    int bytes_received = 0;

    int32_t player_x = 0;
    int32_t player_y = 0;

    int32_t is_running = 1;
    while(is_running) {
        
        int bytes_received = recvfrom( *threadData.sock, recvbuffer, SOCKET_BUFFER_SIZE, flags, (SOCKADDR*)&from, &from_size );
        
        if( bytes_received == SOCKET_ERROR )
        {
            printf( "recvfrom returned SOCKET_ERROR, WSAGetLastError() %d", WSAGetLastError() );
            break;
        }
        else
        {
            recvbuffer[bytes_received] = 0;
            // grab data from packet
            int32_t read_index = 0;

            memcpy( &player_x, &recvbuffer[read_index], sizeof( player_x ) );
            read_index += sizeof( player_x );

            memcpy( &player_y, &recvbuffer[read_index], sizeof( player_y ) );
            read_index += sizeof( player_y );

            memcpy( &is_running, &recvbuffer[read_index], sizeof( is_running ) );

            threadData.is_running = is_running;

            printf( "x:%d, y:%d, is_running:%d ", player_x, player_y, is_running );
        }
    }

    return 0;
}

int main() {

    // Initialize ncurses in order to make getch() into a blocking function.
    WINDOW *w;
    w = initscr();
    
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
    local_address.sin_port = htons( PORT_HERE );
    local_address.sin_addr.s_addr = INADDR_ANY;
    if( bind( sock, (SOCKADDR*)&local_address, sizeof( local_address ) ) == SOCKET_ERROR )
    {
        printf( "bind failed: %d", WSAGetLastError() );
        return 1;
    }

    SOCKADDR_IN server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons( PORT_SERVER );
    server_address.sin_addr.S_un.S_addr = inet_addr( "127.0.0.1" );

    char message[SOCKET_BUFFER_SIZE];

    ThreadData threadData { &sock };
    DWORD receiveThreadID;
    HANDLE receiveThreadHandle = CreateThread(0, 0, receiveThread, &threadData, 0, &receiveThreadID);

    char buffer[SOCKET_BUFFER_SIZE];

    while (threadData.is_running) {

        // get input
        buffer[0] = getchar();

        // send to server
        int buffer_length = 1;
        int flags = 0;
        SOCKADDR* to = (SOCKADDR*)&server_address;
        int to_length = sizeof( server_address );
        if( sendto( sock, buffer, buffer_length, flags, to, to_length ) == SOCKET_ERROR )
        {
            printf( "sendto failed: %d", WSAGetLastError() );
            return 0;
        }
        
    }

    CloseHandle(receiveThreadHandle);

    return 1;
}