#pragma once
#include "pre.h"
#include "game.h"

// Graphics
#include "shader.h"
#include "graphicsfunc.h"
#include "stb_image.h"
#include <GLFW/glfw3.h>
#include "camera.h"

// GL math
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

    float32 last_mouse_x = window_coord_width / 2.0f;
    float32 last_mouse_y = window_coord_height / 2.0f;
    bool firstMouse = true;

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

    Camera camera;

    
    unsigned int texture1;
    unsigned int texture2;

    unsigned int VBOs[2];
    unsigned int VAOs[2];
    unsigned int EBOs[2];

    // world space positions of our cubes
    std::vector<glm::vec3> cubePositions;

    glm::vec3 cameraPos   = glm::vec3(0.0f, 0.0f,  3.0f);
    glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 cameraUp    = glm::vec3(0.0f, 1.0f,  0.0f);

    GraphicsHandle() {
        
        //meshCube.load_from_object_file("../obj/teapot_minusz.obj");
        
    }

    ~GraphicsHandle() {
        glDeleteVertexArrays(2, VAOs);
        glDeleteBuffers(2, VBOs);
        glDeleteBuffers(2, EBOs);
    }

    void init() {

        camera = Camera(glm::vec3(0.0f, 0.0f, 3.0f));

        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        
        // Should enable blending of two textures.
        // HASN'T WORKED SO FAR FOR SOME REASON BUT I DON'T
        // CARE. WHO NEEDS BLENDED TEXTURES ANYWAY.
        // IN FACT, WHO NEEDS ANY TEXTURES?
        // LET'S JUST GET INSPIRED FROM SUPERHOT AND ONLY
        // USE COLORS BUT IT STILL LOOKS SUPERCOOL (GET IT?)
        // FOR SOME REASON
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        ourShader = Shader("../shaders/3.3.shader.vert", "../shaders/3.3.shader.frag");

        int  success;
        char infoLog[512];

        glEnable(GL_DEPTH_TEST);  


        // set up vertex data (and buffer(s)) and configure vertex attributes
        // ------------------------------------------------------------------
        float vertices[] = {
        // vertice coords  // color coords 
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f
        };
        
        cubePositions.push_back( glm::vec3( 0.0f,  0.0f,  0.0f) );
        cubePositions.push_back( glm::vec3( 2.0f,  5.0f, -15.0f) );
        cubePositions.push_back( glm::vec3(-1.5f, -2.2f, -2.5f) );
        cubePositions.push_back( glm::vec3(-3.8f, -2.0f, -12.3f) );
        cubePositions.push_back( glm::vec3( 2.4f, -0.4f, -3.5f) );
        cubePositions.push_back( glm::vec3(-1.7f,  3.0f, -7.5f) );
        cubePositions.push_back( glm::vec3( 1.3f, -2.0f, -2.5f) );
        cubePositions.push_back( glm::vec3( 1.5f,  2.0f, -2.5f) );
        cubePositions.push_back( glm::vec3( 1.5f,  0.2f, -1.5f) );
        cubePositions.push_back( glm::vec3(-1.3f,  1.0f, -1.5f) );

        glGenBuffers( 2, VBOs );
        glGenVertexArrays( 2, VAOs );
        glGenBuffers(2, EBOs);

        // Setup for this triangle.
        glBindVertexArray( VAOs[0] );
        
        glBindBuffer( GL_ARRAY_BUFFER, VBOs[0] );
        glBufferData( GL_ARRAY_BUFFER, sizeof( vertices ), vertices, GL_STATIC_DRAW );

        // position attribute
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        // texture coord attribute
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);

        // texture 1
        // ---------
        glGenTextures(1, &texture1);
        glBindTexture(GL_TEXTURE_2D, texture1);
        // set the texture wrapping parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        // set texture filtering parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // load image, create texture and generate mipmaps
        int width, height, nrChannels;
        stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis.
        unsigned char *data = stbi_load("../textures/container.jpg", &width, &height, &nrChannels, 0);
        if (data)
        {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);
        }
        else
        {
            std::cout << "Failed to load texture" << std::endl;
        }
        stbi_image_free(data);

        // tell opengl for each sampler to which texture unit it belongs to (only has to be done once)
        // -------------------------------------------------------------------------------------------
        ourShader.use();
        ourShader.setInt("texture1", 0);
    }

    void handle_keyboard_input(float32 delta_time) {
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);

        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            camera.ProcessKeyboard(FORWARD, delta_time);
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            camera.ProcessKeyboard(BACKWARD, delta_time);
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            camera.ProcessKeyboard(LEFT, delta_time);
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            camera.ProcessKeyboard(RIGHT, delta_time);
    };

    void handle_mouse_input() {
        GLdouble x_pos_now, y_pos_now;
        glfwGetCursorPos(window, &x_pos_now, &y_pos_now);

        if ( firstMouse ) {
            last_mouse_x = x_pos_now;
            last_mouse_y = y_pos_now;
            firstMouse = false;
        }

        float32 x_offset = x_pos_now - last_mouse_x;
        float32 y_offset = last_mouse_y - y_pos_now;

        last_mouse_x = x_pos_now;
        last_mouse_y = y_pos_now;

        camera.ProcessMouseMovement( x_offset, y_offset );
    };

    // Updates graphics.
    FRESULT Update( std::vector<Player::PlayerState*>& player_states, bool8 state_got, float32 delta_time ) {

        handle_keyboard_input(delta_time);
        handle_mouse_input();

        if ( !window ) {
            return FRESULT(FR_FAILURE);
        }
        else {

            // state-setting function
            glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
            // state-using function
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            
            ourShader.use();

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE, texture1);

            // create transformations
            glm::mat4 view          = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
            glm::mat4 projection    = glm::mat4(1.0f);
            projection = glm::perspective(glm::radians(45.0f), (float32)window_coord_width / (float32)window_coord_height, 0.1f, 100.0f);
            
            view = camera.GetViewMatrix();
            // pass transformation matrices to the shader
            ourShader.setMat4f("projection", projection); // note: currently we set the projection matrix each frame, but since the projection matrix rarely changes it's often best practice to set it outside the main loop only once.
            ourShader.setMat4f("view", view);

            glBindVertexArray(VAOs[0]);

            for(unsigned int i = 0; i < 10; i++) {
                    
                // calculate the model matrix for each object and pass it to shader before drawing
                glm::mat4 model = glm::mat4(1.0f);
                model = glm::translate(model, cubePositions[i]);
                float32 angle = 20.f * i + 20.f;
                model = glm::rotate(model, (float32)glfwGetTime()* glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
                model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
                ourShader.setMat4f("model", model);

                glDrawArrays(GL_TRIANGLES, 0, 36);
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