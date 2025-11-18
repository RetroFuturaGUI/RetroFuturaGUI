#pragma once
#include "config.hpp"
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <string>

namespace RetroFuturaGUI
{
    class Shader
    {
    public:
        Shader(const char* vertexPath, const char* fragmentPath);
        ~Shader();

        void UseProgram();
        void SetUniformBool(const std::string& name, bool value) const;
        void SetUniformInt(const std::string& name, int value) const;
        void SetUniformFloat(const std::string& name, float value) const;
        void SetUniformVec2(const std::string& name, const glm::vec2& value) const;
        void SetUniformVec2(const std::string& name, float x, float y) const;
        void SetUniformVec3(const std::string& name, const glm::vec3& value) const;
        void SetUniformVec3(const std::string& name, float x, float y, float z) const;
        void SetUniformVec4(const std::string& name, const glm::vec4& value) const;
        void SetUniformVec4(const std::string& name, float x, float y, float z, float w) const;
        void SetUniformMat2(const std::string& name, const glm::mat2& mat) const;
        void SetUniformMat3(const std::string& name, const glm::mat3& mat) const;
        void SetUniformMat4(const std::string& name, const glm::mat4& mat) const;
        uint32_t GetProgramId() const;
        //bool LoadVertexShaderFromFile(const char* vertexPath);
        //bool LoadFragmentShaderFromFile(const char* fragmentPath);
        //bool LoadVertexShaderFromString(const char* vertexCode);
        //bool LoadFragmentShaderFromString(const char* fragmentCode);

    private:
        uint32_t _programId;

        std::string loadShaderFile(const char* shaderPath);
        void compileShader(const char* shaderCode, const int shaderType);
    };
}