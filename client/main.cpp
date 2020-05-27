#include "..\include\pre.h"
#include "..\include\network_messages.h"
#include "..\include\network.h"
#include "..\include\timer.h"
#include <glad/glad.h>
#include "..\include\input.h"
#include "..\include\graphics.h"

#pragma comment(lib, "Ws2_32.lib")

#define PORT_SERVER 1234

int main( int argc, char** argv ) {


    printf("Program Started: %s\n", argv[0]); 

    std::string inet_address = "127.0.0.1";

    // Here you can connect to different IPs than the default local.
    if( argc >= 2 ) {

        if ( Network::is_valid_ip( argv[1] )) {
            inet_address = argv[1];
        }
        else {
            printf("Not valid IP: %s\n", inet_address.c_str() );
        }
    }

    // Forces stdout to be line-buffered.
    setvbuf(stdout, NULL, _IONBF, 0);
    FRESULT fr;
    Game::LocalScene mainScene;
    GLFWwindow* window;
    
    graphics::GraphicsHandle graphics_handle;

    graphics_handle.scene = &mainScene;
    graphics_handle.window = window;

    fr = graphics::init();
    if (fr) {
        printf("whattafack we failed to init graphics.\n");
        exit(EXIT_FAILURE);
    }

    fr = graphics::create_window( graphics_handle );
    if (fr) {
        printf("WHAT THE FACK WE FAILED CREATING A WINDOW.\n");
        exit(EXIT_FAILURE);
    }
        
    Camera camera = Camera(glm::vec3(0.0f, 0.0f, 3.0f));

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
    server_address.sin_addr.S_un.S_addr = inet_addr( "127.0.0.1"/*inet_address.c_str()*/ );
    
    printf("Attempting to connect to server: %s\n", inet_address.c_str() );

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

    Player::PlayerState last_known_player_state( NO_ID_GIVEN, glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 0.0, 0.0) );
    
    mainScene.add_player_state( last_known_player_state );

    uint32 local_player_state_i;
    bool8 connected = false;
    uint32 id_from_server = NO_ID_GIVEN;

    uint64 last_heard_from_server_ms = timeSinceEpochMillisec();
    uint64 time_since_heard_from_server_ms = 0;

    uint32 msg_size;

    bool8 hmt_pressed_last_tick = false;
    
    printf("Running.\nAttempting to connect...\n");

    std::thread render_thread;

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
                        msg_size = Network::client_msg_connection_write( s_Msg.buffer, id_from_server );
                        Network::send_msg( &sock, s_Msg, msg_size, r_Msg.address );

                    }
                    break;
                    case Network::ServerMessageType::PlayerStates:
                    {

                        uint64 tick;
                        mainScene.clear_player_states();
                        Network::server_msg_player_states_read( r_Msg.buffer, &mainScene.m_player_states, &tick );

                        for(int i = 0; i < mainScene.m_player_states.size(); i++) {
                            if (mainScene.m_player_states[i].id == id_from_server) {
                                // Copy
                                mainScene.m_local_player_state_i = i;
                                local_player_state_i = i;
                                break;
                            }
                        }
                    
                    }
                    break;
                    case Network::ServerMessageType::Objects:
                    {

                        uint64 tick;
                        mainScene.clear_objects();
                        Network::server_msg_objects_read( r_Msg.buffer, &mainScene.m_objects, &tick );
                        
                    }
                    break;
                    case Network::ServerMessageType::Winner:
                    {
                        uint32 ID;
                        Network::server_msg_winner_read( r_Msg.buffer, &ID );
                        if ( ID == id_from_server ) {
                            graphics_handle.won();
                            printf("I WON.");
                        }
                        else {
                            graphics_handle.lost();
                            printf("I LOST.");
                        }
                    }
                    break;
                    case Network::ServerMessageType::Kicked:
                    {

                        printf("Kicked");
                        connected = false;
                        id_from_server = NO_ID_GIVEN;
                        mainScene.became_offline();
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
        //printf("New tick.\n");

        now = timeSinceEpochMillisec();
        time_since_heard_from_server_ms = now - last_heard_from_server_ms;
        
        if ( (time_since_heard_from_server_ms > 5000 ) && connected ) {
            printf("Not connected anymore.");
            connected = false;
            id_from_server = NO_ID_GIVEN;
            mainScene.became_offline();
            local_player_state_i = 0;
        }

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
        process_input( graphics_handle, camera, input );
        
        if ( connected && id_from_server != NO_ID_GIVEN ) {

            // If input is new, send new input.
            if ( old_input != input ) {
                Network::Message s_Msg;
                msg_size = Network::client_msg_input_write( s_Msg.buffer, id_from_server, input );
                Network::send_msg( &sock, s_Msg, msg_size, server_address );
            }

        }

        mainScene.tick_players( input, local_milliseconds_per_tick );
        mainScene.inform_camera( &camera );

        graphics_handle.camera = camera;
        graphics_handle.Update( local_milliseconds_per_tick );

    }

    printf("Exiting program normally...\n");

    fr = graphics::terminate();
    if (fr) {
        printf("Couldn't terminate graphics.\n");
    }

    WSACleanup();
    exit(EXIT_SUCCESS);
}