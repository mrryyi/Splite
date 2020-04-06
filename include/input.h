#pragma once

#include <GLFW/glfw3.h>

void process_input(GLFWwindow *window) {
    if (glfwGetKey( window, GLFW_KEY_ESCAPE ) == GLFW_PRESS ){
        glfwSetWindowShouldClose( window, true );
    }
};