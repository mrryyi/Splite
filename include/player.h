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
    uint32 id;
    float64 x = 0.0;
    float64 y = 0.0;
    float32 speed_x = 0.0;
    float32 speed_y = 0.0;
    PlayerState(uint32 id, float64 x, float64 y, float32 speed_x, float32 speed_y) : id(id), x(x), y(y) {}
    PlayerState() {}

    void operator =(const PlayerState &p) {
        id = p.id;
        x = p.x;
        y = p.x;
        speed_x = p.speed_x;
        speed_y = p.speed_y;
    };

    const static size_t sizeof_content() {
        return sizeof( int32 ) + sizeof( float64 ) + sizeof( float64 ) + sizeof( float32 ) + sizeof( float32 );
    };
};

// Per millisecond
constexpr float32 player_movement_speed_pms = 0.5;
constexpr float32 player_jump_speed_pms = 2.0;
constexpr float32 gravity_pms = 0.01;


void tick_player_by_physics( PlayerState &player_state, float32 delta_time_ms ) {

    player_state.speed_y -= gravity_pms * delta_time_ms;
    
    player_state.x += player_state.speed_x * delta_time_ms;
    player_state.y += player_state.speed_y * delta_time_ms;

    if (player_state.y < 0.0) {
        player_state.y = 0.0;
    }
};

void tick_player_by_input( PlayerState &player_state, PlayerInput &player_input, float32 delta_time_ms ) {

    int8 desired_x_direction = 0;

    if ( player_input.left ) {
        desired_x_direction -= 1;
    }

    if ( player_input.right ) {
        desired_x_direction += 1;
    }

    bool8 grounded = player_state.y == 0.0;

    // Players can only change their x-direction, or jump, if they're grounded.
    if ( grounded ) {
        
        player_state.speed_x = desired_x_direction * player_movement_speed_pms;
        if ( player_input.jump ) {
            player_state.speed_y = player_jump_speed_pms;
        }
    }

    tick_player_by_physics( player_state, delta_time_ms );

};


void print_player_state(const PlayerState& ps) {
    printf("[id: %d, x:%f, y:%f]\n", ps.id, ps.x, ps.y);
};

void print_player_states(std::vector<PlayerState> player_states) {
    for(int i = 0; i < player_states.size(); i++) {
        printf("[id: %d, x:%f, y:%f]\n", player_states[i].id, player_states[i].x, player_states[i].y);
    }
};

void print_player_states(std::vector<PlayerState*> player_states) {
    for(int i = 0; i < player_states.size(); i++) {
        printf("[id: %d, x:%f, y:%f]\n", player_states[i]->id, player_states[i]->x, player_states[i]->y);
    }
};

}