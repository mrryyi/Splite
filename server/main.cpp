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
    void legacyPosition(Message& msg, int32_t x, int32_t y, int32_t is_running) {
        
        MsgContentLegacyPosition msg_content;
        msg_content.msg_type = MSGTYPE_LEGACYPOSITION;
        msg_content.timestamp_ms = timeSinceEpochMillisec();
        msg_content.x = x;
        msg_content.y = y;
        msg_content.WriteBuffer( msg.buffer );
        msg.bufferLength = msg_content.sizeof_content();

    };

    void registerSyn(Message& msg, int32_t id) {

        MsgContentRegisterSyn msg_content;
        msg_content.msg_type = MSGTYPE_REGISTERSYN;
        msg_content.timestamp_ms = timeSinceEpochMillisec();
        msg_content.id = id;
        msg_content.WriteBuffer( msg.buffer );
        msg.bufferLength = msg_content.sizeof_content();

    };

    void registerAccept(Message& msg, int32_t id) {

        MsgContentRegisterAccept msg_content;
        msg_content.msg_type = MSGTYPE_REGISTERACCEPT;
        msg_content.timestamp_ms = timeSinceEpochMillisec();
        msg_content.id = id;
        msg_content.WriteBuffer( msg.buffer );
        msg.bufferLength = msg_content.sizeof_content();

    };

    void connection(Message& msg, int32_t id) {

        MsgContentConnection msg_content;
        msg_content.msg_type = MSGTYPE_CONNECTION;
        msg_content.timestamp_ms = timeSinceEpochMillisec();
        msg_content.id = id;
        msg_content.WriteBuffer( msg.buffer );
        msg.bufferLength = msg_content.sizeof_content();

    };
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
    };

    void PrintClients() {
        printf("%d registered clients:", clients.size());
        for(auto const& cli : clients) {
            cli.second->PrintShort();
        }
    };


    void ConnectionThread() {
        
        const int64_t interval_check_client_ms = 500;
        const int64_t max_unheard_from_ms = 5000;
        int64_t now;
        int64_t time_since;
        Client* client = nullptr;

        for(ever) {
            now = timeSinceEpochMillisec();
            for( auto const& cli : clients) {
                client = cli.second;
                time_since = now - client->last_seen;
                if (time_since >= max_unheard_from_ms) {
                    // disconnect
                }
                else if (time_since >= interval_check_client_ms) {
                    // send connection packet.
                }

            }
        }
    };

    void MessageLegacy(Message& r_Msg) {
        
        MsgContentLegacyDirection msg_content;
        msg_content.ReadBuffer( r_Msg.buffer );
        
        switch ( msg_content.direction ) {
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
                printf("Unhandled client: %c\n", msg_content.direction);
                break;
        }

        Message s_Msg;
        ConstructMessageContent::legacyPosition( s_Msg, player_x, player_y, is_running );
        s_Msg.SetAddress(r_Msg.address);
        this->pSender->Send(s_Msg);
    };

    void MessageConnection(Message& r_Msg) {
        int32_t message_type;
        int64_t time_sent;
        int32_t id;
        int32_t read_index = 0;

        memcpy( &message_type, &r_Msg.buffer[read_index], sizeof( message_type ) );
        read_index += sizeof( message_type );

        memcpy( &time_sent, &r_Msg.buffer[read_index], sizeof( time_sent ));
        read_index += sizeof( time_sent );

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
            
            MsgContentRegisterAck msg_content;
            msg_content.ReadBuffer(r_Msg.buffer);

            Message s_Msg;
            ConstructMessageContent::registerAccept( s_Msg, msg_content.id );
            s_Msg.SetAddress(r_Msg.address);
            this->pSender->Send(s_Msg);

            Client* new_client = new Client(msg_content.id, r_Msg.address);
            this->clients.insert(std::make_pair(msg_content.id, new_client));
            printf("Registering client: %d", msg_content.id);
            new_client->PrintShort();
            printf("\n");
        }
    }

    void HandleMessage(Message& r_Msg) {

        // The type of message may vary the length of the buffer content,
        // but MsgContentBase::ReadBuffer is smart and only reads about 
        // the members it has. It stops at msg_type and timestamp_ms
        MsgContentBase check;
        check.ReadBuffer(r_Msg.buffer);

        int64_t now_ms = timeSinceEpochMillisec();
        int64_t ping_ms = now_ms - check.timestamp_ms;

        printf("[ From ");
        PrintAddress(r_Msg.address);
        printf(" %dms %s]\n", ping_ms, MsgTypeName(check.msg_type));

        switch ( check.msg_type )
        {
            case MSGTYPE_LECACYDIRECTION:
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