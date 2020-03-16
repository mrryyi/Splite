#pragma once

#include "pre.h"

#include "network.h"
#include "network_messages.h"
#include "message_types.h"

namespace Network
{
    namespace Construct
    {
    
    void game_state(Message& msg, Game::State state )
    {
        MsgContentGameState msg_content;
        msg_content.message_type = ServerMessageType::GameState;
        msg_content.timestamp_ms = timeSinceEpochMillisec();
        msg_content.Write( msg.buffer );
        msg.bufferLength = msg_content.sizeof_content();
    }
    
    void register_syn(Message& msg, uint32 id) {

        MsgContentID msg_content;
        msg_content.message_type = ServerMessageType::RegisterSyn;
        msg_content.timestamp_ms = timeSinceEpochMillisec();
        msg_content.id = id;
        msg_content.Write( msg.buffer );
        msg.bufferLength = msg_content.sizeof_content();

    };

    void register_result(Message& msg, uint32 id) {

        MsgContentID msg_content;
        msg_content.message_type = ServerMessageType::RegisterResult;
        msg_content.timestamp_ms = timeSinceEpochMillisec();
        msg_content.id = id;
        msg_content.Write( msg.buffer );
        msg.bufferLength = msg_content.sizeof_content();

    };

    void connection(Message& msg, uint32 id) {

        MsgContentID msg_content;
        msg_content.message_type = ServerMessageType::ConnectionRequest;
        msg_content.timestamp_ms = timeSinceEpochMillisec();
        msg_content.id = id;
        msg_content.Write( msg.buffer );
        msg.bufferLength = msg_content.sizeof_content();

    };

    void kicked(Message& msg) {

        MsgContentKicked msg_content;
        msg_content.message_type = ServerMessageType::Kicked;
        msg_content.timestamp_ms = timeSinceEpochMillisec();
        msg_content.Write( msg.buffer );
        msg.bufferLength = msg_content.sizeof_content();
        
    }

    }; // End namespace Construct
}; // End namespace Network