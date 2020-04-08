#pragma once
#include "pre.h"
#include "game.h"
#include "shader.h"
#include "graphicsfunc.h"
#include "stb_image.h"
// Our fancy schmancy graphics handler
#include <GLFW/glfw3.h>
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

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

    
    unsigned int texture1;

    unsigned int VBOs[2];
    unsigned int VAOs[2];
    unsigned int EBOs[2];



    GraphicsHandle() {
        
        //meshCube.load_from_object_file("../obj/teapot_minusz.obj");
        
    }

    ~GraphicsHandle() {
        glDeleteVertexArrays(2, VAOs);
        glDeleteBuffers(2, VBOs);
        glDeleteBuffers(2, EBOs);
    }

    void init() {glEnable(GL_BLEND);
glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        ourShader = Shader("../shaders/3.3.shader.vert", "../shaders/3.3.shader.frag");

        int  success;
        char infoLog[512];


        // set up vertex data (and buffer(s)) and configure vertex attributes
        // ------------------------------------------------------------------
        float vertices[] = {
            // positions          // colors           // texture coords
            0.5f,  0.5f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f,   // top right
            0.5f, -0.5f, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f,   // bottom right
            -0.5f, -0.5f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f,   // bottom left
            -0.5f,  0.5f, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f    // top left 
        };

        unsigned int indices[] = {  
            0, 1, 3, // first triangle
            1, 2, 3  // second triangle
        };
        
        glGenBuffers( 2, VBOs );
        glGenVertexArrays( 2, VAOs );
        glGenBuffers(2, EBOs);

        // Setup for this triangle.
        glBindVertexArray( VAOs[0] );
        
        glBindBuffer( GL_ARRAY_BUFFER, VBOs[0] );
        glBufferData( GL_ARRAY_BUFFER, sizeof( vertices ), vertices, GL_STATIC_DRAW );

        glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, EBOs[0] );
        glBufferData( GL_ELEMENT_ARRAY_BUFFER, sizeof( indices ), indices, GL_STATIC_DRAW);

        // position attribute
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        // color attribute
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
        // texture coord attribute
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
        glEnableVertexAttribArray(2);
        // glBindVertexArray( 0 ); // No need to unbind
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        


        // Textures
        glGenTextures(1, &texture1);
        glBindTexture(GL_TEXTURE_2D, texture1);
         // set the texture wrapping parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        // set texture filtering parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        int width, height, nr_channels;
        unsigned char *data;

        stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis.
        
        data = stbi_load("../textures/container.jpg", &width, &height, &nr_channels, 0);
        if ( data ) {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);
        }
        else {
            printf("Failed to load texture.\n");
        }
        stbi_image_free( data );

        ourShader.use();
        ourShader.setInt("texture1", 0);

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

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE, texture1);

            ourShader.use();
            glBindVertexArray(VAOs[0]);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);


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