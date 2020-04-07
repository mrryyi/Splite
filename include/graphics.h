#pragma once
#include "pre.h"
#include "game.h"
#include "shader.h"
#include "graphicsfunc.h"
// Our fancy schmancy graphics handler
#include <GLFW/glfw3.h>

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
private:
    mesh meshCube;
    mat4x4 matProj;

    int32 framebuffer_height = 0;
    int32 framebuffer_width = 0;

    // Placeholder for camera.
    vec3d vCamera;
    vec3d vLookDir;
    float32 fYaw = 0.0;

    float32 fTheta = 0.0;

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

public:

    const float DEG2RAD = 3.14159 / 180;
    float radius = 0.25;

    float32 sc = 1.0;
    GLFWwindow* window;

    unsigned int vertexShader;
    unsigned int fragmentShader_1;
    unsigned int fragmentShader_2;
    unsigned int shaderProgram_1;
    unsigned int shaderProgram_2;

    Shader ourShader;

    unsigned int VBOs[2];
    unsigned int VAOs[2];
    unsigned int EBO;

    GraphicsHandle() {
        
        //meshCube.load_from_object_file("../obj/teapot_minusz.obj");
        
    }

    ~GraphicsHandle() {
        glDeleteVertexArrays(2, VAOs);
        glDeleteBuffers(2, VBOs);
    }

    void init() {

        ourShader = Shader("../shaders/3.3.shader.vert", "../shaders/3.3.shader.frag");

        int  success;
        char infoLog[512];

        // set up vertex data (and buffer(s)) and configure vertex attributes
        // ------------------------------------------------------------------
        float t1_vertices[] = {
            // positions         // colors
            0.5f, -0.5f, 0.0f,  1.0f, 0.0f, 0.0f,  // bottom right
            -0.5f, -0.5f, 0.0f,  0.0f, 1.0f, 0.0f,  // bottom left
            0.0f,  0.5f, 0.0f,  0.0f, 0.0f, 1.0f   // top 
        };
        
        glGenBuffers( 2, VBOs );
        glGenVertexArrays( 2, VAOs );

        // Setup for this triangle.
        //. Bind to first VAO (position attribute)
        glBindVertexArray( VAOs[0] );
        glBindBuffer( GL_ARRAY_BUFFER, VBOs[0] );
        glBufferData( GL_ARRAY_BUFFER, sizeof( t1_vertices ), t1_vertices, GL_STATIC_DRAW );
        glVertexAttribPointer(0,        // Location of the vertex position vertex attribute
                              3,        // Size of the vertex attribute (3 coordinates);
                              GL_FLOAT, // Type of data
                              GL_FALSE, // If we want the data to be normalized.
                              6 * sizeof( float32 ), // Space between consecutive vertex attribute first positions.
                              (void*) 0 // Where the position data begins in the buffer.
                              );
        glEnableVertexAttribArray( 0 );
        
        //. Bind to second VAO (color attribute)
        glVertexAttribPointer(1,        // attribute location 1
                              3,        // 3 values
                              GL_FLOAT, // value of type float
                              GL_FALSE, // not normalize
                              6 * sizeof( float32 ), // We have to stride 6 floats between beginnings of color values
                              (void*) (3*sizeof(float)) // Color attribute starts after this offset.
                              );
        glEnableVertexAttribArray( 1 );
        // glBindVertexArray( 0 ); // No need to unbind as we bind the next line.
        
        glEnableVertexAttribArray(0); 
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    }
    
    // Updates graphics.
    FRESULT Update( std::vector<Player::PlayerState*>& player_states, bool8 state_got, float32 delta_time ) {


        if ( !window ) {
            return FRESULT(FR_FAILURE);
        }
        else {

            // state-setting function
            glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
            // state-using function
            glClear(GL_COLOR_BUFFER_BIT);

            // Draw first triangle using data from the first VAO
            ourShader.use();
            glBindVertexArray( VAOs[0] );
            glDrawArrays(GL_TRIANGLES, 0, 3);

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
    size_t max_history_len = 100;
    
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

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}  

FRESULT create_window(GraphicsHandle& handle) {
    
    handle.window = glfwCreateWindow(window_coord_width, window_coord_width, "Well hello there", NULL, NULL);

    if (!handle.window) {
        glfwTerminate();
        return FRESULT(FR_FAILURE);
    }

    glfwMakeContextCurrent(handle.window);
    
    glfwSwapInterval(1);

    glfwSetFramebufferSizeCallback(handle.window, framebuffer_size_callback); 

    return FRESULT(FR_OK);
}

FRESULT init() {

    if ( !glfwInit() ) {
        return FRESULT(FR_FAILURE);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    return FRESULT(FR_OK);
    
};

FRESULT terminate() {
    glfwTerminate();
    return FRESULT(FR_OK);

};

}