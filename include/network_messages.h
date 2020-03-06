#pragma once

#include "pre.h"

namespace network
{

enum ClientMessageType : uint8 {
    RegisterRequest,
    RegisterAck,
    Connection,
    Leave,
    Input
};

enum ServerMessageType : uint8 {
    RegisterSyn,
    RegisterResult,
    GameState
};

static void write_uint8(uint8** buffer, uint8 ui8) {
    // Okay, so, we've a pointer to a uint8 array. (uint8** buffer)
    // In the implementation, we are sending an iterator acting as
    // the "reader" which is what we are increasing.

    // Set the value at reader iterator
    **buffer = ui8;
    // Increase the iterator
    ++(*buffer);
};

static void write_uint32(uint8** buffer, uint32 ui32) {
    // Same concept as write_uint8(), except we cannot do do a simple
    // assignment of the content to the spot. We need to define how to 
    // copy over the data, which memcpy does for us.
    memcpy(*buffer, &ui32, sizeof( ui32 ));
    *buffer += sizeof(ui32);
};

static void write_uint64(uint8** buffer, uint64 ui64) {
    // ditto write_uint32()
    memcpy(*buffer, &ui64, sizeof( ui64 ));
    *buffer += sizeof(ui64);
};

static void read_uint8(uint8** buffer, uint8* ui8) {
    // Set value of dereferenced pointer to value at buffer iterator.
    *ui8 = **buffer;
    ++(*buffer);
};

static void read_uint32(uint8** buffer, uint32* ui32) {
    // *buffer means the address of the value we want to start at.
    // We read bytes equal to the third argument.
    memcpy(ui32, *buffer, sizeof( ui32 ));
    *buffer += sizeof( ui32 );
};

static void read_uint64(uint8** buffer, uint64* ui64) {
    memcpy(ui64, *buffer, sizeof( ui64 ));
    *buffer += sizeof( ui64 );
};

class MsgContentBase {
public:

    uint8 message_type;
    uint64 time_ms;

    void Write(uint8* buffer) {
        uint8* iterator = buffer;
        write_uint8(&iterator, message_type);
        write_uint64(&iterator, time_ms);
    };

    void Read(uint8* buffer) {
        uint8* iterator = buffer;
        read_uint8(&iterator, &message_type);
        read_uint64(&iterator, &time_ms);
    };
    
    const size_t sizeof_content() {
        return sizeof( message_type ) + sizeof( time_ms );
    };
};

class MsgID : MsgContentBase {
public:
    uint32 id;

    void Write(uint8* buffer) {
        uint8* iterator = buffer;
        write_uint8(&iterator, message_type);
        write_uint64(&iterator, time_ms);
        write_uint32(&iterator, id);
    };

    void Read(uint8* buffer) {
        uint8* iterator = buffer;
        read_uint8(&iterator, &message_type);
        read_uint64(&iterator, &time_ms);
        read_uint32(&iterator, &id);
    };
    
    const size_t sizeof_content() {
        return sizeof( message_type ) + sizeof( time_ms );
    };
};


}