#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include "..\include\pre.h"
#include "..\include\types.h"

#include <map>

#pragma comment(lib, "Ws2_32.lib")

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

        msg.bufferLength = sizeof( type ) + sizeof( x ) + sizeof( y );

    };

    void registerSyn(Message& msg, int32_t id) {
        int32_t type = MSGTYPE_REGISTERSYN;
        int32_t write_index = 0;

        memcpy( &msg.buffer[write_index], &type, sizeof( type ));
        write_index += sizeof( type );

        memcpy( &msg.buffer[write_index], &id, sizeof( id ));
        write_index += sizeof( id );

        msg.bufferLength = sizeof( type ) + sizeof( id );
    };

    void registerAccept(Message& msg, int32_t id) {
        int32_t type = MSGTYPE_REGISTERACCEPT;
        int32_t write_index = 0;

        memcpy( &msg.buffer[write_index], &type, sizeof( type ));
        write_index += sizeof( type );

        memcpy( &msg.buffer[write_index], &id, sizeof( id ));
        write_index += sizeof( id );

        msg.bufferLength = sizeof( type ) + sizeof( id );
    };

    void connection(Message& msg) {
        int32_t type = MSGTYPE_CONNECTION;
        int32_t write_index = 0;

        memcpy( &msg.buffer[write_index], &type, sizeof( type ));
        write_index += sizeof( type );

        msg.bufferLength = sizeof( type );
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
        
        printf("[   To ");
        PrintAddress(s_Msg.address);
        printf(" %s]\n", MsgTypeName(message_type));

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

class Client {
public:
    int32_t player_x = 0;
    int32_t player_y = 0;
    int32_t unique_id;
    SOCKADDR_IN address;
    
    int32_t last_seen;

    Client(int32_t unique_id, SOCKADDR_IN address, int32_t player_x = 0, int32_t player_y = 0)
    : unique_id(unique_id), address(address), player_x(player_x), player_y(player_y)
    {};
    ~Client(){};

    void PrintShort() {
        printf("[id:%d, address: ", unique_id);
        PrintAddress(this->address);
        printf("]");
    }
};

class Communication {
    int32_t player_x = 0;
    int32_t player_y = 0;
    int32_t is_running = 1;

    std::map<int32_t, Client*> clients;

    Sender* pSender;
    SOCKET* pSocket;

    int32_t lastID = 0;

public:

    Communication(Sender* sender, SOCKET* socket) {
        this->pSender = sender;
        this->pSocket = socket;
    };
    
    int32_t NextUniqueID(){
        lastID += 1;
        return lastID;
    }

    void PrintClients() {
        printf("%d registered clients:", clients.size());
        for(auto const& cli : clients) {
            cli.second->PrintShort();
        }
    }


    void ConnectionThread() {

    }

    void MessageLegacy(Message& r_Msg){
        
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

        Message s_Msg;
        ConstructMessageContent::legacyPosition( s_Msg, player_x, player_y, is_running );
        s_Msg.SetAddress(r_Msg.address);
        this->pSender->Send(s_Msg);
    };

    void MessageConnection(Message& r_Msg) {
        int32_t read_index = 0;
        int32_t message_type;
        int32_t id;

        memcpy( &message_type, &r_Msg.buffer[read_index], sizeof( message_type ) );
        read_index += sizeof( message_type );

        memcpy( &id, &r_Msg.buffer[read_index], sizeof( id ) );
        read_index += sizeof( id );

    };

    // Request to register as client from client to server
    // We send a syn message, to get an ack message back,
    // and THEN we accept them.
    void MessageRegisterRequest(Message& r_Msg) {
        Message s_Msg;
        s_Msg.SetAddress(r_Msg.address);
        // This is the id we will pass back and forth.
        // This will carry over to when we receive MSGTYPE_REGISTERACK
        // and becomes the id for that client.
        uint32_t id = this->NextUniqueID();
        ConstructMessageContent::registerSyn(s_Msg, id);
        this->pSender->Send(s_Msg);
    };

    void MessageRegisterAck(Message& r_Msg) {
        // !Todo: make this acceptance test read the message
        bool accepted = true;

        if (accepted) {
            int32_t read_index = sizeof( int32_t ); // Size of first message variable (type)
            int32_t id;

            memcpy( &id, &r_Msg.buffer[read_index], sizeof( id ) );

            Message s_Msg;
            ConstructMessageContent::registerAccept( s_Msg, id );
            s_Msg.SetAddress(r_Msg.address);
            this->pSender->Send(s_Msg);


            Client* new_client = new Client(id, r_Msg.address);
            this->clients.insert(std::make_pair(id, new_client));
            printf("Registering client: %d", id);
            new_client->PrintShort();
            printf("\n");
        }
    }

    void HandleMessage(Message& r_Msg) {

        int32_t message_type = -1;
        int32_t message_type_index = 0;
        memcpy( &message_type, &r_Msg.buffer[message_type_index], sizeof( message_type ) );
        
        printf("[ From ");
        PrintAddress(r_Msg.address);
        printf(" %s]\n", MsgTypeName(message_type));

        switch ( message_type )
        {
            case MSGTYPE_LEGACYPOSITION:
                MessageLegacy( r_Msg );
                break;
            case MSGTYPE_CONNECTION:
                MessageConnection( r_Msg );
                break;
            case MSGTYPE_REGISTERREQUEST:
                MessageRegisterRequest( r_Msg );
                break;
            case MSGTYPE_REGISTERACK:
                MessageRegisterAck( r_Msg );
                break;
            default:
                printf("Unhandled message type.");
        }
        
    };

    void ReceiveThread(){
        while( is_running ) {
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

class ClientHandler {
public:
    std::map<int32_t, Client*> clients;
    ClientHandler() {

    }
};

class GameHandler {

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

    Sender* pSender = new Sender( &sock );
    Communication* pCommunication = new Communication( pSender, &sock );
    std::thread th(&Communication::ReceiveThread, pCommunication);

    char myChar = ' ';
    while(myChar != 'q') {
		myChar = getchar();
        if (myChar == 'p'){
            pCommunication->PrintClients();
        }
	}
    
    printf("Exiting program normally...");

    delete pCommunication;
    return 0;
}