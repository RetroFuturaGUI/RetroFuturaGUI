#pragma once
#include "IncludeHelper.hpp"
#include <filesystem>
#include <functional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace RetroFuturaGUI
{
    /// @brief The features a shader runs, as the bits of uDIP
    enum ShaderFeatures : u32
    {
        RoundedCorners = 1,
        GlassEffect = 1 << 1,
        GlassEffectWithImage = GlassEffect + (1 << 2),
        Raster = 1 << 3,
        FogEffect = 1 << 4,
        Wave = 1 << 5,
        BackgroundImage = 1 << 6
    };

    class Shader
    {
    public:
        /// @brief Loads, compiles and links a shader program from the given shader source files. A source may pull in other files
        ///        with #include "file", and each define is set right after its #version line, so one source can build several
        ///        programs, such as one per fill.
        Shader(const char* vertexPath, const char* fragmentPath, const char* geometryPath = nullptr, const std::vector<std::string>& defines = {});
        ~Shader();

        /// @brief Activates this shader program for subsequent draw calls.
        void UseProgram();

        /// @brief Sets a bool uniform.
        void SetUniformBool(std::string_view name, bool value) const;

        /// @brief Sets an int uniform.
        void SetUniformInt(std::string_view name, int value) const;

        /// @brief Sets a float uniform.
        void SetUniformFloat(std::string_view name, float value) const;

        /// @brief Sets a float array uniform from raw float data.
        void SetUniformFloat(std::string_view name, const f32* values, const u32 count) const;

        /// @brief Sets a vec2 uniform.
        void SetUniformVec2(std::string_view name, const glm::vec2& value) const;

        /// @brief Sets a vec2 (or vec2 array) uniform from raw float data.
        void SetUniformVec2(std::string_view name, const f32* value, const u32 count = 1) const;

        /// @brief Sets a vec2 uniform from separate x, y components.
        void SetUniformVec2(std::string_view name, float x, float y) const;

        /// @brief Sets a vec3 uniform.
        void SetUniformVec3(std::string_view name, const glm::vec3& value) const;

        /// @brief Sets a vec3 (or vec3 array) uniform from raw float data.
        void SetUniformVec3(std::string_view name, const f32* value, const u32 count = 1) const;

        /// @brief Sets a vec3 uniform from separate x, y, z components.
        void SetUniformVec3(std::string_view name, float x, float y, float z) const;

        /// @brief Sets a vec4 uniform.
        void SetUniformVec4(std::string_view name, const glm::vec4& value) const;

        /// @brief Sets a vec4 (or vec4 array) uniform from raw float data.
        void SetUniformVec4(std::string_view name, const f32* value, const u32 count = 1) const;

        /// @brief Sets a vec4 uniform from separate x, y, z, w components.
        void SetUniformVec4(std::string_view name, float x, float y, float z, float w) const;

        /// @brief Sets a mat2 uniform.
        void SetUniformMat2(std::string_view name, const glm::mat2& mat) const;

        /// @brief Sets a mat3 uniform.
        void SetUniformMat3(std::string_view name, const glm::mat3& mat) const;

        /// @brief Sets a mat4 uniform.
        void SetUniformMat4(std::string_view name, const glm::mat4& mat) const;

        /// @brief Returns the OpenGL shader program ID.
        u32 GetProgramId() const;

        /// @brief Returns the cached uniform location of the "projection" uniform.
        i32 GetProjectionLocation() const;

    private:
        /// @brief Hashes a std::string and a std::string_view alike, so finding a name builds no string.
        struct NameHash
        {
            using is_transparent = void;

            uSize operator()(std::string_view name) const noexcept
            {
                return std::hash<std::string_view>{}(name);
            }
        };

        u32 _programId = 0;
        i32 _projectionLocation = -1;
        mutable std::unordered_map<std::string, i32, NameHash, std::equal_to<>> _uniformLocations {}; //asked from GL once per name

        u32 compileShader(const char* shaderCode, const int shaderType);

        /// @brief Reads a shader source with its #include lines replaced by the files they name, and the defines set right after its #version line. Empty when a file can't be read.
        static std::string loadShaderSource(const char* shaderPath, const std::vector<std::string>& defines);

        /// @brief Appends the file's lines to source, an #include "file" line by that file's lines in turn. The file is named
        ///        relative to the one including it, and a file already included is skipped. That holds even when the first include
        ///        sat in an #if branch the defines drop, since #if is left to the GLSL compiler: a file every branch needs is
        ///        included before the #if.
        static bool appendWithIncludes(const std::filesystem::path& path, std::unordered_set<std::string>& included, std::string& source);

        /// @brief The location of the uniform with the given name. -1 when the program has none
        i32 getUniformLocation(std::string_view name) const;
    };
}