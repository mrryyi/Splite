#pragma once

#include "pre.h"

#include "network.h"
#include "network_messages.h"

namespace Network
{
    namespace Construct
    {
    void player_input(Message& msg, uint32 id, Player::PlayerInput input) {

        MsgContentInput msg_content;
        msg_content.message_type = ClientMessageType::Input;
        msg_content.timestamp_ms = timeSinceEpochMillisec();
        msg_content.id = id;
        msg_content.input = input;
        msg_content.Write( msg.buffer );
        msg.bufferLength = msg_content.sizeof_content();

    };

    void register_request(Message& msg) {

        MsgContentBase msg_content;
        msg_content.message_type = ClientMessageType::RegisterRequest;
        msg_content.timestamp_ms = timeSinceEpochMillisec();
        msg_content.Write( msg.buffer );
        msg.bufferLength = msg_content.sizeof_content();

    };

    void register_ack(Message& msg, int32 id) {

        MsgContentID msg_content;
        msg_content.message_type = ClientMessageType::RegisterAck;
        msg_content.timestamp_ms = timeSinceEpochMillisec();
        msg_content.id = id;
        msg_content.Write( msg.buffer );
        msg.bufferLength = msg_content.sizeof_content();

    };

    void connection(Message& msg, int32 id) {

        MsgContentID msg_content;
        msg_content.message_type = ClientMessageType::ConnectionResponse;
        msg_content.timestamp_ms = timeSinceEpochMillisec();
        msg_content.id = id;
        msg_content.Write( msg.buffer );
        msg.bufferLength = msg_content.sizeof_content();

    };

    }; // End namespace Construct
}; // End namespace Network