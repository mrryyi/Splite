#include "..\include\pre.h"
#include "..\include\types.h"
#include "..\include\network.h"
#include "..\include\timer.h"

#include "comm.h"

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

    Communication* pComm = new Communication( &sock );

    char buffer[SOCKET_BUFFER_SIZE];
    int flags = 0;
    SOCKADDR_IN from;
    int from_size = sizeof( from );
    
    printf("[Server started.]\n[Listening...]\n");

    constexpr float32 milliseconds_per_tick = 1000 / SERVER_TICK_RATE;
    int64 now;
    int64 last_check;
    int64 ticks = 0;

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
#ifdef _DEBUG
                printf("[ From ");
                PrintAddress(r_Msg.address);
                printf(" %dms", ping_ms);
                printf(" %s]\n", Network::CliMsgNames[ check.message_type ]);
#endif
                switch((Network::ClientMessageType) check.message_type )
                {
                    case Network::ClientMessageType::RegisterRequest:
                        pComm->MessageRegisterRequest( r_Msg );
                        break;
                    case Network::ClientMessageType::RegisterAck:
                        pComm->MessageRegisterAck( r_Msg );
                        break;
                    case Network::ClientMessageType::ConnectionResponse:
                        pComm->MessageConnection( r_Msg );
                        break;
                    case Network::ClientMessageType::Leave:
                        break;
                    case Network::ClientMessageType::Input:
                        break;
                } // End switch message type.

            } // End if socket recv from.

        } // End while timer not reached ms per tick.

        // Restart timer after tick have passed.
        Timer_ms::timer_start();

        // Check the connection of every client.
        now = timeSinceEpochMillisec();
        if ( (now - last_check) >= INTERVAL_CHECK_CLIENTS_MS) {
            pComm->CheckConnection();
            last_check = now;
        }

    }
    
    printf("Exiting program normally...");

    delete pComm;
    WSACleanup();
    return 0;
}