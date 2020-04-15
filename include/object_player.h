#pragma once
#include "def.h"
#include "object.h"
#include "shader.h"

class OPlayer : public Object {
public:

    

    void setCameraView(const glm::mat4 &view) {
        m_view = view;
    };
    void setProjection(const glm::mat4 &projection) {
        m_projection = projection;
    };
    void render() {
        glUseProgram(m_ShaderProgramID);

        glUseProgram(0);
    };

    void tick() {

    };
    void setShaderProgramID(uint32 p_ShaderProgramID) {
        m_ShaderProgramID = p_ShaderProgramID;
    };

private:
    uint32 m_VAO;
    uint32 m_VBO;
    uint32 m_EBO;

    uint32 m_ShaderProgramID;

    glm::mat4 m_view;
    glm::mat4 m_projection;

};