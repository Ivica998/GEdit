#pragma once

#include <string>

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "my_events.h"
#include "dispatcher.h"


typedef unsigned int uint;
typedef unsigned char uchar;

class Shader
{
public:
    unsigned int ID; 
    ShaderEvent_E e_ShaderPreRend = ShaderEvent_E(&dpch::InvokeQ, this);
    Shader();
    Shader  &Use();
    void SetOutTex(const char* name, int value);
    void    Compile(const char *vertexSource, const char *fragmentSource, const char *geometrySource = nullptr);
    
    void    SetBool     (const char* name, bool value);
    void    SetFloat    (const char *name, float value );
    void    SetInt  (const char *name, int value );
    void    SetUInt     (const char* name, uint value);
    void    SetVec2f (const char *name, float x, float y );
    void    SetVec2f (const char *name, const glm::vec2 &value );
    void    SetVec3f (const char *name, float x, float y, float z );
    void    SetVec3f (const char *name, const glm::vec3 &value );
    void    SetVec4f (const char *name, float x, float y, float z, float w );
    void    SetVec4f (const char *name, const glm::vec4 &value );
    void    SetMax2  (const char* name, const glm::mat2& mat);
    void    SetMat3  (const char* name, const glm::mat3& mat);
    void    SetMat4  (const char *name, const glm::mat4 & mat);
private:
    void    checkCompileErrors(unsigned int object, std::string type); 
};

class IShaderSpecific {
    virtual void OnBegin(uint id) = 0;
};

class Pickshader : public Shader, public IShaderSpecific {
};

class VertShader : public Shader, public IShaderSpecific {

    
};