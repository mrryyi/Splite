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

void printAddress(SOCKADDR_IN address) {
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

    void requestConnection(Message& msg) {
        int32_t type = MSGTYPE_CONNECTION;
        int32_t write_index = 0;

        memcpy( &msg.buffer[write_index], &type, sizeof( type ) );
        write_index += sizeof( type );

        msg.bufferLength = sizeof( type );
    };

    void registrationAccepted(Message& msg, int32_t unique_id) {
        int32_t type = MSGTYPE_REGISTERACCEPTED;
        int32_t write_index = 0;

        memcpy( &msg.buffer[write_index], &type, sizeof( type ) );
        write_index += sizeof( type );

        memcpy( &msg.buffer[write_index], &unique_id, sizeof( unique_id ));

        msg.bufferLength = sizeof( type ) + sizeof ( unique_id );
    };
};

class Sender {
    int32_t player_x = 0;
    int32_t player_y = 0;
    int32_t is_running = 1;

    int32_t socket_set = 0;
    SOCKET* socket;
public:
    Sender(SOCKET* s) {
        socket = s;
        socket_set = 1;
    };

    int32_t SocketSet() { return socket_set; };
    
    void Send(const Message& s_Msg){
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
    int32_t player_x = 0;
    int32_t player_y = 0;
    int32_t connected;
    uint64_t last_seen_ms;
    uint64_t last_asked_ms;
    
    int32_t unique_id;
    SOCKADDR_IN address;
public:
    Client(int32_t unique_id, SOCKADDR_IN address, int32_t player_x = 0, int32_t player_y = 0)
    : unique_id(unique_id), address(address), player_x(player_x), player_y(player_y)
    {};
    ~Client(){};


    void Set_player_x(int32_t player_x) {
        this->player_x = player_x;
    };
    void Set_player_y(int32_t player_y) {
        this->player_y = player_y;
    };
    void Set_connected(int32_t connected){
        this->connected = connected;
    };
    void Set_last_seen_ms(uint64_t last_seen) {
        this->last_seen_ms = last_seen;
    };
    void Set_last_asked_ms(uint64_t last_asked) {
        this->last_asked_ms = last_asked;
    }

    int32_t Get_player_x() {
        return player_x;
    };
    int32_t Get_player_y() {
        return player_y;
    };
    int32_t Get_unique_id() {
        return unique_id;
    };
    SOCKADDR_IN Get_address() {
        return address;
    };
    int32_t Get_connected() {
        return connected;
    };
    uint64_t Get_last_seen_ms() {
        return last_seen_ms;
    };
    uint64_t Get_last_asked_ms() {
        return last_asked_ms;
    }
};

// Could probably be called something like connection handler.
// ToDo: learn coding semantic guidelines.
class ConnectionHandler {
    int32_t max_clients;
    int32_t client_counter;
    Sender* pSender;
    std::map< int32_t, Client* > clients;
    int32_t next_unique_id = 1;

    int32_t generate_unique_id(){
        return ++next_unique_id;
    }

public:

    ConnectionHandler(Sender* sender, int32_t max_clients = 16) {
        this->pSender = sender;
        this->max_clients = max_clients;
    };

    void ConnectionMessageHandler(const Message& r_Msg) {
        int32_t type;
        int32_t client_id;
        int32_t read_index = sizeof (type);

        memcpy( &client_id, &r_Msg.buffer[read_index], sizeof( client_id ));
        
        UpdateLastSeen( client_id );
        printAddress(clients[client_id]->Get_address());
        printf(" connection OK.");
    };

    void RegistrationRequestHandler(const Message& r_Msg) {
        if ( client_counter < max_clients ) {

            int32_t id = this->generate_unique_id();
            Client* client = new Client(id, r_Msg.address);
            
            UpdateLastSeen( id );

            Message s_Msg;
            s_Msg.SetAddress( client->Get_address() );
            ConstructMessageContent::registrationAccepted( s_Msg, id );
            this->pSender->Send( s_Msg );

            this->clients.insert( std::make_pair(id, client) );
            this->client_counter++;

            printAddress(clients[id]->Get_address());
            printf(" registered.");
        }
    };

    void RequestConnectionMessage(Client* clientToMessage) {
        Message s_Msg;
        s_Msg.SetAddress(clientToMessage->Get_address());
        ConstructMessageContent::requestConnection(s_Msg);
        this->pSender->Send(s_Msg);
    };

    void RemoveClient(const int32_t id) {
        if ( clients.find(id) != clients.end() ) {
            delete clients[id];
            clients.erase(id);
        }
    };

    void UpdateLastSeen(const int32_t unique_id) {
        if (clients.find(unique_id) != clients.end()){
            clients[unique_id]->Set_last_seen_ms( timeSinceEpochMillisec() );
        }
    };

    void ConnectionThread() {
        uint64_t delta_time_seen;
        uint64_t delta_time_asked;
        uint64_t now_milli;
        
        for ( ever ) {
            
            now_milli = timeSinceEpochMillisec();

            for(auto const& cli : this->clients) {
                delta_time_seen = now_milli - cli.second->Get_last_seen_ms();
                
                if (delta_time_seen >= MAX_TIME_BEFORE_DISCONNECT_MS) {
                    this->RemoveClient(cli.first);
                }
                else if (delta_time_seen >= TIME_BEFORE_CHECKING_CONNECTION) {

                    delta_time_asked = now_milli - cli.second->Get_last_asked_ms();
                    if (delta_time_asked >= INTERVAL_CHECK_CONNECTION_MS) {
                        printAddress(cli.second->Get_address());
                        printf(" connection check.");
                        cli.second->Set_last_asked_ms( now_milli );
                        std::thread req_msg_thread(&ConnectionHandler::RequestConnectionMessage, this, cli.second);
                    }
                }
            }
        }
    };

    void AddNewClient() {
        // Add new client to map of clients
    };
};

// Could be called something like GameLogic. Will be implemented.
class GameLogicHandler {

public:
    GameLogicHandler(){
    };

    void LegacyPositionMessageHandler(const Message& r_Msg) {

    };
};


class MessageHandler {

public:
    ConnectionHandler* pConnectionHandler = nullptr;
    GameLogicHandler* pGameLogicHandler = nullptr;
    SOCKET* pSocket = nullptr;

    MessageHandler(SOCKET* socket, ConnectionHandler* connection_handler, GameLogicHandler* game_logic_handler) {
        this->pSocket = socket;
        this->pConnectionHandler = connection_handler;
        this->pGameLogicHandler = game_logic_handler;
    }

    bool ValidReferences(){
        if (pConnectionHandler != nullptr &&
            pGameLogicHandler != nullptr &&
            pSocket != nullptr)
        {
            return true;
        }
        else {
            return false;
        }
    };

    void MessageBroker(const Message& r_Msg) {

        int32_t message_type = -1;
        int32_t message_type_index = 0;
        memcpy( &message_type, &r_Msg.buffer[message_type_index], sizeof( message_type ) );


        switch ( message_type )
        {
            case MSGTYPE_LEGACYPOSITION:
                this->pGameLogicHandler->LegacyPositionMessageHandler(r_Msg);
                break;
            case MSGTYPE_CONNECTION:
                this->pConnectionHandler->ConnectionMessageHandler(r_Msg);
                break;
            case MSGTYPE_REGISTERREQUEST:
                this->pConnectionHandler->RegistrationRequestHandler(r_Msg);
            default:
                printf("Unhandled message type [%d]", MsgTypeName(message_type));
                printf("From: ");
                printAddress(r_Msg.address);
        }
    };

    // https://stackoverflow.com/questions/10673585/start-thread-with-member-function
    /*std::thread MessageBrokerThread(Message& r_Msg) {
        return std::thread([=] { MessageBroker(r_Msg); });
    };*/

    void ReceiveThread() {
        
        if ( this->ValidReferences() ) {
            for( ever ) {
                Message r_Msg;
                r_Msg.bytesReceived = recvfrom( *this->pSocket, r_Msg.buffer, SOCKET_BUFFER_SIZE, r_Msg.flags, (SOCKADDR*)&r_Msg.address, &r_Msg.address_size );

                if( r_Msg.bytesReceived == SOCKET_ERROR )
                {
                    printf( "recvfrom returned SOCKET_ERROR, WSAGetLastError() %d", WSAGetLastError() );
                    r_Msg.PrintAddress();
                    break;
                }
                else
                {
                    //std::thread message_broker_thread = this->MessageBrokerThread(r_Msg);
                    std::thread th(&MessageHandler::MessageBroker, this, std::ref(r_Msg));
                    //message_broker_thread.detach();
                }
            }
        }
        else {
            printf("ReceiveThread Error: Invalid references.");
        }
    };
};

int main(int argc, char *argv[]) {
    
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
    if ( iResult != 0 ) {
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



    Sender*             pSender = new Sender( &sock );
    ConnectionHandler*  pConnectionHandler = new ConnectionHandler( pSender );
    GameLogicHandler*   pGameLogicHandler = new GameLogicHandler();
    MessageHandler*     pMessageHandler = new MessageHandler( &sock, pConnectionHandler, pGameLogicHandler );

    std::thread message_handler_thread(&MessageHandler::ReceiveThread, pMessageHandler);
    message_handler_thread.detach();

    char myChar = ' ';
    while(myChar != 'q') {
		myChar = getchar();
	}
    
    printf("Exiting program normally...");

    delete pConnectionHandler;
    delete pSender;
    delete pGameLogicHandler;
    delete pMessageHandler;
    
    return 0;
}