#include "..\include\pre.h"
#include "..\include\types.h"
#include "..\include\network.h"
#include "..\include\network_messages.h"
#include "..\include\server.h"
#include <map>

#pragma comment(lib, "Ws2_32.lib")

class Client {
public:
    int32 player_x = 0;
    int32 player_y = 0;
    int32 unique_id;
    SOCKADDR_IN address;
    
    int64 last_seen;
    int64 last_asked;

    Client(int32_t unique_id, SOCKADDR_IN address, int32_t player_x = 0, int32_t player_y = 0)
    : unique_id(unique_id), address(address), player_x(player_x), player_y(player_y)
    {

        this->last_seen = timeSinceEpochMillisec();
    };
    ~Client(){};

    void PrintShort() {
        printf("[id:%d, address: ", unique_id);
        PrintAddress(this->address);
        printf("]");
    }
};

class Communication {
    int32 player_x = 0;
    int32 player_y = 0;
    int32 is_running = 1;

    std::map<int32, Client*> clients;

    Network::Sender* pSender;
    Network::Sender* pSenderConn;
    SOCKET* pSocket;
    SOCKET* pSocketConn;

    uint32 lastID = 0;

public:

    Communication(Network::Sender* sender, Network::Sender* sender_conn, SOCKET* socket, SOCKET* socket_conn) {
        this->pSender = sender;
        this->pSenderConn = sender_conn;
        this->pSocket = socket;
        this->pSocketConn = socket_conn;
    };
    
    uint32 NextUniqueID(){
        lastID += 1;
        return lastID;
    };
    
    uint32 AmountOfConnectedClients() {
        return clients.size();
    }

    void PrintClients() {

        printf("%d registered clients:", clients.size());
        for(auto const& cli : clients) {
            cli.second->PrintShort();
        }

    };

    void Send(Network::Message& s_Msg){

        uint8 message_type;
        memcpy( &message_type, &s_Msg.buffer[0], sizeof( message_type ) );

        printf("[ To   ");
        PrintAddress(s_Msg.address);
        printf(" %s]\n", Network::SrvMsgNames[ message_type ]);

        this->pSender->Send(s_Msg);

    };

    void SendConnection(Network::Message& s_Msg) {

        uint8 message_type;
        memcpy( &message_type, &s_Msg.buffer[0], sizeof( message_type ) );

        printf("[ To   ");
        PrintAddress(s_Msg.address);
        printf(" %s]\n", Network::SrvMsgNames[ message_type ]);
        
        this->pSenderConn->Send(s_Msg);

    };

    void RemoveClientFromList(int32 id) {

        printf("Removed client " );
        clients[id]->PrintShort();
        printf(".\n");
        clients[id]->~Client();
        clients.erase( id );

    };

    void KickClient(int32 id) {

        Network::Message s_Msg;
        s_Msg.SetAddress( clients[id]->address );
        Network::Construct::kicked( s_Msg );
        this->SendConnection( s_Msg );

        this->RemoveClientFromList(id);

    };

    void ConnectionRequestToClient(int32 id) {

        Network::Message s_Msg;
        s_Msg.SetAddress( clients[id]->address );
        Network::Construct::connection( s_Msg, id );
        this->SendConnection( s_Msg );

    };

    void CheckConnection() {
        
        uint64 now = timeSinceEpochMillisec();
        int64 time_since;

        for(auto const& cli : clients) {

            time_since = now - cli.second->last_seen;
            if ( time_since >= MAX_TIME_UNHEARD_FROM_MS ) {
                this->KickClient( cli.first );
            }
            // Ask only if either time_since surpassed the interval to check.
            // If we've already asked within this interval, don't spam the client.
            else if ( time_since >= INTERVAL_CHECK_CLIENT_MS &&
                      (now - cli.second->last_asked) >= INTERVAL_CHECK_CLIENT_MS) {
                this->ConnectionRequestToClient( cli.first );
                cli.second->last_asked = now;
            };

        };

    };

    void ConnectionThreadCheck() {

        for(ever) {
            this->CheckConnection();
        }

    };

    void MessageConnection(Network::Message& r_Msg) {

        Network::MsgContentID msg_content;
        msg_content.Read( r_Msg.buffer );
        
        if ( clients.count( msg_content.id ) ) {
            clients[ msg_content.id ]->last_seen = timeSinceEpochMillisec();
        }

    };

    // Request to register as client from client to server
    // We send a syn message, to get an ack message back,
    // and THEN we accept them.
    void MessageRegisterRequest(Network::Message& r_Msg) {

        Network::Message s_Msg;
        s_Msg.SetAddress(r_Msg.address);
        // This is the id we will pass back and forth.
        // This will carry over to when we receive MSGTYPE_REGISTERACK
        // and becomes the id for that client.
        uint32 id = this->NextUniqueID();
        Network::Construct::register_syn(s_Msg, id);
        this->Send(s_Msg);

    };

    void MessageRegisterAck(Network::Message& r_Msg) {
        // !Todo: make this acceptance test read the message
        
        bool8 accepted = false;

        if (AmountOfConnectedClients() < MAX_CLIENTS_CONNECTED){
            accepted = true;
        }

        if (accepted) {
            
            Network::MsgContentID msg_content;
            msg_content.Read(r_Msg.buffer);

            Network::Message s_Msg;
            Network::Construct::register_result( s_Msg, msg_content.id );
            s_Msg.SetAddress(r_Msg.address);
            this->Send(s_Msg);

            Client* new_client = new Client(msg_content.id, r_Msg.address);
            this->clients.insert(std::make_pair(msg_content.id, new_client));
            printf("Registering client: %d", msg_content.id);
            new_client->PrintShort();
            printf("\n");

        }
    }

    void HandleMessage(Network::Message& r_Msg) {

        // The type of message may vary the length of the buffer content,
        // but MsgContentBase::ReadBuffer is smart and only reads about 
        // the members it has. It stops at msg_type and timestamp_ms
        Network::MsgContentBase check;
        check.Read(r_Msg.buffer);

        int64 now_ms = timeSinceEpochMillisec();
        int64 ping_ms = now_ms - (int64) check.timestamp_ms;

        printf("[ From ");
        PrintAddress(r_Msg.address);
        printf(" %dms %s]\n", ping_ms, Network::CliMsgNames[ check.message_type ]);

        switch ( (Network::ClientMessageType) check.message_type )
        {
            case Network::ClientMessageType::RegisterRequest:
                MessageRegisterRequest( r_Msg );
                break;
            case Network::ClientMessageType::RegisterAck:
                MessageRegisterAck( r_Msg );
                break;
            case Network::ClientMessageType::ConnectionResponse:
                MessageConnection( r_Msg );
                break;
            case Network::ClientMessageType::Input:
                break;
            default:
                printf("Unhandled message type.");
        }
        
    };

    void ConnectionThreadRecv() {

        while ( is_running ) {
            Network::Message r_Msg;    
            r_Msg.bytesReceived = recvfrom( *this->pSocketConn, (char *) r_Msg.buffer, SOCKET_BUFFER_SIZE, r_Msg.flags, (SOCKADDR*)&r_Msg.address, &r_Msg.address_size );

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

    void ReceiveThread() {

        while( is_running ) {
            Network::Message r_Msg;    
            r_Msg.bytesReceived = recvfrom( *this->pSocket, (char *) r_Msg.buffer, SOCKET_BUFFER_SIZE, r_Msg.flags, (SOCKADDR*)&r_Msg.address, &r_Msg.address_size );
            
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

    SOCKET sock_connection = socket( address_family, type, protocol );

    SOCKADDR_IN local_address_conn;
    local_address_conn.sin_family = AF_INET;
    local_address_conn.sin_port = htons( 1235 );
    local_address_conn.sin_addr.s_addr = INADDR_ANY;

    if( bind( sock_connection, (SOCKADDR*)&local_address_conn, sizeof( local_address_conn ) ) == SOCKET_ERROR )
    {
        printf( "bind failed: %d", WSAGetLastError() );
        return 1;
    }

    Network::Sender* pSender = new Network::Sender( &sock );
    Network::Sender* pSenderConn = new Network::Sender( &sock_connection );
    Communication* pCommunication = new Communication( pSender, pSenderConn, &sock, &sock_connection );
    std::thread recv_thread(&Communication::ReceiveThread, pCommunication);

    std::thread conn_check_thread(&Communication::ConnectionThreadCheck, pCommunication);
    std::thread conn_recv_thread(&Communication::ConnectionThreadRecv, pCommunication);

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