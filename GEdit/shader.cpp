#include "shader.h"
#include <iostream>
#include "resource_manager.h"

Shader::Shader(): ID(0) { }
Shader &Shader::Use(){
    glUseProgram(this->ID);
    return *this;
}
void Shader::SetOutTex(const char* name, int value){
    glBindFragDataLocation(this->ID, value, name);
}
void Shader::Compile(const char* vertexSource, const char* fragmentSource, const char* geometrySource) {
    unsigned int sVertex, sFragment, gShader;
    sVertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(sVertex, 1, &vertexSource, NULL);
    glCompileShader(sVertex);
    checkCompileErrors(sVertex, "VERTEX");
    sFragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(sFragment, 1, &fragmentSource, NULL);
    glCompileShader(sFragment);
    checkCompileErrors(sFragment, "FRAGMENT");
    if (geometrySource != nullptr) {
        gShader = glCreateShader(GL_GEOMETRY_SHADER);
        glShaderSource(gShader, 1, &geometrySource, NULL);
        glCompileShader(gShader);
        checkCompileErrors(gShader, "GEOMETRY");
    }
    this->ID = glCreateProgram();
    glAttachShader(this->ID, sVertex);
    glAttachShader(this->ID, sFragment);
    if (geometrySource != nullptr)
        glAttachShader(this->ID, gShader);
    glLinkProgram(this->ID);
    checkCompileErrors(this->ID, "PROGRAM");
    glDeleteShader(sVertex);
    glDeleteShader(sFragment);
    if (geometrySource != nullptr)
        glDeleteShader(gShader);
}
void Shader::SetBool(const char* name, bool value){  glUniform1i(glGetUniformLocation(this->ID, name), value);}
void Shader::SetFloat(const char *name, float value){ glUniform1f(glGetUniformLocation(this->ID, name), value);}
void Shader::SetInt(const char *name, int value){glUniform1i(glGetUniformLocation(this->ID, name), value);}
void Shader::SetUInt(const char* name, uint value){glUniform1ui(glGetUniformLocation(this->ID, name), value);}
void Shader::SetVec2f(const char *name, float x, float y){glUniform2f(glGetUniformLocation(this->ID, name), x, y);}
void Shader::SetVec2f(const char *name, const glm::vec2 &value){glUniform2f(glGetUniformLocation(this->ID, name), value.x, value.y);}
void Shader::SetVec3f(const char *name, float x, float y, float z){glUniform3f(glGetUniformLocation(this->ID, name), x, y, z);}
void Shader::SetVec3f(const char *name, const glm::vec3 &value){glUniform3f(glGetUniformLocation(this->ID, name), value.x, value.y, value.z);}
void Shader::SetVec4f(const char *name, float x, float y, float z, float w){glUniform4f(glGetUniformLocation(this->ID, name), x, y, z, w);}
void Shader::SetVec4f(const char *name, const glm::vec4 &value){glUniform4f(glGetUniformLocation(this->ID, name), value.x, value.y, value.z, value.w);}
void Shader::SetMax2(const char* name, const glm::mat2& mat){glUniformMatrix2fv(glGetUniformLocation(this->ID, name), 1, false, glm::value_ptr(mat));}
void Shader::SetMat3(const char* name, const glm::mat3& mat){glUniformMatrix3fv(glGetUniformLocation(this->ID, name), 1, false, glm::value_ptr(mat));}
void Shader::SetMat4(const char* name, const glm::mat4& mat) {glUniformMatrix4fv(glGetUniformLocation(this->ID, name), 1, false, glm::value_ptr(mat));}
void Shader::checkCompileErrors(unsigned int object, std::string type) {
    int success;
    char infoLog[1024];
    if (type != "PROGRAM") {
        glGetShaderiv(object, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(object, 1024, NULL, infoLog);
            std::cout << "| ERROR::SHADER: Compile-time error: Type: " << type << "\n"
                << infoLog << "\n -- --------------------------------------------------- -- "
                << std::endl;
        }
    }
    else {
        glGetProgramiv(object, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(object, 1024, NULL, infoLog);
            std::cout << "| ERROR::Shader: Link-time error: Type: " << type << "\n"
                << infoLog << "\n -- --------------------------------------------------- -- "
                << std::endl;
        }
    }
}

