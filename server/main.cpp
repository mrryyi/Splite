#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include "..\common\pre.h"
#include "..\common\types.h"

#pragma comment(lib, "Ws2_32.lib")

#define SOCKET_BUFFER_SIZE 1024
#define ever ;;

class Message{
public:
    char buffer[SOCKET_BUFFER_SIZE];
    int32_t SOCKADDR_IN_size;
    int32_t flags = 0;
    SOCKADDR_IN address;
    int address_size;
    int32_t bufferLength;
    int bytesReceived = SOCKET_ERROR;

    void SetAddress(SOCKADDR_IN address) {
        this->address = address;
        this->address_size = sizeof(address);
    };

    void PrintAddess() {
        this->buffer[this->bytesReceived] = 0;
        printf( "%d.%d.%d.%d:%d", 
        this->address.sin_addr.S_un.S_un_b.s_b1, 
        this->address.sin_addr.S_un.S_un_b.s_b2, 
        this->address.sin_addr.S_un.S_un_b.s_b3, 
        this->address.sin_addr.S_un.S_un_b.s_b4, 
        this->address.sin_port);
    };
};

namespace ConstructMessageContent {
    void legacyPosition(Message& msg, int32_t x, int32_t y, int32_t is_running){
        int32_t type = MSGTYPE_LEGACYPOSITION;
        int32_t write_index = 0;

        memcpy( &msg.buffer[write_index], &type, sizeof( type ));
        write_index += sizeof( type );

        memcpy( &msg.buffer[write_index], &x, sizeof( x ) );
        write_index += sizeof( x );

        memcpy( &msg.buffer[write_index], &y, sizeof( y ) );
        write_index += sizeof( y );

        memcpy( &msg.buffer[write_index], &is_running, sizeof( is_running ));

        msg.bufferLength = sizeof( type ) + sizeof( x ) + sizeof( y ) + sizeof( is_running );

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
                    s_Msg.address_size) == SOCKET_ERROR) {
            printf( "sendto failed: %d", WSAGetLastError() );
        }
    }

    void MessageLegacy(Message& r_Msg){
        
        // Skip the MSGTYPE
        int32_t read_index = 0;
        int32_t message_type;
        int32_t client_input;
        
        memcpy( &message_type, &r_Msg.buffer[read_index], sizeof( message_type ) );
        read_index += sizeof( message_type );

        memcpy( &client_input, &r_Msg.buffer[read_index], sizeof( client_input ) );
        read_index += sizeof( client_input );

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
    };

    void MessageConnection(Message& r_Msg){

    };

    void HandleMessage(Message& r_Msg) {
        r_Msg.PrintAddess();

        int32_t message_type = -1;
        int32_t message_type_index = 0;
        memcpy( &message_type, &r_Msg.buffer[message_type_index], sizeof( message_type ) );

        switch ( message_type )
        {
            case MSGTYPE_LEGACYPOSITION:
                MessageLegacy(r_Msg);
                break;
            case MSGTYPE_CONNECTION:
                MessageConnection(r_Msg);
                break;
            default:
                printf("Unhandled message type.");
        }

        Message s_Msg;
        ConstructMessageContent::legacyPosition( s_Msg, player_x, player_y, is_running );
        s_Msg.SetAddress(r_Msg.address);
        this->Send(s_Msg);
    };

    void ReceiveThread(){
        while( is_running ) {
            Message recvMsg;    
            recvMsg.bytesReceived = recvfrom( *this->socket, recvMsg.buffer, SOCKET_BUFFER_SIZE, recvMsg.flags, (SOCKADDR*)&recvMsg.address, &recvMsg.address_size );
            
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