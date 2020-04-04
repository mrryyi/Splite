#pragma once
#include "pre.h"
#include "game.h"
// Our fancy schmancy graphics handler
#include <GLFW/glfw3.h>
#include <cmath>

namespace graphics
{

class Rect {
public:
    float32 x_start = 0.0;
    float32 y_start = 0.0;
    float32 x_end = 0.0;
    float32 y_end = 0.0;
    Rect(const float32 x1, const float32 y1, const float32 x2, const float32 y2)
    {
        x_start = x1;
        y_start = y1;
        x_end = x2;
        y_end = y2;
    };

    Rect() {};

    void scale(const float32 scale) {
        x_end *= scale;
        y_end *= scale;
    };

    void render( bool8 state_got ) {
        glBegin(GL_POLYGON);

        if ( state_got ) {
            glColor3f(1.0, 0.0, 0.0);
        }
        else {
            glColor3f(1.0, 1.0, 1.0);
        }

        glVertex3f(x_start, y_start, 0.0);
        glVertex3f(x_end, y_start, 0.0);
        glVertex3f(x_end, y_end, 0.0);
        glVertex3f(x_start, y_end, 0.0);
        glEnd();
    };
    
};


class Rect_w : public Rect {
public:
    float32 width = 1.0;
    float32 height = 1.0;

    Rect_w(){};
    Rect_w(const float32 x, const float32 y, const float32 width, const float32 height) {
        this->width = width;
        this->height = height;
        x_start = x;
        y_start = y;
        x_end = x_start + width;
        y_end = y_start + height;
    };
};

class GraphicsHandle {
public:

    const float DEG2RAD = 3.14159 / 180;
    float radius = 0.25;

    float32 sc = 1.0;
    GLFWwindow* window;

    GraphicsHandle() {

    }
    
    // Updates graphics.
    FRESULT Update(std::vector<Player::PlayerState*>& player_states, bool8 state_got) {

        if ( !window ) {
            return FRESULT(FR_FAILURE);
        }
        else {
            //Setup View
            float ratio;
            int width, height;
            int viewport_x = 0;
            int viewport_y = 0;
            glfwGetFramebufferSize(window, &width, &height);
            ratio = width / (float) height;

            // Viewport is, basically, as if we're "moving" where the result
            // of our next thing is. Imagine mapping the result to wherever we're
            // "looking".
            glViewport(viewport_x, viewport_y, width, height);
            glClear(GL_COLOR_BUFFER_BIT);
            
            if ( history_mode_on ) {
                render_players_with_history( player_states, state_got );
            }
            else {
                render_players( player_states, state_got );
            }
            
            glfwSwapBuffers(window);
            glfwPollEvents();
        }

        return FRESULT(FR_OK);
    }
    
    void history_mode_toggle() {
        history_mode_on = !history_mode_on;

        if ( !history_mode_on ) {
            // Luckily std::map destructs any comp
            player_positions.clear();
        }
    }

private:

    struct vector2f {
        float64 x, y;
    };

    struct client_history_pos : vector2f {
        bool8 state_got;
    };

    using history_pos = std::vector<client_history_pos>;

    std::map<uint32, history_pos> player_positions;

    bool8 history_mode_on = true;
    size_t max_history_len = 20;


    
    void render_player(Player::PlayerState* ps, bool8 state_got) {

        Rect_w rect = Rect_w(ps->x, ps->y, player_width, player_height);
        rect.render( state_got );

    }

    void render_players(std::vector<Player::PlayerState*>& player_states, bool8 state_got) {

        for( size_t i = 0; i < player_states.size(); i++) {
            render_player(player_states[i], state_got);
        }

    }

    void render_players_with_history(std::vector<Player::PlayerState*>& player_states, bool8 state_got) {

        for(int i = 0; i < player_states.size(); i++) {

            uint32 player_id = player_states[i]->id;

            // Add player to history map.
            if ( player_positions.count( player_id ) < 1 ) {
                history_pos hpos;
                player_positions.insert( std::pair<uint32, history_pos>(player_id, hpos) );
            }

            if ( player_positions.count( player_id )) {

                // Add new position to history
                player_positions[player_id].push_back( client_history_pos{ { player_states[i]->x, player_states[i]->y }, state_got } );

                // Delete oldest history pos.
                if ( player_positions[player_id].size() > max_history_len ) {
                    player_positions[player_id].erase( player_positions[player_id].begin() );
                }
            }

            // Draw history.
            for( auto const& pos : player_positions[ player_id ]) {
                Rect_w rect = Rect_w(pos.x, pos.y, player_width, player_height);
                rect.render( pos.state_got );
            }
        }
    }

};

FRESULT create_window(GraphicsHandle& handle) {
    
    handle.window = glfwCreateWindow(1000, 1000, "Well hello there", NULL, NULL);

    if (!handle.window) {
        glfwTerminate();
        return FRESULT(FR_FAILURE);
    }

    glfwMakeContextCurrent(handle.window);
    glClearColor(0.0, 0.0, 0.0, 0.0);           // black background
    glMatrixMode(GL_PROJECTION);                // setup viewing projection
    glLoadIdentity();                           // start with identity matrix

    // Oh my god. This is amazing. We DON'T HAVE TO SCALE STUFF MANUALLY!!!!!
    glOrtho(0.0, window_coord_height, 0.0, window_coord_width, -1.0, 1.0);   // setup a 10x10x2 viewing world
    
    glfwSwapInterval(1);

    return FRESULT(FR_OK);
}

FRESULT init() {

    if ( !glfwInit() ) {
        return FRESULT(FR_FAILURE);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    return FRESULT(FR_OK);
    
};

FRESULT terminate() {

    glfwTerminate();
    return FRESULT(FR_OK);

};

}