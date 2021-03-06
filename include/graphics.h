#pragma once
#include "pre.h"
#include "game.h"
#include "object.h"

// Graphics
#include "shader.h"
#include "graphicsfunc.h"
#include "stb_image.h"
#include <GLFW/glfw3.h>
#include "camera.h"

// Font
//#include <ft2build.h>
//#include FT_FREETYPE_H

// GL math
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

namespace graphics
{

class GraphicsHandle {
public:
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
    Shader lightsourceShader;

    Camera camera;
    //FTLibrary ft_library;
    Game::LocalScene* scene;
    std::mutex* scene_update_mutex;
    
    unsigned int texture1;
    unsigned int texture2;

    unsigned int VBOs[2];
    unsigned int VAOs[2];
    unsigned int EBOs[2];
    unsigned int lightVAO;

    glm::vec3 lightPos = glm::vec3( 1.2f, 1.0f, -10.0f );


    GraphicsHandle() {
    }

    ~GraphicsHandle() {
        glDeleteVertexArrays(2, VAOs);
        glDeleteBuffers(2, VBOs);
        glDeleteBuffers(2, EBOs);
    }



    void init() {

        camera = Camera(glm::vec3(0.0f, 0.0f, 3.0f));

        /*auto error = TF_Init_FreeType( &ft_library );
        if (error) {
            printf("Error loading freetype.\n");
        }*/

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

        ourShader = Shader("../shaders/object_shader.vert", "../shaders/object_shader.frag");

        int  success;
        char infoLog[512];

        glEnable(GL_DEPTH_TEST);

        // set up vertex data (and buffer(s)) and configure vertex attributes
        // ------------------------------------------------------------------

        float vertices[] = {
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
         0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,

        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
         0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,

        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,

         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
         0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,

        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
         0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,

        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f
        };

        glGenBuffers( 2, VBOs );
        glGenVertexArrays( 2, VAOs );
        glGenBuffers(2, EBOs);

        // Setup for this triangle.
        glBindVertexArray( VAOs[0] );
        
        glBindBuffer( GL_ARRAY_BUFFER, VBOs[0] );
        glBufferData( GL_ARRAY_BUFFER, sizeof( vertices ), vertices, GL_STATIC_DRAW );

        // position attribute
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        // normal attribute
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
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
        ourShader.setVec3f("lightColor", 1.0f, 1.0f, 1.0f);
        ourShader.setVec3f("objectColor", 1.0f, 0.5f, 0.31f);

        // Light source

        // set up vertex data (and buffer(s)) and configure vertex attributes
        // ------------------------------------------------------------------
        float lightsource_vertices[] = {
            -0.5f, -0.5f, -0.5f, 
            0.5f, -0.5f, -0.5f,  
            0.5f,  0.5f, -0.5f,  
            0.5f,  0.5f, -0.5f,  
            -0.5f,  0.5f, -0.5f, 
            -0.5f, -0.5f, -0.5f, 

            -0.5f, -0.5f,  0.5f, 
            0.5f, -0.5f,  0.5f,  
            0.5f,  0.5f,  0.5f,  
            0.5f,  0.5f,  0.5f,  
            -0.5f,  0.5f,  0.5f, 
            -0.5f, -0.5f,  0.5f, 

            -0.5f,  0.5f,  0.5f, 
            -0.5f,  0.5f, -0.5f, 
            -0.5f, -0.5f, -0.5f, 
            -0.5f, -0.5f, -0.5f, 
            -0.5f, -0.5f,  0.5f, 
            -0.5f,  0.5f,  0.5f, 

            0.5f,  0.5f,  0.5f,  
            0.5f,  0.5f, -0.5f,  
            0.5f, -0.5f, -0.5f,  
            0.5f, -0.5f, -0.5f,  
            0.5f, -0.5f,  0.5f,  
            0.5f,  0.5f,  0.5f,  

            -0.5f, -0.5f, -0.5f, 
            0.5f, -0.5f, -0.5f,  
            0.5f, -0.5f,  0.5f,  
            0.5f, -0.5f,  0.5f,  
            -0.5f, -0.5f,  0.5f, 
            -0.5f, -0.5f, -0.5f, 

            -0.5f,  0.5f, -0.5f, 
            0.5f,  0.5f, -0.5f,  
            0.5f,  0.5f,  0.5f,  
            0.5f,  0.5f,  0.5f,  
            -0.5f,  0.5f,  0.5f, 
            -0.5f,  0.5f, -0.5f, 
        };
        glGenVertexArrays( 1, &lightVAO );
        glBindVertexArray( lightVAO );

        glBindBuffer( GL_ARRAY_BUFFER, VBOs[1] );
        glBufferData(GL_ARRAY_BUFFER, sizeof(lightsource_vertices), lightsource_vertices, GL_STATIC_DRAW);
        
        // Set the vertex attributes
        glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray( 0 );

        // Light source shader
        lightsourceShader = Shader("../shaders/lightsource.vert", "../shaders/lightsource.frag");

        printf("graphics::GraphicsHandle::init successful.\n");
    }

    // Updates graphics.
    FRESULT Update( float32 delta_time_ms ) {

        uint64 time_started_render = timeSinceEpochMillisec();
        //printf("Start render.\n");
        
        if ( !window ) {
            return FRESULT(FR_FAILURE);
        }
        else {
            
            glm::vec3 gl_clear_color = { 1.0f, 1.0f, 1.0f };

            if ( is_won_animating( delta_time_ms ) ) {
                gl_clear_color = { won_animation_time_curr / 3000,
                                   won_animation_time_curr / 2000,
                                   won_animation_time_curr / 1000 };
            };

            if ( is_lost_animating( delta_time_ms ) ) {
                gl_clear_color = { won_animation_time_curr / 3000,
                                   won_animation_time_curr / 3000,
                                   won_animation_time_curr / 3000 };
            };

            // state-setting function
            glClearColor(gl_clear_color.x, gl_clear_color.y, gl_clear_color.z, 1.0f);
            // state-using function
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            
            // Get COPY of player states as constant as we will not alter the player states.
            std::vector<Player::PlayerState> player_states;
            player_states = scene->get_player_states();
            uint32 local_i = scene->m_local_player_state_i;
            std::vector<Object> objects;
            objects = scene->get_objects();

            // create transformations
            glm::mat4 view          = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
            glm::mat4 projection    = glm::mat4(1.0f);
            projection = glm::perspective(glm::radians(90.0f), (float32)window_coord_width / (float32)window_coord_height, 0.1f, 1000.0f);
            
            view = camera.GetViewMatrix();

            ourShader.use();

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE, texture1);

            lightPos.x = 1.0f + sin(glfwGetTime()) * 2.0f;
            lightPos.y = sin(glfwGetTime() / 2.0f) * 1.0f;
            // pass transformation matrices to the shader
            ourShader.setMat4f("projection", projection); // note: currently we set the projection matrix each frame, but since the projection matrix rarely changes it's often best practice to set it outside the main loop only once.
            ourShader.setMat4f("view", view);
            ourShader.setVec3f("lightPos", lightPos);
            ourShader.setVec3f("viewPos", camera.Position);

            glBindVertexArray(VAOs[0]);

            ourShader.setVec3f("objectColor", 0.3f, 0.3f, 0.3f);

            for(unsigned int i = 0; i < objects.size(); i++) {
                    
                // calculate the model matrix for each object and pass it to shader before drawing
                glm::mat4 model = glm::mat4(1.0f);
                float32 angle = 20.f * i + 20.f;
                model = glm::translate(model, objects[i].position);
                angle *= (float32) glfwGetTime();
                
                model = glm::rotate(model, glm::radians( angle ), glm::vec3(1.0f, 0.3f, 0.5f));
                model = glm::rotate(model, glm::radians( angle ), glm::vec3(1.0f, 0.3f, 0.5f));
                ourShader.setMat4f("model", model);

                glDrawArrays(GL_TRIANGLES, 0, 36);
            }

            // also draw the lightsource object
            lightsourceShader.use();
            lightsourceShader.setMat4f("projection", projection);
            lightsourceShader.setMat4f("view", view);
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, lightPos);
            model = glm::scale(model, glm::vec3(0.2f)); // a smaller cube
            lightsourceShader.setMat4f("model", model);
            glBindVertexArray( lightVAO );
            glDrawArrays(GL_TRIANGLES, 0, 36);

            // Draw players
            glBindVertexArray(VAOs[0]);
            ourShader.use();
            ourShader.setVec3f("objectColor", 1.0f, 0.5f, 0.31f);

            for( uint32 i = 0; i < player_states.size(); i++ ) {
                if ( i != local_i ) {
                    glm::mat4 model = glm::mat4(1.0f);
                    model = glm::translate(model, player_states[i].position);
                    model = glm::rotate(model, glm::radians(player_states[i].yaw), glm::vec3(0.0f, 1.0f, 0.0f));
                    model = glm::rotate(model, glm::radians(player_states[i].pitch), glm::vec3(1.0f, 0.0f, 0.0f));
                    ourShader.setMat4f("model", model);
                    glDrawArrays(GL_TRIANGLES, 0, 36);
                }
            }
            
            glfwSwapBuffers(window);
            glfwPollEvents();
        }

        uint64 now = timeSinceEpochMillisec();
        uint64 time_took = now - time_started_render;
        
        //printf("Render time: %d ms.\n", time_took);

        return FRESULT(FR_OK);
    }

    const float32 won_animation_max_time = 3000.0f;
    float32 won_animation_time_curr = 0.0f;
    bool won_animating = false;

    void won() {
        won_animation_time_curr = 0.0f;
        won_animating = true;
    };

    bool is_won_animating( float32 delta_time_ms ) {
        if (won_animating) {
            if ( won_animation_time_curr < won_animation_max_time ) {
                won_animation_time_curr += delta_time_ms;
                return true;
            }
            else {
                won_animation_time_curr = 0.0f;
                won_animating = false;
                return false;
            }  
        }
        else {
            return false;
        }
    };

    const float32 lost_animation_max_time = 3000.0f;
    float32 lost_animation_time_curr = 0.0f;
    bool lost_animating = false;

    void lost() {
        lost_animation_time_curr = 0.0f;
        lost_animating = true;
    };

    bool is_lost_animating( float32 delta_time_ms ) {

        if (lost_animating) {
            if ( lost_animation_time_curr < lost_animation_max_time ) {
                lost_animation_time_curr += delta_time_ms;
                return true;
            }
            else {
                lost_animation_time_curr = 0.0f;
                lost_animating = false;
                return false;
            }  
        }
        else {
            return false;
        }
        
    };

};

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}  

FRESULT create_window( GraphicsHandle& handle ) {
    
    handle.window = glfwCreateWindow(window_coord_width, window_coord_width, "Well hello there", NULL, NULL);

    if ( !handle.window ) {
        glfwTerminate();
        return FRESULT(FR_FAILURE);
    }

    printf("graphics::create_window successful.\n");

    glfwMakeContextCurrent( handle.window );

    glfwSwapInterval(1);

    glfwSetFramebufferSizeCallback( handle.window, framebuffer_size_callback );

    return FRESULT(FR_OK);
}

FRESULT init() {

    if ( !glfwInit() ) {
        return FRESULT(FR_FAILURE);
    }

    printf("graphics::init successful.\n");

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