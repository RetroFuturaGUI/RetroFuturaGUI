#pragma once
#include "IncludeHelper.hpp"
#include <string>

namespace RetroFuturaGUI
{
    class Shader
    {
    public:
        /// @brief Loads, compiles and links a shader program from the given shader source files.
        Shader(const char* vertexPath, const char* fragmentPath, const char* geometryPath = nullptr);
        ~Shader();

        /// @brief Activates this shader program for subsequent draw calls.
        void UseProgram();

        /// @brief Sets a bool uniform.
        void SetUniformBool(const std::string& name, bool value) const;

        /// @brief Sets an int uniform.
        void SetUniformInt(const std::string& name, int value) const;

        /// @brief Sets a float uniform.
        void SetUniformFloat(const std::string& name, float value) const;

        /// @brief Sets a vec2 uniform.
        void SetUniformVec2(const std::string& name, const glm::vec2& value) const;

        /// @brief Sets a vec2 (or vec2 array) uniform from raw float data.
        void SetUniformVec2(const std::string& name, const f32* value, const u32 count = 1) const;

        /// @brief Sets a vec2 uniform from separate x, y components.
        void SetUniformVec2(const std::string& name, float x, float y) const;

        /// @brief Sets a vec3 uniform.
        void SetUniformVec3(const std::string& name, const glm::vec3& value) const;

        /// @brief Sets a vec3 (or vec3 array) uniform from raw float data.
        void SetUniformVec3(const std::string& name, const f32* value, const u32 count = 1) const;

        /// @brief Sets a vec3 uniform from separate x, y, z components.
        void SetUniformVec3(const std::string& name, float x, float y, float z) const;

        /// @brief Sets a vec4 uniform.
        void SetUniformVec4(const std::string& name, const glm::vec4& value) const;

        /// @brief Sets a vec4 (or vec4 array) uniform from raw float data.
        void SetUniformVec4(const std::string& name, const f32* value, const u32 count = 1) const;

        /// @brief Sets a vec4 uniform from separate x, y, z, w components.
        void SetUniformVec4(const std::string& name, float x, float y, float z, float w) const;

        /// @brief Sets a mat2 uniform.
        void SetUniformMat2(const std::string& name, const glm::mat2& mat) const;

        /// @brief Sets a mat3 uniform.
        void SetUniformMat3(const std::string& name, const glm::mat3& mat) const;

        /// @brief Sets a mat4 uniform.
        void SetUniformMat4(const std::string& name, const glm::mat4& mat) const;

        /// @brief Returns the OpenGL shader program ID.
        u32 GetProgramId() const;

        /// @brief Returns the cached uniform location of the "projection" uniform.
        i32 GetProjectionLocation() const;

    private:
        u32 _programId = 0;
        i32 _projectionLocation = -1;

        std::string loadShaderFile(const char* shaderPath);
        u32 compileShader(const char* shaderCode, const int shaderType);
    };
}