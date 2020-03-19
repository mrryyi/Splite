#pragma once
#include "pre.h"
// Our fancy schmancy graphics handler
#include <GLFW/glfw3.h>

namespace graphics
{

FRESULT init() {
    if (!glfwInit()){
        return FRESULT(FR_FAILURE);
    }

    return FRESULT(FR_OK);
};

}