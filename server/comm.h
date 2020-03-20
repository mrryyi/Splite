#pragma once

#include "..\include\def.h"
#include "..\include\sperror.h"
#include "..\include\network.h"
#include "..\include\network_messages.h"
#include "..\include\msg_construct_server.h"
#include "..\include\client.h"

class Communication {
    int32 is_running = 1;


    SOCKET* pSocket;

    int32 lastID = 0;

public:

    ClientMap *clients;

    Communication(SOCKET* socket, std::map<int32, Client*> *clients) {
        this->pSocket = socket;
        this->clients = clients;
    };
    
    int32 UniqueID(){
        lastID += 1;
        return lastID;
    };

    int32 GetLastID() {
        return lastID;
    };
    
    uint32 AmountOfConnectedClients() {
        return clients->size();
    }

    void PrintClients() {

        printf("%d registered clients:", clients->size());
        for(auto const& cli : *clients) {
            cli.second->PrintShort();
        }

    };

    void Send(Network::Message& s_Msg){

        uint8 message_type;
        memcpy( &message_type, &s_Msg.buffer[0], sizeof( message_type ) );
        
        Network::send_msg( pSocket, s_Msg );
        
#ifdef _DEBUG_EVERY_MESSAGE
        printf("[ To   ");
        PrintAddress(s_Msg.address);
        printf(" %s]\n", Network::SrvMsgNames[ message_type ]);
#endif

    };

    void UpdateLastSeen(int32 id, int64 time) {

        if ( clients->count( id )) {
            (*clients)[ id ]->last_seen = time;
        }

    }

    void RemoveClientFromList(int32 id) {

        if ( clients->count( id ) ) {
            printf("Removed client " );
            (*clients)[id]->PrintShort();
            printf(".\n");
            (*clients)[id]->~Client();
            (*clients).erase( id );
        }

    };

    void KickClient(int32 id) {
        
        if ( clients->count( id ) ) {
            Network::Message s_Msg;
            s_Msg.SetAddress( (*clients)[id]->address );
            Network::Construct::kicked( s_Msg );
            this->Send( s_Msg );
        }

    };

    void ConnectionRequestToClient(int32 id) {

        Network::Message s_Msg;
        s_Msg.SetAddress( (*clients)[id]->address );
        Network::Construct::connection( s_Msg, id );
        this->Send( s_Msg );

    };


    void CheckConnection() {
        
        uint64 now = timeSinceEpochMillisec();
        int64 time_since;

        std::vector<int32> clients_to_kick;

        for(auto const& cli : *clients) {

            time_since = now - cli.second->last_seen;
            if ( time_since >= MAX_TIME_UNHEARD_FROM_MS ) {
                clients_to_kick.push_back( cli.first );
            }
            // Ask only if either time_since surpassed the interval to check.
            // If we've already asked within this interval, don't spam the client.
            else if ( time_since >= INTERVAL_CHECK_CLIENT_MS &&
                      (now - cli.second->last_asked) >= INTERVAL_CHECK_CLIENT_MS) {
                this->ConnectionRequestToClient( cli.first );
                cli.second->last_asked = now;
            };

        };

        // So that we don't segfault by removing clients inside
        // the iteration above lol.
        for(int i = 0; i < clients_to_kick.size(); i++) {
            
            this->KickClient( clients_to_kick[i] );
            this->RemoveClientFromList( clients_to_kick[i] );
        };

    };

    void MessageConnection(Network::Message& r_Msg) {

        Network::MsgContentID msg_content;
        msg_content.Read( r_Msg.buffer );
        
        if ( clients->count( msg_content.id ) ) {
            (*clients)[ msg_content.id ]->last_seen = timeSinceEpochMillisec();
        }

    };

    // Request to register as client from client to server
    // We send a syn message, to get an ack message back,
    // and THEN we accept them.
    void MessageRegisterRequest(Network::Message& r_Msg) {

        Network::Message s_Msg;
        s_Msg.SetAddress(r_Msg.address);
        // This is the id we will pass back and forth.
        // This will carry over to when we receive MSGTYPE_REGISTERACK
        // and becomes the id for that client.
        uint32 id = this->UniqueID();
        Network::Construct::register_syn(s_Msg, id);
        this->Send(s_Msg);

    };

    void MessageRegisterAck(Network::Message& r_Msg) {
        // !Todo: make this acceptance test read the message
        
        bool8 accepted = false;

        if (AmountOfConnectedClients() < MAX_CLIENTS_CONNECTED){
            accepted = true;
        }

        if (accepted) {
            
            Network::MsgContentID msg_content;
            msg_content.Read(r_Msg.buffer);

            Network::Message s_Msg;
            Network::Construct::register_result( s_Msg, msg_content.id );
            s_Msg.SetAddress(r_Msg.address);
            this->Send(s_Msg);
            
            Client* new_client = new Client(msg_content.id, r_Msg.address);
            clients->insert(std::make_pair(msg_content.id, new_client));
            
            printf("Registering client: %d", msg_content.id);
            new_client->PrintShort();
            printf("\n");

        }
    };

};
