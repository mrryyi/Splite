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
        write_player_input(&iterator, &input);
    };

    void Read(uint8* buffer) {
        uint8* iterator = buffer;
        read_uint8(&iterator, &message_type);
        read_uint64(&iterator, &timestamp_ms);
        read_uint32(&iterator, &id);
        read_player_input(&iterator, &input);
    };
    
    const size_t sizeof_content() {
        // sizeof( bool8 ) representing the compressed player input.
        return sizeof( message_type ) +
               sizeof( timestamp_ms ) +
               sizeof( id ) + 
               sizeof( bool8 );
    };
};

// TODO: fix this and sizeof stuff because sizeof can really mess things up
// if the content size varies.
class MsgContentGameState : public MsgContentBase {};
class MsgContentKicked : public MsgContentBase {};
class MsgContentConnection : public MsgContentID {};

} // end namespace Network