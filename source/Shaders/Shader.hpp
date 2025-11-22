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
        Shader(const char* vertexPath, const char* fragmentPath, const char* geometryPath = nullptr);
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
        u32 GetProgramId() const;
        i32 GetProjectionLocation() const;
        //bool LoadVertexShaderFromFile(const char* vertexPath);
        //bool LoadFragmentShaderFromFile(const char* fragmentPath);
        //bool LoadVertexShaderFromString(const char* vertexCode);
        //bool LoadFragmentShaderFromString(const char* fragmentCode);

    private:
        u32 _programId = 0;
        i32 _projectionLocation = -1;

        std::string loadShaderFile(const char* shaderPath);
        u32 compileShader(const char* shaderCode, const int shaderType);
    };
}