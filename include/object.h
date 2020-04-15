#pragma once

#include <glm/glm.hpp>


struct Object {

    virtual void setCameraView( const glm::mat4& view ) = 0;
    virtual void setProjection( const glm::mat4& projection ) = 0;
    virtual void render() = 0;
    virtual void tick( ) = 0;

};