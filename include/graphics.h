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







struct vec3d {
    float32 x, y, z;
};

struct triangle {
    vec3d p[3];
};

struct mesh {
    std::vector<triangle> tris;
};

struct mat4x4 {
    float32 m[4][4] = { 0 };
};










class GraphicsHandle {
private:
    mesh meshCube;
    mat4x4 matProj;

    int32 framebuffer_height = 0;
    int32 framebuffer_width = 0;

    float32 fTheta;
    
    void multiply_matrix_vector(vec3d &i, vec3d &o, mat4x4 &m) {
        o.x = i.x * m.m[0][0] + i.y * m.m[1][0] + i.z * m.m[2][0] + m.m[3][0];
        o.y = i.x * m.m[0][1] + i.y * m.m[1][1] + i.z * m.m[2][1] + m.m[3][1];
        o.z = i.x * m.m[0][2] + i.y * m.m[1][2] + i.z * m.m[2][2] + m.m[3][2];
        float32 w = i.x * m.m[0][3] + i.y * m.m[1][3] + i.z * m.m[2][3] + m.m[3][3];

        if (w != 0.0f) {
            o.x /= w;
            o.y /= w;
            o.z /= w;
        }
    }

    void calcMatProj() {
        float32 fNear = 0.1f;
        float32 fFar = 1000.0f;
        float32 fFov = 90.0f;

        float32 fAspectRatio = (float32)framebuffer_height / (float32)framebuffer_width;
        float32 fFovRad = 1.0f / tanf(fFov * 0.5f / 180.0f *  3.14159f);

        matProj.m[0][0] = fAspectRatio * fFovRad;
        matProj.m[1][1] = fFovRad;
        matProj.m[2][2] = fFar / (fFar - fNear);
        matProj.m[3][2] = (-fFar * fNear) / (fFar - fNear);
        matProj.m[2][3] = 1.0;
        matProj.m[3][3] = 0.0;
    }

    void draw_line( float32 x1, float32 y1 , float x2, float32 y2){
        glBegin(GL_LINES);
        glVertex2f(x1, y1);
        glVertex2f(x2, y2);
        glEnd();
    }

    void draw_triangle ( const triangle& tri ) {
        draw_line( tri.p[0].x, tri.p[0].y, tri.p[1].x, tri.p[1].y );
        draw_line( tri.p[1].x, tri.p[1].y, tri.p[2].x, tri.p[2].y );
        draw_line( tri.p[2].x, tri.p[2].y, tri.p[0].x, tri.p[0].y );
    }


public:

    const float DEG2RAD = 3.14159 / 180;
    float radius = 0.25;

    float32 sc = 1.0;
    GLFWwindow* window;

    GraphicsHandle() {
        meshCube.tris = {
            // South
            { 0.0, 0.0, 0.0,  0.0, 1.0, 0.0,  1.0, 1.0, 0.0},
            { 0.0, 0.0, 0.0,  1.0, 1.0, 0.0,  1.0, 0.0, 0.0},

            // East
            { 1.0, 0.0, 0.0,  1.0, 1.0, 0.0,  1.0, 1.0, 1.0},
            { 1.0, 0.0, 0.0,  1.0, 1.0, 1.0,  1.0, 0.0, 1.0},

            // North
            { 1.0, 0.0, 1.0,  1.0, 1.0, 1.0,  0.0, 1.0, 1.0 },
            { 1.0, 0.0, 1.0,  0.0, 1.0, 1.0,  0.0, 0.0, 1.0 },

            // West
            { 0.0, 0.0, 1.0,  0.0, 1.0, 1.0,  0.0, 1.0, 0.0 },
            { 0.0, 0.0, 1.0,  0.0, 1.0, 0.0,  0.0, 0.0, 0.0 },

            // Top
            { 0.0, 1.0, 0.0,  0.0, 1.0, 1.0,  1.0, 1.0, 1.0 },
            { 0.0, 1.0, 0.0,  1.0, 1.0, 1.0,  1.0, 1.0, 1.0 },

            // Down
            { 1.0, 0.0, 1.0,  0.0, 0.0, 1.0,  0.0, 0.0, 0.0 },
            { 1.0, 0.0, 1.0,  0.0, 0.0, 0.0,  1.0, 0.0, 0.0 }
        };

        
    }



    
    // Updates graphics.
    FRESULT Update(std::vector<Player::PlayerState*>& player_states, bool8 state_got, uint64 delta_time) {

        if ( !window ) {
            return FRESULT(FR_FAILURE);
        }
        else {
            //Setup View
            float ratio;
            int32 width, height;
            int viewport_x = 0;
            int viewport_y = 0;

            glfwGetFramebufferSize(window, &width, &height);

            if (height != framebuffer_height || width != framebuffer_width) {
                framebuffer_height = height;
                framebuffer_width = width;
                calcMatProj();
            }

            // Viewport is, basically, as if we're "moving" where the result
            // of our next thing is. Imagine mapping the result to wherever we're
            // "looking".
            glViewport(viewport_x, viewport_y, width, height);
            glClear(GL_COLOR_BUFFER_BIT);
            
            glColor3f(1.0, 1.0, 1.0);

            float32 scale = ((float32) (rand() % 1000)) / 1000;

            // Set up rotation matrices
            mat4x4 matRotZ, matRotX;
            fTheta += 0.001f * delta_time;

            // Rotation Z
            matRotZ.m[0][0] = cosf(fTheta);
            matRotZ.m[0][1] = sinf(fTheta);
            matRotZ.m[1][0] = -sinf(fTheta);
            matRotZ.m[1][1] = cosf(fTheta);
            matRotZ.m[2][2] = 1;
            matRotZ.m[3][3] = 1;

            // Rotation X
            matRotX.m[0][0] = 1;
            matRotX.m[1][1] = cosf(fTheta * 0.5f);
            matRotX.m[1][2] = sinf(fTheta * 0.5f);
            matRotX.m[2][1] = -sinf(fTheta * 0.5f);
            matRotX.m[2][2] = cosf(fTheta * 0.5f);
            matRotX.m[3][3] = 1;
            
            for( auto tri : meshCube.tris ) {

                triangle triProjected, triTranslated, triRotatedZ, triRotatedZX;

                // Rotate in Z-Axis
                multiply_matrix_vector(tri.p[0], triRotatedZ.p[0], matRotZ);
                multiply_matrix_vector(tri.p[1], triRotatedZ.p[1], matRotZ);
                multiply_matrix_vector(tri.p[2], triRotatedZ.p[2], matRotZ);

                // Rotate in X-Axis
                multiply_matrix_vector(triRotatedZ.p[0], triRotatedZX.p[0], matRotX);
                multiply_matrix_vector(triRotatedZ.p[1], triRotatedZX.p[1], matRotX);
                multiply_matrix_vector(triRotatedZ.p[2], triRotatedZX.p[2], matRotX);


                // Offset into the screen
                triTranslated = triRotatedZX;
                triTranslated.p[0].z = triRotatedZX.p[0].z + 3.0f;
                triTranslated.p[1].z = triRotatedZX.p[1].z + 3.0f;
                triTranslated.p[2].z = triRotatedZX.p[2].z + 3.0f;

                // Project triangles from 3D --> 2D
                multiply_matrix_vector(triTranslated.p[0], triProjected.p[0], matProj);
                multiply_matrix_vector(triTranslated.p[1], triProjected.p[1], matProj);
                multiply_matrix_vector(triTranslated.p[2], triProjected.p[2], matProj);

                triProjected.p[0].x += 1.0f; triProjected.p[0].y += 1.0f;
                triProjected.p[1].x += 1.0f; triProjected.p[1].y += 1.0f;
                triProjected.p[2].x += 1.0f; triProjected.p[2].y += 1.0f;

                triProjected.p[0].x *= 0.5f * (float32) width;
                triProjected.p[0].y *= 0.5f * (float32) height;
                triProjected.p[1].x *= 0.5f * (float32) width;
                triProjected.p[1].y *= 0.5f * (float32) height;
                triProjected.p[2].x *= 0.5f * (float32) width;
                triProjected.p[2].y *= 0.5f * (float32) height;

                draw_triangle(triProjected);

            }
            
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