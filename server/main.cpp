#include "..\include\pre.h"
#include "..\include\types.h"
#include "..\include\network.h"
#include "..\include\network_messages.h"
#include "..\include\server.h"
#include "..\include\timer.h"
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

    Client(int32 unique_id, SOCKADDR_IN address, int32 player_x = 0, int32 player_y = 0)
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
    int32 is_running = 1;

    std::map<int32, Client*> clients;

    SOCKET* pSocket;

    uint32 lastID = 0;

public:

    Communication(SOCKET* socket) {
        this->pSocket = socket;
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
        
        Network::send( pSocket, s_Msg );

        printf("[ To   ");
        PrintAddress(s_Msg.address);
        printf(" %s]\n", Network::SrvMsgNames[ message_type ]);


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
        this->Send( s_Msg );

    };

    void ConnectionRequestToClient(int32 id) {

        Network::Message s_Msg;
        s_Msg.SetAddress( clients[id]->address );
        Network::Construct::connection( s_Msg, id );
        this->Send( s_Msg );

    };

    void CheckConnection() {
        
        uint64 now = timeSinceEpochMillisec();
        int64 time_since;

        for(auto const& cli : clients) {

            time_since = now - cli.second->last_seen;
            if ( time_since >= MAX_TIME_UNHEARD_FROM_MS ) {
                this->KickClient( cli.first );
                this->RemoveClientFromList( cli.first );
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

};


void input_thread(Communication* pCommunication, bool8* running) {
    char myChar = ' ';
    while(myChar != 'q') {
		myChar = getchar();
        if (myChar == 'p'){
            pCommunication->PrintClients();
        }
	}
    *running = false;
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

    SOCKET sock;

    if( !Network::make_socket( &sock ) )
    {
        printf( "socket failed: %d", WSAGetLastError() );
        WSACleanup();
        return 1;
    }

    SOCKADDR_IN local_address;
    local_address.sin_family = AF_INET;
    local_address.sin_port = htons( 1234 );
    local_address.sin_addr.s_addr = INADDR_ANY;

    if( bind( sock, (SOCKADDR*)&local_address, sizeof( local_address ) ) == SOCKET_ERROR )
    {
        printf( "bind failed: %d", WSAGetLastError() );
        WSACleanup();
        return 1;
    }
    
    bool8 running = true;

    Communication* pComm = new Communication( &sock );
    std::thread input_th(&input_thread, pComm, &running);


    constexpr float32 milliseconds_per_tick = 1000 / SERVER_TICK_RATE;

    int64 now;
    int64 last_check;

    Timer_ms::timer_start();
    int64 ticks = 0;

    printf("[Server started.]\n[Listening...]\n");

    char buffer[SOCKET_BUFFER_SIZE];
    int flags = 0;
    SOCKADDR_IN from;
    int from_size = sizeof( from );

    while( running ) {
        ticks++;
        while (Timer_ms::timer_get_ms_since_start() < milliseconds_per_tick)
        {
            int bytes_received = recvfrom( sock, buffer, SOCKET_BUFFER_SIZE, flags, (SOCKADDR*)&from, &from_size );
            if( bytes_received != SOCKET_ERROR )
            {
                Network::Message r_Msg;
                r_Msg.address = from;
                r_Msg.address_size = from_size;
                memcpy( &r_Msg.buffer, &buffer, SOCKET_BUFFER_SIZE );

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

                switch((Network::ClientMessageType) check.message_type )
                {
                    case Network::ClientMessageType::RegisterRequest:
                        pComm->MessageRegisterRequest( r_Msg );
                        break;
                    case Network::ClientMessageType::RegisterAck:
                        pComm->MessageRegisterAck( r_Msg );
                        break;
                    case Network::ClientMessageType::ConnectionResponse:
                        pComm->MessageConnection( r_Msg );
                        break;
                    case Network::ClientMessageType::Leave:
                        break;
                    case Network::ClientMessageType::Input:
                        break;
                } // End switch message type.

            } // End if socket recv from.

        } // End while timer not reached ms per tick.

        // Restart timer after tick have passed.
        Timer_ms::timer_start();

        // Check the connection of every client.
        now = timeSinceEpochMillisec();
        if ( (now - last_check) >= INTERVAL_CHECK_CLIENT_MS) {
            pComm->CheckConnection();
            last_check = now;
        }

    }

    input_th.join();
    
    printf("Exiting program normally...");

    delete pComm;
    WSACleanup();
    return 0;
}