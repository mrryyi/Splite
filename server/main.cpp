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
#define ever ;;

typedef struct ThreadData {
    SOCKET* sock;
} ;

DWORD WINAPI receiveThread( _Inout_ LPVOID lpParam) {
    

    ThreadData& threadData = *((ThreadData*)lpParam);

    char recvBuffer[SOCKET_BUFFER_SIZE];
    char sendBuffer[SOCKET_BUFFER_SIZE];
    int flags = 0;
    SOCKADDR_IN from;
    int from_size = sizeof( from );
    int bytes_received = 0;

    int32_t player_x = 0;
    int32_t player_y = 0;

    int32_t is_running = 1;

    while(is_running) {
        
        int bytes_received = recvfrom( *threadData.sock, recvBuffer, SOCKET_BUFFER_SIZE, flags, (SOCKADDR*)&from, &from_size );
        
        if( bytes_received == SOCKET_ERROR )
        {
            printf( "recvfrom returned SOCKET_ERROR, WSAGetLastError() %d", WSAGetLastError() );
            break;
        }
        else
        {
            recvBuffer[bytes_received] = 0;
            printf( "%d.%d.%d.%d:%d - %s", 
            from.sin_addr.S_un.S_un_b.s_b1, 
            from.sin_addr.S_un.S_un_b.s_b2, 
            from.sin_addr.S_un.S_un_b.s_b3, 
            from.sin_addr.S_un.S_un_b.s_b4, 
            from.sin_port, 
            recvBuffer );

            char client_input = recvBuffer[0];

            switch ( client_input ) {
                case 'w':
                    ++player_y;
                    break;
                case 'a':
                    --player_x;
                    break;
                case 's':
                    --player_y;
                    break;
                case 'd':
                    ++player_x;
                    break;
                case 'q':
                    is_running = 0;
                    break;
                default:
                    printf("Unhandled client: %c\n", client_input);
                    break;
            }

            int32_t write_index = 0;
            memcpy( &sendBuffer[write_index], &player_x, sizeof( player_x ) );
            write_index += sizeof( player_x );

            memcpy( &sendBuffer[write_index], &player_y, sizeof( player_y ) );
            write_index += sizeof( player_y );

            memcpy (&sendBuffer[write_index], &is_running, sizeof( is_running ));

            int bufferLength = sizeof( player_x ) + sizeof( player_y ) + sizeof( is_running );
            flags = 0;

            SOCKADDR* to = (SOCKADDR*)&from;
            int toLength = sizeof( from );

            if (sendto( *threadData.sock, sendBuffer, bufferLength, flags, to, toLength) == SOCKET_ERROR) {
                printf( "sendto failed: %d", WSAGetLastError() );
                return 1;
            }


        }
    }

    return 0;
}

void ErrorHandler(LPTSTR lpszFunction);



int main() {
    
    // Forces stdout to be line-buffered.
    setvbuf(stdout, NULL, _IONBF, 0);

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

    ThreadData threadData { &sock };
    DWORD receiveThreadID;
    HANDLE receiveThreadHandle = CreateThread(0, 0, receiveThread, &threadData, 0, &receiveThreadID);

    char myChar = ' ';
    while(myChar != 'q') {
		myChar = getchar();
	}

    CloseHandle(receiveThreadHandle);

    return 0;
}