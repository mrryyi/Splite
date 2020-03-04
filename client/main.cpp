#include "..\include\pre.h"
#include "..\include\types.h"
#include <ncurses.h> // getch

#pragma comment(lib, "Ws2_32.lib")
#define NO_ID_GIVEN -1

#define PORT_HERE 1500
#define PORT_SERVER 1234

uint64_t timeSinceEpochMillisec() {
  using namespace std::chrono;
  return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
}

void PrintAddress(SOCKADDR_IN address) {
        printf( "%d.%d.%d.%d:%d", 
        address.sin_addr.S_un.S_un_b.s_b1, 
        address.sin_addr.S_un.S_un_b.s_b2, 
        address.sin_addr.S_un.S_un_b.s_b3, 
        address.sin_addr.S_un.S_un_b.s_b4, 
        address.sin_port);
};

class Message {
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

namespace ConstructMessageContent {
    void legacyPosition(Message& msg, int32_t direction){
        int32_t type = MSGTYPE_LEGACYPOSITION;
        int64_t now = timeSinceEpochMillisec();
        int32_t write_index = 0;

        memcpy( &msg.buffer[write_index], &type, sizeof( type ));
        write_index += sizeof( type );

        memcpy( &msg.buffer[write_index], &now, sizeof( now ));
        write_index += sizeof( now );

        memcpy( &msg.buffer[write_index], &direction, sizeof( direction ) );
        write_index += sizeof( direction );

        msg.bufferLength = sizeof( type ) + sizeof( now ) + sizeof( direction );
    };

    void registerRequest(Message& msg) {
        int32_t type = MSGTYPE_REGISTERREQUEST;
        int64_t now = timeSinceEpochMillisec();
        int32_t write_index = 0;

        memcpy( &msg.buffer[write_index], &type, sizeof( type ));
        write_index += sizeof( type );

        memcpy( &msg.buffer[write_index], &now, sizeof( now ));
        write_index += sizeof( now );

        msg.bufferLength = sizeof( type ) + sizeof( now );
    }

    void registerAck(Message& msg, int32_t id) {
        int32_t type = MSGTYPE_REGISTERACK;
        int64_t now = timeSinceEpochMillisec();
        int32_t write_index = 0;

        memcpy( &msg.buffer[write_index], &type, sizeof( type ));
        write_index += sizeof( type );

        memcpy( &msg.buffer[write_index], &now, sizeof( now ));
        write_index += sizeof( now );

        memcpy( &msg.buffer[write_index], &id, sizeof( id ));
        write_index += sizeof( id );

        msg.bufferLength = sizeof( type ) + sizeof( now ) + sizeof( id );
    }


    void connection(Message& msg, int32_t id) {
        int32_t type = MSGTYPE_CONNECTION;
        int64_t now = timeSinceEpochMillisec();
        int32_t write_index = 0;

        memcpy( &msg.buffer[write_index], &type, sizeof( type ));
        write_index += sizeof( type );

        memcpy( &msg.buffer[write_index], &now, sizeof( now ));
        write_index += sizeof( now );

        memcpy( &msg.buffer[write_index], &id, sizeof( id ));
        write_index += sizeof( id );

        msg.bufferLength = sizeof( type ) + sizeof( now ) + sizeof( id );
    }
};

class Sender {
public:
    SOCKET* socket;
    Sender(SOCKET* s){
        this->socket = s;
    }

    void Send(Message& s_Msg){
        int32_t message_type;
        memcpy( &message_type, &s_Msg.buffer[0], sizeof( message_type ) );
        
        printf("[ To   ");
        PrintAddress(s_Msg.address);
        printf(" %s]", MsgTypeName(message_type));

        if (sendto( *this->socket,
                    s_Msg.buffer,
                    s_Msg.bufferLength,
                    s_Msg.flags,
                    (SOCKADDR*)&s_Msg.address,
                    s_Msg.address_size) == SOCKET_ERROR) {
            printf( "sendto failed: %d", WSAGetLastError() );
        }
    }
};

class Communication {
    SOCKET* pSocket;
    Sender* pSender;
public:

    Communication(SOCKET* socket, Sender* sender) {
        pSocket = socket;
        pSender = sender;
    };

    bool connected = false;
    int32_t id_from_server = NO_ID_GIVEN;

    void MessageLegacy(Message& r_Msg) {
        // grab data from packet
        int32_t type;
        int64_t time_sent;
        int32_t player_x;
        int32_t player_y;
        int32_t read_index = 0;

        memcpy( &type, &r_Msg.buffer[read_index], sizeof( type ));
        read_index += sizeof( type );

        memcpy( &time_sent, &r_Msg.buffer[read_index], sizeof( time_sent ));
        read_index += sizeof( time_sent );

        memcpy( &player_x, &r_Msg.buffer[read_index], sizeof( player_x ) );
        read_index += sizeof( player_x );

        memcpy( &player_y, &r_Msg.buffer[read_index], sizeof( player_y ) );
        read_index += sizeof( player_y );

        printf("[x: %d, y: %d]", player_x, player_y);
    };

    void MessageConnection(Message& r_Msg) {
        
        if (id_from_server != NO_ID_GIVEN) {
            Message s_Msg;
            s_Msg.SetAddress(r_Msg.address);
            ConstructMessageContent::connection(s_Msg, id_from_server);
            this->pSender->Send(s_Msg);
        }
        else {
            printf("Received connection message without having an ID.");
        }
    };

    void MessageRegisterSyn(Message& r_Msg) {
        int32_t read_index = 0;
        int32_t type;
        int32_t id;
        int64_t time_sent;

        memcpy( &type, &r_Msg.buffer[read_index], sizeof( type ));
        read_index += sizeof( type );

        memcpy( &time_sent, &r_Msg.buffer[read_index], sizeof( time_sent ));
        read_index += sizeof( time_sent );

        memcpy( &id, &r_Msg.buffer[read_index], sizeof( id ) );
        read_index += sizeof( id );

        Message s_Msg;
        s_Msg.SetAddress(r_Msg.address);
        ConstructMessageContent::registerAck(s_Msg, id);
        this->pSender->Send(s_Msg);
    };

    void MessageRegisterAccept(Message& r_Msg) {

        int32_t read_index = 0;
        int32_t type;
        int32_t id;
        int64_t time_sent;

        memcpy( &type, &r_Msg.buffer[read_index], sizeof( type ));
        read_index += sizeof( type );

        memcpy( &time_sent, &r_Msg.buffer[read_index], sizeof( time_sent ));
        read_index += sizeof( time_sent );

        memcpy( &id, &r_Msg.buffer[read_index], sizeof( id ) );
        read_index += sizeof( id );

        this->id_from_server = id;
        this->connected = true;
    };

    void HandleMessage(Message& r_Msg) {

        int32_t message_type = -1;
        int32_t read_index = 0;
        int64_t time_sent_ms;
        int64_t now_ms = timeSinceEpochMillisec();

        memcpy( &message_type, &r_Msg.buffer[read_index], sizeof( message_type ) );
        read_index += sizeof( message_type );

        memcpy( &time_sent_ms, &r_Msg.buffer[read_index], sizeof( time_sent_ms ) );
        read_index += sizeof( time_sent_ms );
        
        int64_t ping_ms = now_ms - time_sent_ms;

        printf("[ From ");
        PrintAddress(r_Msg.address);
        printf(" %dms %s]", ping_ms, MsgTypeName(message_type));

        switch ( message_type )
        {
            case MSGTYPE_LEGACYPOSITION:
                MessageLegacy(r_Msg);
                break;
            case MSGTYPE_CONNECTION:
                MessageConnection(r_Msg);
                break;
            case MSGTYPE_REGISTERSYN:
                MessageRegisterSyn(r_Msg);
                break;
            case MSGTYPE_REGISTERACCEPT:
                MessageRegisterAccept(r_Msg);
                break;
            default:
                printf("Unhandled message type.");
        }
    };

    void ReceiveThread(){
        for(ever) {
            Message r_Msg;    
            r_Msg.bytesReceived = recvfrom( *this->pSocket, r_Msg.buffer, SOCKET_BUFFER_SIZE, r_Msg.flags, (SOCKADDR*)&r_Msg.address, &r_Msg.address_size );
            
            if( r_Msg.bytesReceived == SOCKET_ERROR )
            {
                printf( "recvfrom returned SOCKET_ERROR, WSAGetLastError() %d", WSAGetLastError() );
                break;
            }
            else
            {
                HandleMessage( r_Msg );
            }
        }
    };
};

int main() {

    // Forces stdout to be line-buffered.
    setvbuf(stdout, NULL, _IONBF, 0);

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
        printf("WSAStartup failed: %d", iResult);
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

    int32_t write_index;
    int32_t userInput;

    Sender* pSender = new Sender( &sock );
    Communication* pCommunication = new Communication( &sock, pSender );
    std::thread th(&Communication::ReceiveThread, pCommunication);

    int64_t interval_ms = 1000;
    int64_t last_ask = timeSinceEpochMillisec();
    int64_t now;
    
    while (pCommunication->connected != true) {
        now = timeSinceEpochMillisec();
        if (now - last_ask > interval_ms) {
            Message s_Msg;
            ConstructMessageContent::registerRequest(s_Msg);
            s_Msg.SetAddress(server_address);
            pSender->Send(s_Msg);
            last_ask = now;
        }
    }

    printf("Accepted.");

    for(ever) {

        // get input
        userInput = getchar();

        Message s_Msg;
        ConstructMessageContent::legacyPosition(s_Msg, userInput);
        s_Msg.SetAddress(server_address);
        pSender->Send(s_Msg);
    }

    printf("Exiting program normally...");

    delete pCommunication;
    return 0;
}