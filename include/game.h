#pragma once

#include "pre.h"
#include <vector>
#include "camera.h"
#include "glm/glm.hpp"
#include <mutex>
namespace Game
{

using namespace Player;


class Scene {
public:

    std::vector<Player::PlayerState> m_player_states;

    const std::vector<Player::PlayerState>& get_player_states() const {
        return m_player_states;
    };

    void clear_player_states() {
        m_player_states.clear();
    };

    void add_player_state( Player::PlayerState new_player ) {
        m_player_states.push_back( new_player );
    };
    void set_player_states( std::vector<Player::PlayerState> p_states ) {
        m_player_states.clear();
        m_player_states = p_states;
    };

    void remove_player_state( uint32 player_id ) {
        uint32 index_to_remove = -1;

        for( uint32 i = 0; i < m_player_states.size(); i++ ) {
            if ( m_player_states[i].id == player_id ) {
                index_to_remove = i;
                break;
            }
        }

        if (index_to_remove != -1) {
            m_player_states.erase( m_player_states.begin() + index_to_remove );
        }
    };

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
        front.y = 0.0f; //sin(glm::radians(player_state.pitch));
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

        bool8 jumping = false;

        if( player_input.jump) {
            jumping = true;
        }

        bool8 direction;

        if (glm::length(desired_movement_direction) == 0.0) {
            direction = false;
        }
        else {
            desired_movement_direction = glm::normalize(desired_movement_direction);
            direction = true;
        }

        glm::vec3 velocity;

        bool is_grounded = player_state.position.y == 0.0f;

        if ( is_grounded ) {
            if ( direction ) {
                velocity = desired_movement_direction * player_movement_speed_pms;
            }
            else {
                velocity = {0.0, 0.0, 0.0};
            }

            if ( jumping ) {
                glm::vec3 jumping_velocity = glm::vec3( 0.0f, player_jump_speed_pms, 0.0f);
                velocity += jumping_velocity;
            }
        }

        if ( !is_grounded ) {

            velocity = player_state.velocity;

        }

        player_state.velocity = velocity;

        player_state.yaw = player_input.yaw;
        player_state.pitch = player_input.pitch;

        tick_player_by_physics( player_state, delta_time_ms );

    };

};


class LocalScene : public Scene {
public:

    uint32 m_local_player_state_i = 0;

    void tick_players( Player::PlayerInput input, float32 delta_time_ms) {

        for(int i = 0; i < m_player_states.size(); i++) {

            if ( i == m_local_player_state_i ) {
                Player::tick_player_by_input( m_player_states[i], input, delta_time_ms );
            }
            else {
                Player::tick_player_by_physics( m_player_states[i], delta_time_ms );
            }

        }
    };

    void inform_camera( Camera* camera ) {
        
        camera->SetCamera( m_player_states[m_local_player_state_i].position,
                           m_player_states[m_local_player_state_i].yaw,
                           m_player_states[m_local_player_state_i].pitch );
    };
    
    void became_offline() {
        Player::PlayerState local_ps = m_player_states[m_local_player_state_i];
        clear_player_states();
        add_player_state( local_ps );
        m_local_player_state_i = 0;
    };

};


// TODO: Design how maps are designed.
class Map {
public:
    Map() {};
};

};