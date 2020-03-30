#include "..\include\pre.h"
#include "..\include\types.h"
#include "..\include\network.h"
#include "..\include\timer.h"
#include "..\include\network_messages.h"
#include "..\include\msg_construct_server.h"
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

    Player::PlayerInput empty_player_input = Player::PlayerInput();

    char buffer[SOCKET_BUFFER_SIZE];
    int flags = 0;
    SOCKADDR_IN from;
    int from_size = sizeof( from );
    
    printf("[Server started.]\n[Listening...]\n");

    int64 now;
    int64 last_check;
    int64 ticks = 0;
    uint32 lastID = 0;

    bool8 running = true;
    Timer_ms::timer_start();

    char buffer[SOCKET_BUFFER_SIZE];
    int flags = 0;
    SOCKADDR_IN from;
    int from_size = sizeof( from );

    uint32 msg_size;

    while ( running ) {
        while( Timer_ms::timer_get_ms_since_start() < milliseconds_per_tick) {

            uint32 bytes_received;

            Network::Message r_Msg;

            bytes_received = recvfrom( sock, (char*) r_Msg.buffer, SOCKET_BUFFER_SIZE, flags, (SOCKADDR*)&r_Msg.address, &r_Msg.address_size );
            
            if ( bytes_received != SOCKET_ERROR ) {
                switch ( (Network::ClientMessageType) r_Msg.buffer[0] ){
                    case Network::ClientMessageType::RegisterRequest:
                    {

                        uint32 id_for_client = ++lastID;
                        Network::Message s_Msg;
                        msg_size = Network::server_msg_syn_write( s_Msg.buffer, id_for_client );
                        Network::send_msg( &sock, s_Msg, msg_size, r_Msg.address );

                    }
                    break;
                    case Network::ClientMessageType::RegisterAck:
                    {

                        uint8 yes_no = false;

                        if ( clients.size() < MAX_CLIENTS_CONNECTED ) {
                            yes_no = true;
                        }

                        Network::Message s_Msg;
                        msg_size = Network::server_msg_register_result_write( s_Msg.buffer, yes_no );
                        Network::send_msg( &sock, s_Msg, msg_size, r_Msg.address );

                    }
                    break;
                    default:
                    break;
                } // End switch
            } // End if bytes_received is not socket error
        } // End while tick not complete

        Timer_ms::timer_start();
        
    } // End while running
    
    printf("Exiting program normally...");
    WSACleanup();
    return 0;
}