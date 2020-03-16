#pragma once

#include "pre.h"

namespace Network
{

enum ClientMessageType : uint8 {
    RegisterRequest = 0,
    RegisterAck,
    ConnectionResponse,
    Leave,
    Input
};

const char *CliMsgNames[] = {"RegisterRequest", 
                             "RegisterAck",
                             "ConnectionResponse",
                             "Leave",
                             "Input"};

enum ServerMessageType : uint8 {
    RegisterSyn = 0,
    RegisterResult,
    ConnectionRequest,
    GameState,
    Kicked
};

const char *SrvMsgNames[] = {"RegisterSyn",
                             "RegisterResult",
                             "ConnectionRequest",
                             "GameState",
                             "Kicked"};

}