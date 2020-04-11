#include "..\include\pre.h"
#include "..\include\network.h"
#include "..\include\timer.h"
#include "..\include\network_messages.h"
#include "..\include\client.h"

#include <map>

#pragma comment(lib, "Ws2_32.lib")

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

    ClientMap clients;

    char buffer[SOCKET_BUFFER_SIZE];
    int flags = 0;
    SOCKADDR_IN from;
    int from_size = sizeof( from );
    
    printf("[Server started.]\n[Listening...]\n");

    uint64 now;
    uint64 last_check;
    uint64 tick = 0;
    uint32 lastID = 0;

    bool8 running = true;
    Timer_ms::timer_start();

    uint32 msg_size;

    while ( running ) {
        while( Timer_ms::timer_get_ms_since_start() < server_milliseconds_per_tick) {

            uint32 bytes_received;

            Network::Message r_Msg;

            bytes_received = recvfrom( sock, (char*) r_Msg.buffer, SOCKET_BUFFER_SIZE, flags, (SOCKADDR*)&r_Msg.address, &r_Msg.address_size );
            
            if ( bytes_received != SOCKET_ERROR ) {

                uint8 message_type = r_Msg.buffer[0];
                
                switch ( message_type ){
                    case Network::ClientMessageType::RegisterRequest:
                    {

                        printf("Register request.\n");
                        uint32 id_for_client = ++lastID;
                        Network::Message s_Msg;
                        msg_size = Network::server_msg_syn_write( s_Msg.buffer, id_for_client );
                        Network::send_msg( &sock, s_Msg, msg_size, r_Msg.address );

                    }
                    break;
                    case Network::ClientMessageType::RegisterAck:
                    {

                        uint8 yes_no = 0;
                        uint32 id;
                        Network::client_msg_ack_read( r_Msg.buffer, &id );
                        printf("registerack");

                        if ( clients.size() < MAX_CLIENTS_CONNECTED && clients.count( id ) == 0 ) {
                            yes_no = 1;
                            clients[id] = new Client( id, r_Msg.address );
                            printf("New client registered.\n");
                        }

                        Network::Message s_Msg;
                        msg_size = Network::server_msg_register_result_write( s_Msg.buffer, yes_no );
                        Network::send_msg( &sock, s_Msg, msg_size, r_Msg.address );

                    }
                    break;
                    case Network::ClientMessageType::ConnectionResponse:
                    {
                        uint32 id;
                        Network::client_msg_connection_read( r_Msg.buffer, &id );
                        if ( clients.count( id ) > 0 ) {
                            uint64 time_now = timeSinceEpochMillisec();
                            clients[id]->last_seen = now;
                            clients[id]->last_asked = now;
                        }
                    }
                    break;
                    case Network::ClientMessageType::Input:
                    {
                        
                        uint32 id;
                        uint64 timestamp_ms;
                        Player::PlayerInput player_input;
                        Network::client_msg_input_read( r_Msg.buffer, &id, &timestamp_ms, &player_input );
                        
                        if( clients.count( id ) > 0 ) {
                            uint64 time_now = timeSinceEpochMillisec();
                            clients[id]->input = player_input;
                            clients[id]->last_seen = timeSinceEpochMillisec();
                            clients[id]->last_asked = timeSinceEpochMillisec();
                        }
                    }
                    default:
                    break;
                } // End switch
            } // End if bytes_received is not socket error
        } // End while tick not complete

        Timer_ms::timer_start();

        now = timeSinceEpochMillisec();

        if ( now - last_check >= INTERVAL_CHECK_CLIENTS_MS ) {

            std::vector<uint32> client_ids_to_kick;

            for( auto const& cli : clients ) {
                
                if ( now - cli.second->last_seen >= MAX_TIME_UNHEARD_FROM_MS) {

                    Network::Message s_Msg;
                    msg_size = Network::server_msg_kicked_write( s_Msg.buffer );
                    Network::send_msg( &sock, s_Msg, msg_size, cli.second->address );

                    client_ids_to_kick.push_back( cli.first );

                }
                else if ((now - cli.second->last_seen  >= INTERVAL_CHECK_CLIENT_MS) &&
                         (now - cli.second->last_asked >= INTERVAL_CHECK_CLIENT_MS))
                {

                    cli.second->last_asked = now;

                    Network::Message s_Msg;
                    msg_size = Network::server_msg_connection_write( s_Msg.buffer );
                    Network::send_msg( &sock, s_Msg, msg_size, cli.second->address );

                }
            }

            for( auto const& client_id : client_ids_to_kick ) {
                printf("Client kicked: %d\n", client_id);
                clients.erase( client_id );
            }

        }

        if( clients.size() > 0 )
        {

            std::vector<Player::PlayerState> player_states;

            for( auto const& cli : clients ) {
                
                // Tick player movement.
                Player::tick_player_by_input( cli.second->player_state, cli.second->input, server_milliseconds_per_tick );

                // 
                player_states.push_back(cli.second->player_state);

            };

            Network::Message s_Msg;
            msg_size = Network::server_msg_player_states_write( s_Msg.buffer, player_states, tick);

            for( auto const& cli : clients ) {
                printf("Sending playerstate to client %d\n", cli.second->unique_id);
                Network::send_msg( &sock, s_Msg, msg_size, cli.second->address );
            }

        }

        tick++;
    } // End while running
    
    printf("Exiting program normally...");
    WSACleanup();
    return 0;
}