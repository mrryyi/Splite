#pragma once

#include "..\include\def.h"

#include "..\include\network.h"
#include "..\include\network_messages.h"
#include "..\include\msg_construct_client.h"

class Communication {
    SOCKET* pSocket;
public:

    Communication(SOCKET* socket) {
        pSocket = socket;
    };

    bool8 connected = false;
    int32 id_from_server = NO_ID_GIVEN;

    void Send(Network::Message& s_Msg) {

        uint8 message_type;
        memcpy( &message_type, &s_Msg.buffer[0], sizeof( message_type ) );
        Network::send_msg( pSocket, s_Msg);
        
#ifdef _DEBUG_EVERY_MESSAGE
        printf("[ To   ");
        PrintAddress(s_Msg.address);
        printf(" %s]\n", Network::CliMsgNames[ message_type ]);
#endif
        
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

        printf("[Connected]");
        
    };

    void MessageConnection(Network::Message& r_Msg) {
        
        if (id_from_server != NO_ID_GIVEN) {

            Network::Message s_Msg;
            s_Msg.SetAddress( r_Msg.address );
            Network::Construct::connection( s_Msg, id_from_server );
            this->Send( s_Msg );

        }
        else {
            printf("Received connection message without having an ID.\n");
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