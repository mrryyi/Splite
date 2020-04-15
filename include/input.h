#pragma once

#include <GLFW/glfw3.h>
#include "graphics.h"
#include "player.h"

void process_input( graphics::GraphicsHandle &handle, Camera& camera, Player::PlayerInput& input ) {
    
    if (glfwGetKey( handle.window, GLFW_KEY_ESCAPE ) == GLFW_PRESS ){
        glfwSetWindowShouldClose( handle.window, true );
    }

    input.forward   = (uint8) (glfwGetKey( handle.window, GLFW_KEY_W     ) == GLFW_PRESS) ? 1 : 0; 
    input.backward  = (uint8) (glfwGetKey( handle.window, GLFW_KEY_S     ) == GLFW_PRESS) ? 1 : 0; 
    input.left      = (uint8) (glfwGetKey( handle.window, GLFW_KEY_A     ) == GLFW_PRESS) ? 1 : 0; 
    input.right     = (uint8) (glfwGetKey( handle.window, GLFW_KEY_D     ) == GLFW_PRESS) ? 1 : 0; 
    input.jump      = (uint8) (glfwGetKey( handle.window, GLFW_KEY_SPACE ) == GLFW_PRESS) ? 1 : 0;

    GLdouble x_pos_now, y_pos_now;
    glfwGetCursorPos(handle.window, &x_pos_now, &y_pos_now);

    if ( handle.firstMouse ) {
        handle.last_mouse_x = x_pos_now;
        handle.last_mouse_y = y_pos_now;
        handle.firstMouse = false;
    }

    float32 x_offset = x_pos_now - handle.last_mouse_x;
    float32 y_offset = handle.last_mouse_y - y_pos_now;

    handle.last_mouse_x = x_pos_now;
    handle.last_mouse_y = y_pos_now;

    camera.LoadMouseMovement( x_offset, y_offset );

    input.yaw = camera.Yaw;
    input.pitch = camera.Pitch;

};