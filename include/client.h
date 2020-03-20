#pragma once

#include "pre.h"
#include <map>
#include <vector>

class Client {
public:
    int32 unique_id;
    SOCKADDR_IN address;
    
    int64 last_seen;
    int64 last_asked;

    bool8 grounded = true;
    Player::PlayerState* player_state;
    Player::PlayerInput input;

    Client(int32 unique_id, SOCKADDR_IN address, int32 player_x = 0, int32 player_y = 0)
    : unique_id(unique_id), address(address)
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

typedef std::map<int32, Client*> ClientMap;