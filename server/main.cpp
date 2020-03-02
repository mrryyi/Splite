#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include "..\include\pre.h"
#include "..\include\types.h"

#pragma comment(lib, "Ws2_32.lib")

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

        msg.bufferLength = sizeof( type ) + sizeof( x ) + sizeof( y );

    };

    void connectionCommand(Message& msg) {
        int32_t type = MSGTYPE_CONNECTION;
        int32_t write_index = 0;

        memcpy( &msg.buffer[write_index], &type, sizeof( type ) );
        write_index += sizeof( type );

        msg.bufferLength = sizeof( type );
    }

    void registrationAccepted(Message& msg) {
        int32_t type = MSGTYPE_REGISTERACCEPTED;
        int32_t write_index = 0;

        memcpy( &msg.buffer[write_index], &type, sizeof( type ) );
        write_index += sizeof( type );

        msg.bufferLength = sizeof( type );
    }
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
            Message r_Msg;    
            r_Msg.bytesReceived = recvfrom( *this->socket, r_Msg.buffer, SOCKET_BUFFER_SIZE, r_Msg.flags, (SOCKADDR*)&r_Msg.address, &r_Msg.address_size );
            
            if( r_Msg.bytesReceived == SOCKET_ERROR )
            {
                printf( "recvfrom returned SOCKET_ERROR, WSAGetLastError() %d", WSAGetLastError() );
                break;
            }
            else
            {
                HandleMessage( r_Msg);
            }
        }
    };
};

class Client {
    int32_t player_x = 0;
    int32_t player_y = 0;
    int32_t unique_id;
    int32_t address;
    int32_t connected;
    int64_t last_seen;
    
public:
    Client(int32_t unique_id, int32_t address, int32_t player_x = 0, int32_t player_y = 0)
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
    void Set_last_seen(int32_t) {
        this->last_seen = last_seen;
    };

    int32_t Get_player_x() {
        return player_x;
    };
    int32_t Get_player_y() {
        return player_y;
    };
    int32_t Get_unique_id() {
        return unique_id;
    };
    int32_t Get_address() {
        return address;
    };
    int32_t Get_connected() {
        return connected;
    };
    int32_t Get_last_seen() {
        return last_seen;
    };
};

class ClientStore() {
public:
    std::map< int32_t, Client* > clients;

    void AddNewClient( Client& new_client ) {
        clients.insert( std::make_pair( new_client, new_client.Get_unique_id() ) );
    }
};

// Could probably be called something like connection handler.
// ToDo: learn coding semantic guidelines.
class ConnectionHandler {
    int32_t max_clients;
    ClientStore p_ClientStore;
public:
    ClientHandler(ClientStore* client_store, int32_t max_clients = 16) {
        this->p_ClientStore = client_store;
        this->max_clients = max_clients;
    };

    void ConnectionMessageHandler(Message& r_Msg){
        // Find client unique ID, then update clients[unique ID]
    }

    void ConnectionThread(){
        for(int i = 0; i < clients.size(); i++){
            // if maxtimebetweenlastseen < client.lastseen
            // then disconnected
            // if timetocheck
            // send a connection message
        }
    }

    void AddNewClient() {
        // Add new client to map of clients    
    }
};

// Could be called something like GameLogic. Will be implemented.
class GameLogicHandler {

    ClientStore* p_clients = nullptr;

public:
    GameLogicHandler(ClientStore* clients){
        p_clients = clients;
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
};


class MessageHandler {

    void MessageBroker(Message& r_Msg) {

        int32_t message_type = -1;
        int32_t message_type_index = 0;
        memcpy( &message_type, &r_Msg.buffer[message_type_index], sizeof( message_type ) );

        switch ( message_type )
        {
            case MSGTYPE_LEGACYPOSITION:
                this->pGameLogicHandler.LegacyPositionMessageHandler(r_Msg);
                break;
            case MSGTYPE_CONNECTION:
                this->pConnectionHandler.ConnectionMessageHandler(r_Msg);
                break;
            default:
                printf("Unhandled message type.");
        }
    };

    bool thread_is_running = false;

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

    void ReceiveThread() {

        if ( this->ValidReferences ) {
            for( ever ) {
                Message r_Msg;    
                r_Msg.bytesReceived = recvfrom( *this->socket, r_Msg.buffer, SOCKET_BUFFER_SIZE, r_Msg.flags, (SOCKADDR*)&r_Msg.address, &r_Msg.address_size );
                
                if( r_Msg.bytesReceived == SOCKET_ERROR )
                {
                    printf( "recvfrom returned SOCKET_ERROR, WSAGetLastError() %d", WSAGetLastError() );
                    break;
                }
                else
                {
                    std::thread th(&MessageHandler::UnderstandMessage, this);
                    this->MessageBroker( r_Msg );
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

    //Communication* pCommunication = new Communication( &sock );
    //std::thread commThread(&Communication::ReceiveThread, pCommunication);

    ClientStore*       pClientStore = new ClientStore( 20 );
    ConnectionHandler* pConnectionHandler = new ConnectionHandler( pClientStore );
    GameLogicHandler*  pGameLogicHandler = GameLogicHandler( pClientStore );
    MessageHandler*    pMessageHandler = new MessageHandler( &sock, pGameLogicHandler, pConnectionHandler );

    pMessageHandler->connection_handler = pGameLogicHandler;

    std::thread message_handler_thread(&MessageHandler::ReceiveThread, pMessageHandler);

    char myChar = ' ';
    while(myChar != 'q') {
		myChar = getchar();
	}
    
    printf("Exiting program normally...");

    delete pConnectionHandler;
    delete pClientStore;
    delete pGameLogicHandler;
    delete pMessageHandler;
    
    return 0;
}