#include "..\include\pre.h"
#include "..\include\types.h"
#include "..\include\network_messages.h"
#include "..\include\network.h"
#include "..\include\timer.h"

#include "..\include\graphics.h"
#include "comm.h"

#pragma comment(lib, "Ws2_32.lib")

#define PORT_SERVER 1234

int main() {

    // Forces stdout to be line-buffered.
    setvbuf(stdout, NULL, _IONBF, 0);
    FRESULT fr;

    fr = graphics::init();
    if (fr) {
        exit(EXIT_FAILURE);
    }

    graphics::GraphicsHandle graphics_handle;

    fr = graphics::create_window(graphics_handle);
    if (fr) {
        exit(EXIT_FAILURE);
    }
    
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
        exit(EXIT_FAILURE);
    }

    int address_family = AF_INET;
    int type = SOCK_DGRAM;
    int protocol = IPPROTO_UDP;

    SOCKET sock;

    if( !Network::make_socket( &sock ) )
    {
        printf( "socket failed: %d", WSAGetLastError() );
        WSACleanup();
        exit(EXIT_FAILURE);
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
        exit(EXIT_FAILURE);
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

    constexpr int32 framerate = 144;
    constexpr float32 milliseconds_per_frame = 1000 / (float32) framerate;
    int64 last_frame_ms = timeSinceEpochMillisec();


    Timer_ms::timer_start();

    char buffer[SOCKET_BUFFER_SIZE];
    int flags = 0;
    SOCKADDR_IN from;
    int from_size = sizeof( from );

    bool running = true;

    std::vector<Player::PlayerState*> player_states;

    while( running ) {

        while ( Timer_ms::timer_get_ms_since_start() < milliseconds_per_tick) {
            int bytes_received = recvfrom( sock, buffer, SOCKET_BUFFER_SIZE, flags, (SOCKADDR*)&from, &from_size );
            if( bytes_received != SOCKET_ERROR )
            {
                Network::Message r_Msg;
                r_Msg.address = from;
                r_Msg.address_size = from_size;
                r_Msg.timestamp_received_ms = timeSinceEpochMillisec();
                memcpy( &r_Msg.buffer, &buffer, SOCKET_BUFFER_SIZE );

                // The Read() function of MsgContentBase only reads ID and timestamp.
                Network::MsgContentBase check;
                check.Read( r_Msg.buffer );

                int64 ping_ms = r_Msg.timestamp_received_ms - (int64) check.timestamp_ms;
                
                #ifdef _DEBUG_EVERY_MESSAGE
                printf("[ From ");
                PrintAddress(r_Msg.address);
                printf(" %dms", ping_ms);
                printf(" %s]\n", Network::SrvMsgNames[ check.message_type ]);
                #endif

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
                    case Network::ServerMessageType::PlayerStates:
                    {
                        player_states.clear();

                        Network::MsgContentPlayerStates msg_content;
                        msg_content.Read( r_Msg.buffer, player_states );

                        Player::print_player_states( player_states );
                    
                    }   
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
        
        if ( pCommunication->connected != true ) {
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

            Player::PlayerInput old_input = input;

            input.up = (uint8) (glfwGetKey( graphics_handle.window, GLFW_KEY_W ) == GLFW_PRESS) ? 1 : 0; 
            input.down = (uint8) (glfwGetKey( graphics_handle.window, GLFW_KEY_S ) == GLFW_PRESS) ? 1 : 0;
            input.left = (uint8) (glfwGetKey( graphics_handle.window, GLFW_KEY_A ) == GLFW_PRESS) ? 1 : 0;
            input.right = (uint8) (glfwGetKey( graphics_handle.window, GLFW_KEY_D ) == GLFW_PRESS) ? 1 : 0;
            input.jump = (uint8) (glfwGetKey( graphics_handle.window, GLFW_KEY_SPACE ) == GLFW_PRESS) ? 1 : 0;

            // If input is new, send new input.
            if ( old_input != input ) {
                Network::Message s_Msg;
                Network::Construct::player_input(s_Msg, pCommunication->id_from_server, input);

                s_Msg.SetAddress(server_address);
                pCommunication->Send(s_Msg);
            }
        }

        // Update physics
        // TODO: ADD PHYSICS

        // Update graphics
        // Should take in a gamestate object.
        now = timeSinceEpochMillisec();
        if (now - last_frame_ms >= milliseconds_per_frame) {
            graphics_handle.Update( player_states );
            last_frame_ms = now;
        }
    }

    printf("Exiting program normally...\n");

    fr = graphics::terminate();
    if (fr) {
        printf("Couldn't terminate graphics.\n");
    }

    delete pCommunication;
    WSACleanup();
    exit(EXIT_SUCCESS);
}