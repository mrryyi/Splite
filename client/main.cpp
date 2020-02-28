#include "..\include\pre.h"
#include "..\include\types.h"
#include <ncurses.h> // getch

#pragma comment(lib, "Ws2_32.lib")

#define PORT_HERE 1500
#define PORT_SERVER 1234

typedef struct ThreadData {
    SOCKET* sock;
    int32_t is_running = 1;
};

DWORD WINAPI receiveThread( _Inout_ LPVOID lpParam) {
    
    ThreadData& threadData = *((ThreadData*)lpParam);

    char recvbuffer[SOCKET_BUFFER_SIZE];
    int flags = 0;
    SOCKADDR_IN from;
    int from_size = sizeof( from );
    int bytes_received = 0;

    int32_t message_type = -1;
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

            memcpy( &message_type, &recvbuffer[read_index], sizeof( message_type ));
            read_index += sizeof( message_type );

            memcpy( &player_x, &recvbuffer[read_index], sizeof( player_x ) );
            read_index += sizeof( player_x );

            memcpy( &player_y, &recvbuffer[read_index], sizeof( player_y ) );
            read_index += sizeof( player_y );

            memcpy( &is_running, &recvbuffer[read_index], sizeof( is_running ) );

            threadData.is_running = is_running;
            
            printf( "[Type: %s, x:%d, y:%d, is_running:%d]", MsgTypeName(message_type), player_x, player_y, is_running );
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

    int32_t write_index;
    int32_t message_type = MSGTYPE_LEGACYPOSITION;
    int32_t userInput;
    while (threadData.is_running) {

        // get input
        userInput = getchar();

        write_index = 0;
        memcpy( &buffer[write_index], &message_type, sizeof( message_type ));
        write_index += sizeof( message_type );

        memcpy( &buffer[write_index], &userInput, sizeof( userInput ));
        write_index += sizeof( userInput );


        // send to server
        int buffer_length = sizeof ( message_type ) + sizeof( userInput );
        int flags = 0;
        SOCKADDR* to = (SOCKADDR*)&server_address;
        int to_length = sizeof( server_address );
        if( sendto( sock, buffer, buffer_length, flags, to, to_length ) == SOCKET_ERROR )
        {
            printf( "sendto failed: %d", WSAGetLastError() );
            return 0;
        }
        
    }

    printf("[Program exit: threadData.is_running:%d]", threadData.is_running);

    CloseHandle(receiveThreadHandle);

    return 1;
}