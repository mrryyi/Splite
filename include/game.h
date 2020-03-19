#pragma once

#include "pre.h"
#include <vector>

namespace Game
{

using namespace Player;

class State {
public:
    State(){};
    ~State(){};
    std::vector<PlayerState> player_states;
};

// TODO: Design how maps are designed.
class Map {
public:
    Map() {};
};

};