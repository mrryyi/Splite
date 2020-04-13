#include "..\include\pre.h"
#include "..\include\network_messages.h"
#include "..\include\network.h"
#include "..\include\timer.h"
#include <glad/glad.h>
#include "..\include\input.h"
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
        printf("WHAT THE FACK WE FAILED CREATING A WINDOW.\n");
        exit(EXIT_FAILURE);
    }

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
       printf("Failed to initialize GLAD");
        return -1;
    }

    glViewport(0, 0, window_coord_width, window_coord_height);

    graphics_handle.init();
    
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

    Player::PlayerState last_known_player_state( NO_ID_GIVEN, glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 0.0, 0.0) );
    player_states.push_back(&last_known_player_state);
    uint32 local_player_state_i;
    bool8 connected = false;
    uint32 id_from_server = NO_ID_GIVEN;

    uint64 last_heard_from_server_ms = timeSinceEpochMillisec();
    uint64 time_since_heard_from_server_ms = 0;

    uint32 msg_size;

    bool8 hmt_pressed_last_tick = false;
    
    printf("Running.\nAttempting to connect...\n");

    while( !glfwWindowShouldClose( graphics_handle.window )) {
        
        bool8 state_got_this_tick = false;

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
                        
                        //if ( id_from_server == NO_ID_GIVEN ) {
                            printf("Handshaking...\n");
                            Network::server_msg_syn_read( r_Msg.buffer, &id_from_server );
                            Network::Message s_Msg;
                            msg_size = Network::client_msg_ack_write( s_Msg.buffer, id_from_server );
                            Network::send_msg( &sock, s_Msg, msg_size, r_Msg.address );
                        //else {
                        //    printf("Server tried to register syn, but ID is present.");
                        //}

                    }
                    break;
                    case Network::ServerMessageType::RegisterResult:
                    {
                        
                        uint8 ye_nah;
                        Network::server_msg_register_result_read( r_Msg.buffer, &ye_nah );
                        
                        //if ( (id_from_server == NO_ID_GIVEN) && !connected ) {
                            if ( ye_nah ) {
                                printf("Connected...\n");
                                connected = true;
                            }
                            else {
                                printf("Rejected :/\n");
                                id_from_server = NO_ID_GIVEN;
                                connected = false;
                            }
                        //}
                        //else {
                        //    printf("Server tried to register result, but ID is present.");
                        //}

                    }
                    break;
                    case Network::ServerMessageType::ConnectionRequest:
                    {
                        printf("Connection upkeep.\n");
                        Network::Message s_Msg;
                        msg_size = Network::client_msg_connection_write( s_Msg.buffer, id_from_server );
                        Network::send_msg( &sock, s_Msg, msg_size, r_Msg.address );

                    }
                    break;
                    case Network::ServerMessageType::PlayerStates:
                    {

                        state_got_this_tick = true;

                        player_states.clear();
                        uint64 tick;
                        Network::server_msg_player_states_read( r_Msg.buffer, &player_states, &tick );

                        printf("Server tick: %d\n", tick);
                        for(int i = 0; i < player_states.size(); i++) {
                            if (player_states[i]->id == id_from_server) {
                                // Copy
                                local_player_state_i = i;
                                break;
                            }
                        }
                    
                    }
                    break;
                    case Network::ServerMessageType::Kicked:
                    {

                        printf("Kicked");
                        connected = false;
                        id_from_server = NO_ID_GIVEN;
                        player_states.clear();
                        player_states.push_back(&last_known_player_state);
                        local_player_state_i = 0;

                    }
                    break;
                    default:
                    {
                        printf("Invalid message received.\n");
                    }
                    break;
                }

            } // End if received non-garbage
        } // End while timer not reached ms per tick

        Timer_ms::timer_start();

        now = timeSinceEpochMillisec();
        time_since_heard_from_server_ms = now - last_heard_from_server_ms;
        //printf("time heard from server ms: %d", time_since_heard_from_server_ms);
        
        
        if ( (time_since_heard_from_server_ms > 5000 ) && connected ) {
            printf("Not connected anymore.");
            connected = false;
            id_from_server = NO_ID_GIVEN;
            player_states.clear();
            player_states.push_back(&last_known_player_state);
            local_player_state_i = 0;
        }
        

        //printf("[Connected: { %d }]\n", connected);

        if ( connected != true ) {
            now = timeSinceEpochMillisec();

            int32 time_since = now - last_ask;
            if ( time_since > interval_ms ) {
                printf("Not connected: requesting server to register.\n");
                Network::Message s_Msg;
                msg_size = Network::client_msg_register_write( s_Msg.buffer );
                Network::send_msg( &sock, s_Msg, msg_size, server_address );
                last_ask = now;
            }
        }

        Player::PlayerInput old_input = input;

        // Now we have pitch and yaw into our input.
        process_input( graphics_handle, input );
        
        if ( connected && id_from_server != NO_ID_GIVEN ) {

            // If input is new, send new input.
            if ( old_input != input ) {
                Network::Message s_Msg;
                msg_size = Network::client_msg_input_write( s_Msg.buffer, id_from_server, timeSinceEpochMillisec(), input );
                Network::send_msg( &sock, s_Msg, msg_size, server_address );
            }

        }

        for(int i = 0; i < player_states.size(); i++) {

            if ( i == local_player_state_i ) {
                Player::tick_player_by_input( *player_states[i], input, local_milliseconds_per_tick );

                graphics_handle.camera.SetCamera( player_states[i]->position, player_states[i]->yaw, player_states[i]->pitch );
                //Player::print_player_state( *player_states[i] );
            }
            else {
                Player::tick_player_by_physics( *player_states[i], local_milliseconds_per_tick);
            }

        }
        // Update graphics
        // Should take in a gamestate object.
        now = timeSinceEpochMillisec();
        bool8 hmt_pressed_now = (bool8) glfwGetKey( graphics_handle.window, GLFW_KEY_H );
        // Toggle history mode by button release.
        if (  (!hmt_pressed_last_tick && hmt_pressed_now) ) {
            graphics_handle.history_mode_toggle();
        }
        hmt_pressed_last_tick = hmt_pressed_now;

        uint64 delta_frame_time = now - last_frame_ms;
        if ( delta_frame_time >= milliseconds_per_frame ) {
            graphics_handle.Update( player_states, local_player_state_i, state_got_this_tick, milliseconds_per_frame );
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