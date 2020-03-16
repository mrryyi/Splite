#pragma once

#include "..\include\def.h"

#include "..\include\network.h"
#include "..\include\network_messages.h"
#include "..\include\msg_construct_server.h"

#include <map>

class Client {
public:
    int32 player_x = 0;
    int32 player_y = 0;
    int32 unique_id;
    SOCKADDR_IN address;
    
    int64 last_seen;
    int64 last_asked;

    Client(int32 unique_id, SOCKADDR_IN address, int32 player_x = 0, int32 player_y = 0)
    : unique_id(unique_id), address(address), player_x(player_x), player_y(player_y)
    {

        this->last_seen = timeSinceEpochMillisec();
    };
    ~Client(){};

    void PrintShort() {
        printf("[id:%d, address: ", unique_id);
        PrintAddress(this->address);
        printf("]");
    }
};

class Communication {
    int32 is_running = 1;

    std::map<int32, Client*> clients;

    SOCKET* pSocket;

    uint32 lastID = 0;

public:

    Communication(SOCKET* socket) {
        this->pSocket = socket;
    };
    
    uint32 NextUniqueID(){
        lastID += 1;
        return lastID;
    };
    
    uint32 AmountOfConnectedClients() {
        return clients.size();
    }

    void PrintClients() {

        printf("%d registered clients:", clients.size());
        for(auto const& cli : clients) {
            cli.second->PrintShort();
        }

    };

    void Send(Network::Message& s_Msg){

        uint8 message_type;
        memcpy( &message_type, &s_Msg.buffer[0], sizeof( message_type ) );
        
        Network::send_msg( pSocket, s_Msg );
        
#ifdef _DEBUG
        printf("[ To   ");
        PrintAddress(s_Msg.address);
        printf(" %s]\n", Network::SrvMsgNames[ message_type ]);
#endif

    };

    void RemoveClientFromList(int32 id) {

        if ( clients.count( id ) ) {
            printf("Removed client " );
            clients[id]->PrintShort();
            printf(".\n");
            clients[id]->~Client();
            clients.erase( id );
        }

    };

    void KickClient(int32 id) {
        
        if ( clients.count( id ) ) {
            Network::Message s_Msg;
            s_Msg.SetAddress( clients[id]->address );
            Network::Construct::kicked( s_Msg );
            this->Send( s_Msg );
        }

    };

    void ConnectionRequestToClient(int32 id) {

        Network::Message s_Msg;
        s_Msg.SetAddress( clients[id]->address );
        Network::Construct::connection( s_Msg, id );
        this->Send( s_Msg );

    };

    void CheckConnection() {
        
        uint64 now = timeSinceEpochMillisec();
        int64 time_since;

        for(auto const& cli : clients) {

            time_since = now - cli.second->last_seen;
            if ( time_since >= MAX_TIME_UNHEARD_FROM_MS ) {
                this->KickClient( cli.first );
                this->RemoveClientFromList( cli.first );
            }
            // Ask only if either time_since surpassed the interval to check.
            // If we've already asked within this interval, don't spam the client.
            else if ( time_since >= INTERVAL_CHECK_CLIENT_MS &&
                      (now - cli.second->last_asked) >= INTERVAL_CHECK_CLIENT_MS) {
                this->ConnectionRequestToClient( cli.first );
                cli.second->last_asked = now;
            };

        };

    };

    void ConnectionThreadCheck() {

        for(ever) {
            this->CheckConnection();
        }

    };

    void MessageConnection(Network::Message& r_Msg) {

        Network::MsgContentID msg_content;
        msg_content.Read( r_Msg.buffer );
        
        if ( clients.count( msg_content.id ) ) {
            clients[ msg_content.id ]->last_seen = timeSinceEpochMillisec();
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
        uint32 id = this->NextUniqueID();
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
            this->clients.insert(std::make_pair(msg_content.id, new_client));

            printf("Registering client: %d", msg_content.id);
            new_client->PrintShort();
            printf("\n");

        }
    }

};
