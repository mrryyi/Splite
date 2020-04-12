#pragma once

#include "pre.h"

namespace Network
{

enum ClientMessageType : uint8 {
    RegisterRequest = 1,
    RegisterAck,
    ConnectionResponse,
    Leave,
    Input
};

const char *CliMsgNames[] = {"NoMessage"
                             "RegisterRequest", 
                             "RegisterAck",
                             "ConnectionResponse",
                             "Leave",
                             "Input"};

enum ServerMessageType : uint8 {
    RegisterSyn = 1,
    RegisterResult,
    ConnectionRequest,
    GameState,
    Kicked,
    PlayerStates
};

const char *SrvMsgNames[] = {"NoMessage",
                             "RegisterSyn",
                             "RegisterResult",
                             "ConnectionRequest",
                             "GameState",
                             "Kicked",
                             "PlayerStates"};

}