#pragma once

#include <vector>

namespace Player
{

struct PlayerInput
{
    bool8 up, down, left, right, jump;

    void operator =(const PlayerInput &p) {
        up = p.up;
        down = p.down;
        left = p.left;
        right = p.right;
        jump = p.jump;
    };
};

class PlayerState {
public:
    int32 id;
    float64 x;
    float64 y;
    float32 speed_x;
    float32 speed_y;
    PlayerState(int32 id, float64 x, float64 y, float32 speed_x, float32 speed_y) : id(id), x(x), y(y) {}
    PlayerState() {}
};

void print_player_states(std::vector<PlayerState> player_states) {
    for(int i = 0; i < player_states.size(); i++) {
        printf("[id: %d, x:%d, y:%d]\n", player_states[i].id, player_states[i].x, player_states[i].y);
    }
}

}