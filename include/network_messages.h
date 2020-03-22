#pragma once

#include "pre.h"
#include "netfunc.h"

namespace Network
{

class MsgContentBase {
protected:
public:

    uint8 message_type;
    uint64 timestamp_ms;

    void Write(uint8* buffer) {
        uint8* iterator = buffer;
        write_uint8(&iterator, message_type);
        write_uint64(&iterator, timestamp_ms);
    };

    void Read(uint8* buffer) {
        uint8* iterator = buffer;
        read_uint8(&iterator, &message_type);
        read_uint64(&iterator, &timestamp_ms);
    };
    
    const size_t sizeof_content() {
        return sizeof( message_type ) + sizeof( timestamp_ms );
    };
};

class MsgContentID : public MsgContentBase {
public:

    uint32 id;

    void Write(uint8* buffer) {
        uint8* iterator = buffer;
        write_uint8(&iterator, message_type);
        write_uint64(&iterator, timestamp_ms);
        write_uint32(&iterator, id);
    };

    void Read(uint8* buffer) {
        uint8* iterator = buffer;
        read_uint8(&iterator, &message_type);
        read_uint64(&iterator, &timestamp_ms);
        read_uint32(&iterator, &id);
    };
    
    const size_t sizeof_content() {
        return sizeof( message_type ) +
               sizeof( timestamp_ms ) +
               sizeof( id );
    };
};

class MsgContentInput : public MsgContentID {
public:
    Player::PlayerInput input;

    void Write(uint8* buffer) {
        uint8* iterator = buffer;
        write_uint8(&iterator, message_type);
        write_uint64(&iterator, timestamp_ms);
        write_uint32(&iterator, id);
        write_player_input_verbose(&iterator, &input);
    };

    void Read(uint8* buffer) {
        uint8* iterator = buffer;
        read_uint8(&iterator, &message_type);
        read_uint64(&iterator, &timestamp_ms);
        read_uint32(&iterator, &id);
        read_player_input_verbose(&iterator, &input);
    };
    
    const size_t sizeof_content() {
        // sizeof( bool8 ) representing the compressed player input.
        return sizeof( message_type ) +
               sizeof( timestamp_ms ) +
               sizeof( id ) + 
               Player::PlayerInput::sizeof_content();
    };
};

class MsgContentPlayerStates : public MsgContentBase {
public:
    
    std::vector<Player::PlayerState*> player_states;

    void Write( uint8* buffer) {
        uint8* iterator = buffer;
        write_uint8(&iterator, message_type);
        write_uint64(&iterator, timestamp_ms);
        uint8 amount_of_players = player_states.size();
        write_uint8(&iterator, amount_of_players);

        for(int i = 0; i < player_states.size(); i++) {
            write_int32(&iterator, player_states[i]->id);
            write_float64(&iterator, player_states[i]->x);
            write_float64(&iterator, player_states[i]->y);
            write_float32(&iterator, player_states[i]->speed_x);
            write_float32(&iterator, player_states[i]->speed_y);
        }
    }

    void Read( uint8* buffer ) {
        uint8* iterator = buffer;
        read_uint8(&iterator, &message_type);
        read_uint64(&iterator, &timestamp_ms);
        uint8 amount_of_players;
        read_uint8(&iterator, &amount_of_players);
        
        for(int i = 0; i < amount_of_players; i++) {
            Player::PlayerState* player = new Player::PlayerState();

            read_int32(&iterator, &player->id);
            read_float64(&iterator, &player->x);
            read_float64(&iterator, &player->y);
            read_float32(&iterator, &player->speed_x);
            read_float32(&iterator, &player->speed_y);
            player_states.push_back(player);
        }
    }

    const size_t sizeof_content() {
        size_t size;

        size += sizeof( message_type );
        size += sizeof( timestamp_ms);
        size += sizeof( uint8 ); // amount_of_players;
        for(int i = 0; i < player_states.size(); i++) {
            // Size of id + x + y.
            size += sizeof( int32 ) + sizeof( float64 ) + sizeof( float64 );
            size += sizeof( float32 ) + sizeof( float32 );
        }
        return size;
    }

};

// TODO: fix this and sizeof stuff because sizeof can really mess things up
// if the content size varies.
class MsgContentGameState : public MsgContentBase {};
class MsgContentKicked : public MsgContentBase {};
class MsgContentConnection : public MsgContentID {};

} // end namespace Network