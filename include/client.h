#pragma once

#include "pre.h"
#include <map>
#include <vector>

class Client {
public:
    uint32 unique_id;
    SOCKADDR_IN address;
    
    int64 last_seen;
    int64 last_asked;

    bool8 grounded = true;
    Player::PlayerState player_state;
    Player::PlayerInput input;

    Client(int32 unique_id, SOCKADDR_IN address, float64 player_x = 0, float64 player_y = 0)
    : unique_id(unique_id), address(address)
    {

        this->last_seen = timeSinceEpochMillisec();
        this->last_asked = this->last_seen;
        this->player_state = Player::PlayerState();
        this->player_state.id = unique_id;

    };
    ~Client(){};

    void PrintShort() {
        printf("[id:%d, address: ", unique_id);
        PrintAddress(this->address);
        printf("]");
    }
};

typedef std::map<uint32, Client*> ClientMap;