#include "..\include\pre.h"
#include "..\include\types.h"
#include "..\include\network.h"
#include "..\include\timer.h"
#include "..\include\network_messages.h"
#include "..\include\msg_construct_server.h"
#include "..\include\client.h"

#include <map>

#pragma comment(lib, "Ws2_32.lib")


void Send(SOCKET* socket, Network::Message& s_Msg){

        uint8 message_type;
        memcpy( &message_type, &s_Msg.buffer[0], sizeof( message_type ) );
        
        Network::send_msg( socket, s_Msg );
        
#ifdef _DEBUG_EVERY_MESSAGE
        printf("[ To   ");
        PrintAddress(s_Msg.address);
        printf(" %s]\n", Network::SrvMsgNames[ message_type ]);
#endif

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

    ClientMap clients;

    Player::PlayerInput empty_player_input = Player::PlayerInput();

    char buffer[SOCKET_BUFFER_SIZE];
    int flags = 0;
    SOCKADDR_IN from;
    int from_size = sizeof( from );
    
    printf("[Server started.]\n[Listening...]\n");

    constexpr float32 milliseconds_per_tick = 1000 / SERVER_TICK_RATE;
    constexpr float32 acceleration_per_millisecond = 0.5;
    constexpr float32 gravitation_y_per_millisecond = 0.003;
    int64 now;
    int64 last_check;
    int64 ticks = 0;
    uint32 lastID = 0;

    bool8 running = true;
    Timer_ms::timer_start();

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
                r_Msg.timestamp_received_ms = timeSinceEpochMillisec();
                memcpy( &r_Msg.buffer, &buffer, SOCKET_BUFFER_SIZE );

                // The type of message may vary the length of the buffer content,
                // but MsgContentBase::ReadBuffer is smart and only reads about 
                // the members it has. It stops at msg_type and timestamp_ms
                Network::MsgContentBase check;
                check.Read(r_Msg.buffer);
                
                int64 ping_ms = r_Msg.timestamp_received_ms - (int64) check.timestamp_ms;

                #ifdef _DEBUG_EVERY_MESSAGE
                printf("[ From ");
                PrintAddress(r_Msg.address);
                printf(" %dms", ping_ms);
                printf(" %s]\n", Network::CliMsgNames[ check.message_type ]);
                #endif

                switch((Network::ClientMessageType) check.message_type )
                {
                    case Network::ClientMessageType::RegisterRequest:
                    {

                        Network::Message s_Msg;
                        s_Msg.SetAddress(r_Msg.address);
                        // This is the id we will pass back and forth.
                        // This will carry over to when we receive MSGTYPE_REGISTERACK
                        // and becomes the id for that client.
                        Network::Construct::register_syn(s_Msg, ++lastID);
                        Send( &sock, s_Msg );

                    }
                        break;
                    case Network::ClientMessageType::RegisterAck:
                    {

                        if ( clients.size() < MAX_CLIENTS_CONNECTED ) {

                            Network::MsgContentID msg_content;
                            msg_content.Read(r_Msg.buffer);

                            Network::Message s_Msg;
                            Network::Construct::register_result( s_Msg, msg_content.id );
                            s_Msg.SetAddress(r_Msg.address);
                            Send( &sock, s_Msg );
                            
                            Client* new_client = new Client( msg_content.id, r_Msg.address );
                            
                            clients.insert(std::make_pair( msg_content.id, new_client ));
                            
                            printf("Registering client: %d", msg_content.id);
                            new_client->PrintShort();
                            printf("\n");

                        }

                    }
                        break;
                    case Network::ClientMessageType::ConnectionResponse:
                    {

                        Network::MsgContentID msg_content;
                        msg_content.Read( r_Msg.buffer );
                        
                        if ( clients.count( msg_content.id ) ) {
                            clients[ msg_content.id ]->last_seen = timeSinceEpochMillisec();
                        }

                    }
                        break;
                    case Network::ClientMessageType::Leave:
                        break;
                    case Network::ClientMessageType::Input:
                    {
                        
                        int32 read_index = 0;
                        uint8 type;
                        uint64 timestamp;
                        uint32 id;

                        memcpy(&type, &r_Msg.buffer[read_index], sizeof( type ));
                        read_index += sizeof( type );
                        memcpy(&timestamp, &r_Msg.buffer[read_index], sizeof( timestamp ));
                        read_index += sizeof( timestamp );
                        memcpy(&id, &r_Msg.buffer[read_index], sizeof( id ));
                        read_index += sizeof( id );
                        
                        if ( clients.count( id ) ) {

                            Player::PlayerInput input;

                            memcpy(&input.up, &r_Msg.buffer[read_index], sizeof( input.up ));
                            read_index += sizeof( input.up );
                            memcpy(&input.down, &r_Msg.buffer[read_index], sizeof( input.down ));
                            read_index += sizeof( input.down );
                            memcpy(&input.left, &r_Msg.buffer[read_index], sizeof( input.left ));
                            read_index += sizeof( input.left );
                            memcpy(&input.right, &r_Msg.buffer[read_index], sizeof( input.right ));
                            read_index += sizeof( input.right );
                            memcpy(&input.jump, &r_Msg.buffer[read_index], sizeof( input.jump ));
                            read_index += sizeof( input.jump );
                        
                            clients[ id ]->last_seen = timeSinceEpochMillisec();
                            clients[ id ]->input = input;
                            
                        }

                    }
                        break;
                } // End switch message type.

            } // End if socket recv from.

        } // End while timer not reached ms per tick.

        // Restart timer after tick have passed.
        Timer_ms::timer_start();
        
        for( auto const& cli : clients) {

            Client* cli_p = cli.second;

            if ( cli_p->input.up ) {
                // DO nothing.
            }
            if ( cli_p->input.down ) {
                // Crouch
            }
            if ( cli_p->input.left ) {
                if ( cli_p->grounded ) {
                    cli_p->player_state->speed_x = -0.1;
                }
                else {
                    cli_p->player_state->speed_x = 0.0;
                }
            }
            if ( cli_p->input.right ) {
                if ( cli_p->grounded ) {
                    cli_p->player_state->speed_x = 0.1;
                }
                else {
                    cli_p->player_state->speed_x = 0.0;
                }
            }
            if ( cli_p->input.jump ) {
                if (cli_p->grounded) {
                    cli_p->player_state->speed_y = 0.5;
                }
            }

            cli_p->player_state->x += cli_p->player_state->speed_x * milliseconds_per_tick;
            cli_p->player_state->y += cli_p->player_state->speed_y * milliseconds_per_tick;
            
            cli.second->player_state->speed_y -= gravitation_y_per_millisecond * milliseconds_per_tick;

            if (cli_p->player_state->y <= 0) {
                cli_p->player_state->y = 0;
                cli_p->player_state->speed_y = 0;
                cli_p->grounded = 1;
            }
            else {
                cli_p->grounded = 0;
            }
            printf("[player:%d x:%f y:%f]", cli_p->unique_id, cli_p->player_state->x, cli_p->player_state->y);

            cli.second->input = empty_player_input;
        }

        // Check the connection of every client.
        now = timeSinceEpochMillisec();
        if ( (now - last_check) >= INTERVAL_CHECK_CLIENTS_MS) {

            int64 time_since;
            std::vector<uint32> clients_to_kick;

            for(auto const& cli : clients) {

                time_since = now - cli.second->last_seen;
                if ( time_since >= MAX_TIME_UNHEARD_FROM_MS ) {
                    clients_to_kick.push_back( cli.first );
                }
                // Ask only if either time_since surpassed the interval to check.
                // If we've already asked within this interval, don't spam the client.
                else if ( time_since >= INTERVAL_CHECK_CLIENT_MS &&
                        (now - cli.second->last_asked) >= INTERVAL_CHECK_CLIENT_MS) {
                    
                    Network::Message s_Msg;
                    s_Msg.SetAddress( clients[ cli.second->unique_id ]->address );
                    Network::Construct::connection( s_Msg, cli.second->unique_id );
                    Send( &sock, s_Msg );
                    cli.second->last_asked = now;
                };

            };

            uint32 cli_id;
            // So that we don't segfault by removing clients inside
            // the iteration above lol.
            for(int i = 0; i < clients_to_kick.size(); i++) {
                cli_id = clients_to_kick[i];

                if ( clients.count( cli_id ) ) {

                    // Attempting to inform player that they are kicked.
                    Network::Message s_Msg;
                    s_Msg.SetAddress( clients[ cli_id ]->address );
                    Network::Construct::kicked( s_Msg );
                    Send( &sock, s_Msg );

                    // Removing client from list.
                    printf("Removed client " );
                    clients[ cli_id ]->PrintShort();
                    printf(".\n");
                    clients[ cli_id ]->~Client();
                    clients.erase( cli_id );

                };

            };

            last_check = now;
        }; // End connection handling.

        // Broadcast player states here.

    }; // End main loop.
    
    printf("Exiting program normally...");
    WSACleanup();
    return 0;
}