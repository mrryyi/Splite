#include "..\include\pre.h"
#include "..\include\types.h"
#include "..\include\network_messages.h"
#include "..\include\network.h"
#include "..\include\timer.h"

#include "..\include\graphics.h"

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

    int64 interval_ms = 1000;
    int64 last_ask = timeSinceEpochMillisec();
    int64 now;

    constexpr float32 local_milliseconds_per_tick = 1000 / ((float32) CLIENT_TICK_RATE);

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
    uint32 my_player_state_i;


    bool8 connected = false;
    uint32 id_from_server = NO_ID_GIVEN;

    uint64 last_heard_from_server_ms = 0;
    uint64 time_since_heard_from_server_ms = 0;

    uint32 msg_size;


    while( running ) {

        while ( Timer_ms::timer_get_ms_since_start() < local_milliseconds_per_tick) {
            int bytes_received = recvfrom( sock, buffer, SOCKET_BUFFER_SIZE, flags, (SOCKADDR*)&from, &from_size );
            if( bytes_received != SOCKET_ERROR )
            {
                last_heard_from_server_ms = timeSinceEpochMillisec();

                Network::Message r_Msg;
                r_Msg.address = from;
                r_Msg.address_size = from_size;
                
                memcpy( &r_Msg.buffer, &buffer, SOCKET_BUFFER_SIZE );

                switch ( (Network::ServerMessageType) r_Msg.buffer[0] ) {
                    case Network::ServerMessageType::RegisterSyn:
                    {
                        printf("Handshaking...\n");
                        Network::server_msg_syn_read( r_Msg.buffer, &id_from_server );
                        Network::Message s_Msg;
                        msg_size = Network::client_msg_ack_write( s_Msg.buffer, id_from_server );
                        Network::send_msg( &sock, s_Msg, msg_size, r_Msg.address );

                    }
                    break;
                    case Network::ServerMessageType::RegisterResult:
                    {
                        
                        uint8 ye_nah;
                        Network::server_msg_register_result_read( r_Msg.buffer, &ye_nah );

                        if ( ye_nah ) {
                            printf("Connected...\n");
                            connected = true;
                        }
                        else {
                            printf("Rejected :/\n");
                            id_from_server = NO_ID_GIVEN;
                            connected = false;
                        }

                    }
                    break;
                    case Network::ServerMessageType::ConnectionRequest:
                    {
                        printf("Connection upkeep.\n");
                        Network::Message s_Msg;
                        msg_size = Network::client_msg_connection_write( s_Msg.buffer );
                        Network::send_msg( &sock, s_Msg, msg_size, r_Msg.address );

                    }
                    case Network::ServerMessageType::PlayerStates:
                    {

                        player_states.clear();
                        uint64 tick;
                        Network::server_msg_player_states_read( r_Msg.buffer, &player_states, &tick );

                        printf("Server tick: %d\n", tick);
                        for(int i = 0; i < player_states.size(); i++) {
                            if (player_states[i]->id == id_from_server) {
                                // Copy
                                my_player_state_i = i;
                                break;
                            }
                        }
                    
                    }
                    break;
                    default:
                    {

                    }
                    break;
                }

            } // End if received non-garbage

        } // End while timer not reached ms per tick

        Timer_ms::timer_start();

        time_since_heard_from_server_ms = last_heard_from_server_ms - now;
        
        /*
        if ( time_since_heard_from_server_ms > 5000 ) {
            connected = false;
            id_from_server = NO_ID_GIVEN;
        }*/

        if ( connected != true ) {
            now = timeSinceEpochMillisec();

            int32 time_since = now - last_ask;
            if ( time_since > interval_ms ) {
                Network::Message s_Msg;
                msg_size = Network::client_msg_register_write( s_Msg.buffer );
                Network::send_msg( &sock, s_Msg, msg_size, server_address );
                last_ask = now;
            }
        }
        
        if ( connected ) {

            Player::PlayerInput old_input = input;

            input.up = (uint8) (glfwGetKey( graphics_handle.window, GLFW_KEY_W ) == GLFW_PRESS) ? 1 : 0; 
            input.down = (uint8) (glfwGetKey( graphics_handle.window, GLFW_KEY_S ) == GLFW_PRESS) ? 1 : 0;
            input.left = (uint8) (glfwGetKey( graphics_handle.window, GLFW_KEY_A ) == GLFW_PRESS) ? 1 : 0;
            input.right = (uint8) (glfwGetKey( graphics_handle.window, GLFW_KEY_D ) == GLFW_PRESS) ? 1 : 0;
            input.jump = (uint8) (glfwGetKey( graphics_handle.window, GLFW_KEY_SPACE ) == GLFW_PRESS) ? 1 : 0;


            // If input is new, send new input.
            if ( old_input != input ) {
                Network::Message s_Msg;
                msg_size = Network::client_msg_input_write( s_Msg.buffer, id_from_server, timeSinceEpochMillisec(), input );
                Network::send_msg( &sock, s_Msg, msg_size, server_address );
            }

            for(int i = 0; i < player_states.size(); i++) {

                if ( i == my_player_state_i ) {
                    Player::tick_player_by_input( *player_states[i], input, local_milliseconds_per_tick );
                }
                else {
                    Player::tick_player_by_physics( *player_states[i], local_milliseconds_per_tick);
                }

            }
        }


        // Update physics
        // TODO: ADD PHYSICS

        // Update graphics
        // Should take in a gamestate object.
        now = timeSinceEpochMillisec();
        if ( now - last_frame_ms >= milliseconds_per_frame ) {
            graphics_handle.Update( player_states );
            last_frame_ms = now;
        }
    }

    printf("Exiting program normally...\n");

    fr = graphics::terminate();
    if (fr) {
        printf("Couldn't terminate graphics.\n");
    }

    WSACleanup();
    exit(EXIT_SUCCESS);
}