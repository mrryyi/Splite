#pragma once

#include <vector>

namespace Player
{

struct PlayerInput
{
    bool8 up = 0;
    bool8 down = 0;
    bool8 left = 0;
    bool8 right = 0;
    bool8 jump = 0;

    void operator =(const PlayerInput &p) {
        up = p.up;
        down = p.down;
        left = p.left;
        right = p.right;
        jump = p.jump;
    };

    bool8 operator ==(const PlayerInput &p) {
        return  (up == p.up)       && 
                (down == p.down)   &&
                (left == p.left)   &&
                (right == p.right) &&
                (jump == p.jump);
    };

    bool8 operator !=(const PlayerInput &p) {
        return !(*this == p);
    };

    const static size_t sizeof_content() {
        return sizeof( uint8 ) * 5;
    };
};

class PlayerState {
public:
    int32 id;
    float64 x = 0.0;
    float64 y = 0.0;
    float32 speed_x = 0.0;
    float32 speed_y = 0.0;
    PlayerState(int32 id, float64 x, float64 y, float32 speed_x, float32 speed_y) : id(id), x(x), y(y) {}
    PlayerState() {}
};

void print_player_states(std::vector<PlayerState> player_states) {
    for(int i = 0; i < player_states.size(); i++) {
        printf("[id: %d, x:%d, y:%d]\n", player_states[i].id, player_states[i].x, player_states[i].y);
    }
}

}