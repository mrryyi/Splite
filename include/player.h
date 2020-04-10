#pragma once

#include <vector>
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

namespace Player
{

void print_vec3f(const glm::vec3& vec3f) {
    printf("x:%f, y:%f, z:%f\n", vec3f.x, vec3f.y, vec3f.z);
};

struct PlayerInput
{
    bool8 forward = 0;
    bool8 backward = 0;
    bool8 up = 0;
    bool8 down = 0;
    bool8 left = 0;
    bool8 right = 0;
    bool8 jump = 0;

    float32 yaw;
    float32 pitch;

    void operator =(const PlayerInput &p) {
        forward = p.forward;
        backward = p.backward;
        up = p.up;
        down = p.down;
        left = p.left;
        right = p.right;
        jump = p.jump;

        yaw = p.yaw;
        pitch = p.pitch;
    };

    bool8 operator ==(const PlayerInput &p) {
        return  (forward == p.forward)   &&
                (backward == p.backward) &&
                (up == p.up)             && 
                (down == p.down)         &&
                (left == p.left)         &&
                (right == p.right)       &&
                (jump == p.jump)         &&
                (yaw == p.yaw)           && 
                (pitch == p.pitch);
    };

    bool8 operator !=(const PlayerInput &p) {
        return !(*this == p);
    };
};

class PlayerState {
public:
    uint32 id = NO_ID_GIVEN;
    glm::vec3 position;
    glm::vec3 velocity;
    float32 yaw = 0.0f;
    float32 pitch = 0.0f;

    PlayerState(uint32 id, glm::vec3 position, glm::vec3 velocity )
    : id(id), position(position), velocity(velocity) {


    }

    PlayerState() {
        position = glm::vec3(0.0, 0.0, 0.0);
        velocity = glm::vec3(0.0, 0.0, 0.0);
    }

    void operator =(const PlayerState &p) {
        id = p.id;
        position = p.position;
        velocity = p.velocity;
        yaw = p.yaw;
        pitch = p.pitch;
    };
};

// Per millisecond
constexpr float32 player_movement_speed_pms = 0.05;
constexpr float32 player_jump_speed_pms = 2.0;
constexpr float32 gravity_pms = 0.01;


void tick_player_by_physics( PlayerState &player_state, float32 delta_time_ms ) {

    glm::vec3 gravity = glm::vec3( 0.0f, -gravity_pms, 0.0f);
    player_state.velocity += ( gravity );

    player_state.position += player_state.velocity * delta_time_ms;

    if (player_state.position.y < 0.0) {
        player_state.position.y = 0.0;
    }
};

void tick_player_by_input( PlayerState &player_state, PlayerInput &player_input, float32 delta_time_ms ) {

    glm::vec3 front;
    front.x = cos(glm::radians(player_state.yaw)) * cos(glm::radians(player_state.pitch));
    front.y = sin(glm::radians(player_state.pitch));
    front.z = sin(glm::radians(player_state.yaw)) * cos(glm::radians(player_state.pitch));
    front = glm::normalize(front);
    // Also re-calculate the Right and Up vector

    glm::vec3 WorldUp = glm::vec3( 0.0, 1.0, 0.0 );
    glm::vec3 right = glm::normalize(glm::cross(front, WorldUp));  // Normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.

    glm::vec3 desired_movement_direction = glm::vec3(0.0f, 0.0f, 0.0f);

    if ( player_input.forward ) {
        desired_movement_direction += front;
    }
    if ( player_input.backward ) {
        desired_movement_direction -= front;
    }
    if ( player_input.left ) {
        desired_movement_direction -= right;
    }
    if ( player_input.right ) {
        desired_movement_direction += right;
    }

    bool8 no_direction;

    if (glm::length(desired_movement_direction) == 0.0) {
        no_direction = true;
    }
    else {
        desired_movement_direction = glm::normalize(desired_movement_direction);
        no_direction = false;
    }

    glm::vec3 velocity;

    bool is_grounded = player_state.position.y == 0.0f;

    if ( !no_direction ) {

        if ( is_grounded ) {
            velocity = desired_movement_direction * player_movement_speed_pms;
            if ( player_input.jump ) {
                velocity.y = player_jump_speed_pms;
            }
        }
        else {
            // If player's not grounded, we carry the previous velocity.
            velocity = player_state.velocity;
        }

    }
    else {
        if ( !is_grounded ) {
            // If player's not grounded, we carry the previous velocity.
            velocity = player_state.velocity;
        }
    }

    player_state.velocity = velocity;

    player_state.yaw = player_input.yaw;
    player_state.pitch = player_input.pitch;

    tick_player_by_physics( player_state, delta_time_ms );

};

void print_player_input(const PlayerInput& pi) {
    printf("[w:%d s:%d a:%d d:%d]\n", pi.forward, pi.backward, pi.left, pi.right );
};

void print_player_state(const PlayerState& ps) {
    printf("[id: %d, x:%f, y:%f, z:%f]\n", ps.id, ps.position.x, ps.position.y, ps.position.z);
};

void print_player_states(std::vector<PlayerState> player_states) {
    for(int i = 0; i < player_states.size(); i++) {
        print_player_state( player_states[i] );
    }
};

void print_player_states(std::vector<PlayerState*> player_states) {
    for(int i = 0; i < player_states.size(); i++) {
        print_player_state( *player_states[i] );
    }
};

}