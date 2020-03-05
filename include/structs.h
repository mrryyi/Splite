#pragma once
#include "pre.h"

class MsgContentBase {
protected:
    int32_t read_index = 0;
    int32_t write_index = 0;
    
    virtual void WriteFurther( char* buffer ) {};
    virtual void ReadFurther( char* buffer ) {};
public:
    int32_t msg_type;
    int64_t timestamp_ms;

    MsgContentBase() {
    };

    void WriteBuffer(char* buffer) {
        write_index = 0;

        memcpy( &buffer[write_index], &msg_type, sizeof( msg_type ) );
        write_index += sizeof( msg_type );

        memcpy( &buffer[write_index], &timestamp_ms, sizeof( timestamp_ms ));
        write_index += sizeof( timestamp_ms );

        WriteFurther( buffer );
    };

    void ReadBuffer(char* buffer) {
        read_index = 0;

        memcpy( &msg_type, &buffer[read_index], sizeof( msg_type ) );
        read_index += sizeof( msg_type );

        memcpy( &timestamp_ms, &buffer[read_index], sizeof( timestamp_ms ));
        read_index += sizeof( timestamp_ms );

        this->ReadFurther( buffer );
    };

    virtual size_t sizeof_content() {
        return sizeof( msg_type ) + sizeof( timestamp_ms );
    }

};

class MsgContentID : public MsgContentBase {
protected:
    void WriteFurther( char* buffer ) {
        memcpy( &buffer[write_index], &id, sizeof( id ));
        write_index += sizeof( id );
    };

    void ReadFurther( char* buffer ) {
        memcpy( &id, &buffer[read_index], sizeof( id ) );
        read_index += sizeof( id );
    };

public:
    int32_t id;

    using MsgContentBase::MsgContentBase;

    size_t sizeof_content() {
        return sizeof( msg_type ) + sizeof( timestamp_ms ) + sizeof( id );
    }
};

//
// Register
//
class MsgContentRegisterRequest : public MsgContentBase {};
class MsgContentRegisterSyn : public MsgContentID {};
class MsgContentRegisterAck : public MsgContentID {};
class MsgContentRegisterAccept : public MsgContentID {};

//
// Connection
//
class MsgContentConnection : public MsgContentID {};

//
// Legacy
//
class MsgContentLegacyPosition : public MsgContentBase {
protected:
    void WriteFurther( char* buffer) {
        memcpy( &buffer[write_index], &x, sizeof( x ));
        write_index += sizeof( x );
        memcpy( &buffer[write_index], &y, sizeof( y ));
        write_index += sizeof( y );
    };

    void ReadFurther( char* buffer ) {
        memcpy( &x, &buffer[read_index], sizeof( x ) );
        read_index += sizeof( x );
        memcpy( &y, &buffer[read_index], sizeof( y ) );
        read_index += sizeof( y );
    };

public:
    int32_t x;
    int32_t y;

    size_t sizeof_content() {
        return sizeof( msg_type ) + sizeof( timestamp_ms ) + sizeof( x ) + sizeof( y );
    };
};

class MsgContentLegacyDirection : public MsgContentBase {
protected:
    void WriteFurther( char* buffer ) {
        memcpy( &buffer[write_index], &direction, sizeof( direction ));
        write_index += sizeof( direction );
    };

    void ReadFurther( char* buffer ) {
        memcpy( &direction, &buffer[read_index], sizeof( direction ) );
        read_index += sizeof( direction );
    };

public:
    int32_t direction;

    using MsgContentBase::MsgContentBase;

    size_t sizeof_content() {
        return sizeof( msg_type ) + sizeof( timestamp_ms ) + sizeof( direction );
    }
};