#pragma once

namespace Player
{

struct PlayerInput
{
    bool8 up, down, left, right, jump;
};

class PlayerState {
public:
    int64 id;
    int64 x;
    int64 y;
    PlayerState(int64 id, int64 x, int64 y) : id(id), x(x), y(y) {}
};

}