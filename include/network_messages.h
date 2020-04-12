#pragma once

#include "pre.h"
#include "netfunc.h"
#include "client.h"
#include "message_types.h"

namespace Network
{


uint32 client_msg_register_write( uint8* buffer) {
    uint8* iterator = buffer;

    write_uint8(&iterator, (uint8) ClientMessageType::RegisterRequest);
    
    return (uint32)(iterator - buffer);
};

uint32 client_msg_ack_write( uint8* buffer, uint32 id) {
    uint8* iterator = buffer;

    write_uint8(&iterator, (uint8) ClientMessageType::RegisterAck);
    write_uint32(&iterator, id);

    return (uint32)(iterator - buffer);
};

void client_msg_ack_read( uint8* buffer, uint32* id ) {
    uint8* iterator = buffer;

    uint8 message_type;
    read_uint8(&iterator, &message_type);
    assert(message_type == (uint8) ClientMessageType::RegisterAck);

    read_uint32(&iterator, id);
};

uint32 client_msg_leave_write( uint8* buffer, uint32 id ) {
    uint8* iterator = buffer;

    write_uint8(&iterator, (uint8) ClientMessageType::Leave);
    write_uint32(&iterator, id);

    return (uint32)(iterator - buffer);
};

void client_msg_leave_read( uint8* buffer, uint32* id ) {
    uint8* iterator = buffer;

    uint8 message_type;
    read_uint8(&iterator, &message_type);
    assert(message_type == (uint8) ClientMessageType::Leave);

    read_uint32(&iterator, id);
};

uint32 client_msg_connection_write( uint8* buffer, uint32 id ) {
    uint8* iterator = buffer;

    write_uint8(&iterator, (uint8) ClientMessageType::ConnectionResponse);

    write_uint32(&iterator, id);
    return (uint32)(iterator - buffer);
};

void client_msg_connection_read( uint8* buffer, uint32* id) {
    uint8* iterator = buffer;
    
    uint8 message_type;
    read_uint8(&iterator, &message_type);
    assert(message_type == (uint8) ClientMessageType::ConnectionResponse);

    read_uint32(&iterator, id);
};




uint32 server_msg_syn_write( uint8* buffer, uint32 id ) {
    uint8* iterator = buffer;

    write_uint8(&iterator, (uint8) ServerMessageType::RegisterSyn);
    write_uint32(&iterator, id);

    return (uint32)(iterator - buffer);
};

void server_msg_syn_read( uint8* buffer, uint32* id ) {
    uint8* iterator = buffer;

    uint8 message_type;
    read_uint8(&iterator, &message_type);
    assert(message_type == (uint8) ServerMessageType::RegisterSyn);

    read_uint32(&iterator, id);

};

uint32 server_msg_register_result_write( uint8* buffer, uint8 yes_no ) {
    uint8* iterator = buffer;

    write_uint8(&iterator, (uint8) ServerMessageType::RegisterResult);
    write_uint8(&iterator, yes_no);

    return (uint32)(iterator - buffer);
};

void server_msg_register_result_read( uint8* buffer, uint8* yes_no ) {
    uint8* iterator = buffer;

    uint8 message_type;
    read_uint8(&iterator, &message_type);
    assert(message_type == (uint8) ServerMessageType::RegisterResult);

    read_uint8(&iterator, yes_no);
};

uint32 server_msg_kicked_write( uint8* buffer ) {
    uint8* iterator = buffer;

    write_uint8(&iterator, (uint8) ServerMessageType::Kicked);

    return (uint32)(iterator - buffer);
};

uint32 server_msg_connection_write( uint8* buffer ) {
    uint8* iterator = buffer;

    write_uint8(&iterator, (uint8) ServerMessageType::ConnectionRequest);

    return (uint8)(iterator - buffer);
};





uint32 server_msg_player_states_write( uint8* buffer, std::vector<Player::PlayerState> player_states, uint64 tick ) {
    assert(player_states.size() > 0);
     
    uint8* iterator = buffer;

    write_uint8( &iterator, (uint8) ServerMessageType::PlayerStates );
    write_uint64( &iterator, tick );


    uint8 num_players = player_states.size();

    write_uint8( &iterator, num_players );

    for( auto const& player : player_states ) {
        write_player_state( &iterator, &player );
    }

    return (uint32)(iterator - buffer);
};

void server_msg_player_states_read( uint8* buffer, std::vector<Player::PlayerState*>* player_states, uint64* tick ) {
    uint8* iterator = buffer;

    uint8 message_type;
    read_uint8( &iterator, &message_type );
    assert( message_type == (uint8) ServerMessageType::PlayerStates );

    read_uint64( &iterator, tick);

    uint8 num_players;
    read_uint8( &iterator, &num_players);

    for( uint8 i = 0; i < num_players; i++) {
        Player::PlayerState* ps = new Player::PlayerState();
        read_player_state( &iterator, ps );
        player_states->push_back( ps );
    }
    
};






uint32 client_msg_input_write( uint8* buffer, uint32 id, uint64 timestamp_ms, Player::PlayerInput input ) {
    uint8* iterator = buffer;
    write_uint8(&iterator, (uint8) ClientMessageType::Input);
    write_uint32(&iterator, id);
    write_uint64(&iterator, timestamp_ms);
    write_player_input(&iterator, &input);

    return (uint32)(iterator - buffer);
};

void client_msg_input_read( uint8* buffer, uint32* id, uint64* timestamp_ms, Player::PlayerInput* input ) {
    uint8* iterator = buffer;
    uint8 message_type;
    read_uint8(&iterator, &message_type);
    assert(message_type == (uint8) ClientMessageType::Input);
    
    read_uint32(&iterator, id);
    read_uint64(&iterator, timestamp_ms);
    read_player_input(&iterator, input);

};




}