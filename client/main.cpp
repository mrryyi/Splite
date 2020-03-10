#include "..\include\pre.h"
#include "..\include\types.h"
#include "..\include\network_messages.h"
#include "..\include\client.h"
#include "..\include\timer.h"

/*
https://how-to.fandom.com/wiki/How_to_setup_a_Simple_Direct_Media_Layer(SDL)_build_environment
*/

#include <ncurses.h> // getch

#pragma comment(lib, "Ws2_32.lib")
#define NO_ID_GIVEN -1

#define PORT_HERE 1500
#define PORT_SERVER 1234

class Communication {
    SOCKET* pSocket;
public:

    Communication(SOCKET* socket) {
        pSocket = socket;
    };

    bool connected = false;
    int32_t id_from_server = NO_ID_GIVEN;

    void Send(Network::Message& s_Msg) {

        uint8 message_type;
        memcpy( &message_type, &s_Msg.buffer[0], sizeof( message_type ) );

        printf("[ To   ");
        PrintAddress(s_Msg.address);
        printf(" %s]\n", Network::CliMsgNames[ message_type ]);
        Network::send( pSocket, s_Msg);
        
    }

    void MessageRegisterSyn(Network::Message& r_Msg) {
        
        Network::MsgContentID msg_content;
        msg_content.Read( r_Msg.buffer );

        Network::Message s_Msg;
        s_Msg.SetAddress( r_Msg.address );
        Network::Construct::register_ack( s_Msg, msg_content.id );
        this->Send( s_Msg );

    };

    void MessageRegisterResult(Network::Message& r_Msg) {

        Network::MsgContentID msg_content;
        msg_content.Read( r_Msg.buffer );

        this->id_from_server = msg_content.id;
        this->connected = true;
        
    };

    void MessageConnection(Network::Message& r_Msg) {
        
        if (id_from_server != NO_ID_GIVEN) {

            Network::Message s_Msg;
            s_Msg.SetAddress( r_Msg.address );
            Network::Construct::connection( s_Msg, id_from_server );
            this->Send( s_Msg );

        }
        else {
            printf("Received connection message without having an ID.");
        }

    };

    void MessageGameState(Network::Message& r_Msg) {

    };

    void MessageKicked(Network::Message& r_Msg) {
        printf("Got kicked from the server.");
        this->connected = false;
        this->id_from_server = NO_ID_GIVEN;
    };

};

int main() {

    // Forces stdout to be line-buffered.
    setvbuf(stdout, NULL, _IONBF, 0);

    // Initialize ncurses in order to make getch() into a blocking function.
    //WINDOW *w;
    //w = initscr();
    
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

    SOCKET sock;

    if( !Network::make_socket( &sock ) )
    {
        printf( "socket failed: %d", WSAGetLastError() );
        WSACleanup();
        return 1;
    }

    srand( (unsigned) time(NULL) );
    u_short random_port = 1000 + rand() % 56000;

    SOCKADDR_IN local_address;
    local_address.sin_family = AF_INET;
    local_address.sin_port = random_port;//htons( PORT_HERE );
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

    int32 userInput;
    Player::PlayerInput input;

    Communication* pCommunication = new Communication( &sock );

    int64 interval_ms = 1000;
    int64 last_ask = timeSinceEpochMillisec();
    int64 now;

    constexpr float32 milliseconds_per_tick = 1000 / ((float32) CLIENT_TICK_RATE);
    Timer_ms::timer_start();

    char buffer[SOCKET_BUFFER_SIZE];
    int flags = 0;
    SOCKADDR_IN from;
    int from_size = sizeof( from );

    bool running = true;

    while( running ) {

        while ( Timer_ms::timer_get_ms_since_start() < milliseconds_per_tick) {
            int bytes_received = recvfrom( sock, buffer, SOCKET_BUFFER_SIZE, flags, (SOCKADDR*)&from, &from_size );
            if( bytes_received != SOCKET_ERROR )
            {
                Network::Message r_Msg;
                r_Msg.address = from;
                r_Msg.address_size = from_size;
                memcpy( &r_Msg.buffer, &buffer, SOCKET_BUFFER_SIZE );

                Network::MsgContentBase check;
                check.Read( r_Msg.buffer );

                int64 now_ms = timeSinceEpochMillisec();
                int64 ping_ms = now_ms - (int64) check.timestamp_ms;

                printf("[ From ");
                PrintAddress(r_Msg.address);
                printf(" %dms %s]\n", ping_ms, Network::SrvMsgNames[ check.message_type ]);

                switch ( (Network::ServerMessageType) check.message_type )
                {
                    case Network::ServerMessageType::RegisterSyn:
                        pCommunication->MessageRegisterSyn(r_Msg);
                        break;
                    case Network::ServerMessageType::RegisterResult:
                        pCommunication->MessageRegisterResult(r_Msg);
                        break;
                    case Network::ServerMessageType::ConnectionRequest:
                        pCommunication->MessageConnection(r_Msg);
                        break;
                    case Network::ServerMessageType::GameState:
                        pCommunication->MessageGameState(r_Msg);
                        break;
                    case Network::ServerMessageType::Kicked:
                        pCommunication->MessageKicked(r_Msg);
                        break;
                    default:
                        printf("Unhandled message type.");
                } // End switch message type

            } // End if received non-garbage

        } // End while timer not reached ms per tick

        Timer_ms::timer_start();
        
        if (pCommunication->connected != true) {
            now = timeSinceEpochMillisec();
            if (now - last_ask > interval_ms) {
                Network::Message s_Msg;
                Network::Construct::register_request(s_Msg);
                s_Msg.SetAddress(server_address);
                pCommunication->Send(s_Msg);
                last_ask = now;
            }
        }
        
        if ( pCommunication->connected ) {
            // get input
            userInput = getchar();

            input.up = (uint8) (userInput == 'w') ? 1 : 0; 
            input.down = (uint8) (userInput == 's') ? 1 : 0;
            input.left = (uint8) (userInput == 'a') ? 1 : 0;
            input.right = (uint8) (userInput == 'd') ? 1 : 0;
            input.jump = (uint8) (userInput == 'j') ? 1 : 0;
            

            Network::Message s_Msg;
            Network::Construct::player_input(s_Msg, pCommunication->id_from_server, input);
            s_Msg.SetAddress(server_address);
            pCommunication->Send(s_Msg);
        }

    }

    printf("Exiting program normally...");

    delete pCommunication;
    WSACleanup();
    return 0;
}