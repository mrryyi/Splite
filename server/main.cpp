#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <stdio.h>
#include <thread>

#pragma comment(lib, "Ws2_32.lib")

#define SOCKET_BUFFER_SIZE 1024
#define ever ;;

typedef struct ThreadData {
    SOCKET* sock;
};

class Message{
public:
    char buffer[SOCKET_BUFFER_SIZE];
    int32_t SOCKADDR_IN_size;
    int32_t flags = 0;
    SOCKADDR_IN address;
    int addressSize;
    int32_t bufferLength;
    int bytesReceived = SOCKET_ERROR;

    void setAddress(SOCKADDR_IN address) {
        this->address = address;
        this->addressSize = sizeof(address);
    };
};

namespace ConstructMessageContent {
    void legacyPosition(Message& msg, int32_t x, int32_t y, int32_t is_running){

        int32_t write_index = 0;
        memcpy( &msg.buffer[write_index], &x, sizeof( x ) );
        write_index += sizeof( x );

        memcpy( &msg.buffer[write_index], &y, sizeof( y ) );
        write_index += sizeof( y );

        memcpy( &msg.buffer[write_index], &is_running, sizeof( is_running ));

        msg.bufferLength = sizeof( x ) + sizeof( y ) + sizeof( is_running );

    };
};

class Communication {
    int32_t player_x = 0;
    int32_t player_y = 0;
    int32_t is_running = 1;

    int32_t socket_set = 0;
    SOCKET* socket;
public:
    Communication(SOCKET* s) {
        socket = s;
        socket_set = 1;
    };

    int32_t SocketSet(){ return socket_set; };
    
    void Send(Message& s_Msg){
        if (sendto( *this->socket,
                    s_Msg.buffer,
                    s_Msg.bufferLength,
                    s_Msg.flags,
                    (SOCKADDR*)&s_Msg.address,
                    s_Msg.addressSize) == SOCKET_ERROR) {
            printf( "sendto failed: %d", WSAGetLastError() );
        }
    }

    void HandleMessage(Message& r_Msg) {
        r_Msg.buffer[r_Msg.bytesReceived] = 0;
        printf( "%d.%d.%d.%d:%d - %s", 
        r_Msg.address.sin_addr.S_un.S_un_b.s_b1, 
        r_Msg.address.sin_addr.S_un.S_un_b.s_b2, 
        r_Msg.address.sin_addr.S_un.S_un_b.s_b3, 
        r_Msg.address.sin_addr.S_un.S_un_b.s_b4, 
        r_Msg.address.sin_port, 
        r_Msg.buffer );

        char client_input = r_Msg.buffer[0];

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

        Message s_Msg;
        ConstructMessageContent::legacyPosition( s_Msg, player_x, player_y, is_running );
        s_Msg.setAddress(r_Msg.address);
        this->Send(s_Msg);
    };

    void ReceiveThread(){
        while( is_running ) {
            Message recvMsg;    
            recvMsg.bytesReceived = recvfrom( *this->socket, recvMsg.buffer, SOCKET_BUFFER_SIZE, recvMsg.flags, (SOCKADDR*)&recvMsg.address, &recvMsg.addressSize );
            
            if( recvMsg.bytesReceived == SOCKET_ERROR )
            {
                printf( "recvfrom returned SOCKET_ERROR, WSAGetLastError() %d", WSAGetLastError() );
                break;
            }
            else
            {
                HandleMessage(recvMsg);
            }
        }
    };

};  

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

    Communication* pCommunication = new Communication( &sock );
    std::thread th(&Communication::ReceiveThread, pCommunication);

    char myChar = ' ';
    while(myChar != 'q') {
		myChar = getchar();
	}
    
    delete pCommunication;
    return 0;
}