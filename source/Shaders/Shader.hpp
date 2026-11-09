#pragma once
#include "IncludeHelper.hpp"
#include "Projection.hpp"
#include <array>
#include <filesystem>
#include <functional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace RetroFuturaGUI
{
    /// @brief The features a shader runs as the bits of uDIP
    enum ShaderFeatures : u32
    {
        RoundedCorners = 1,
        GlassEffect = 1 << 1,
        GlassEffectWithImage = GlassEffect + (1 << 2),
        Raster = 1 << 3,
        FogEffect = 1 << 4,
        Wave = 1 << 5,
        BackgroundImage = 1 << 6,
        Mask = 1 << 7,
        Pixelate = 1 << 8
    };

    class Shader
    {
    public:
        /// @brief While one lives, everything drawn is that much more see-through: its opacity multiplies the one around it, and
        ///        every shader program in use takes the product as uOpacity. A widget draws inside one at its opacity.
        class OpacityScope
        {
        public:
            OpacityScope(const f32 opacity);
            OpacityScope(const OpacityScope&) = delete;
            OpacityScope(OpacityScope&&) = delete;
            auto operator =(const OpacityScope&) = delete;
            auto operator =(OpacityScope&&) = delete;
            ~OpacityScope();

        private:
            f32 _outerOpacity { 1.0f };
        };

        /// @brief How a FrameScope's matrix meets the frame around it
        enum class FrameComposition : u32
        {
            Inside,     // multiplies the frame around it: A widget inside its container
            Instead     // replaces it: what draws later apart from its widget in the frame the widget had
        };

        /// @brief While one lives, everything drawn is placed through its frame. The matrix that tilts an untilted layout into
        ///        place. Every shader program in use takes the frame as uFrame. A widget draws and takes the pointer inside one
        ///        that tilts it around its center inside its container's
        class FrameScope
        {
        public:
            FrameScope(const glm::mat4& frame, const FrameComposition composition);
            FrameScope(const FrameScope&) = delete;
            FrameScope(FrameScope&&) = delete;
            auto operator =(const FrameScope&) = delete;
            auto operator =(FrameScope&&) = delete;
            ~FrameScope();

        private:
            glm::mat4 _outerFrame { 1.0f };
        };

        /// @brief The frame of the FrameScopes around what is drawn now or the identity outside every one
        static const glm::mat4& GetDrawFrame();

        /// @brief While one lives, everything drawn is cut to a rectangle of the untilted layout of the frame around it (just as like as the
        ///        eye sees it). Additionally also to the ClipScopes around it. That frame however is tilted. Every shader program in use takes the
        ///        cut as uClipPlanes: four planes from the eye through the rectangle's edges which under perspective also cut what
        ///        lies in front of or behind the rectangle where it reaches past it on the screen. Nested rectangles in one frame
        ///        and depth are cut to their overlap. Any other adds its own four planes, two deep at most (the innermost two count)
        class ClipScope
        {
        public:
            /// @brief Cuts to box: left, bottom, width and height in the untilted layout, lying at depth there, as projection's eye sees it
            ClipScope(const glm::vec4& box, const f32 depth, const Projection& projection);
            ClipScope(const ClipScope&) = delete;
            ClipScope(ClipScope&&) = delete;
            auto operator =(const ClipScope&) = delete;
            auto operator =(ClipScope&&) = delete;
            ~ClipScope();

        private:
            std::array<glm::vec4, 8> _outerPlanes {};
            u32 _outerPlaneCount { 0 };
            glm::vec4 _outerBox { 0.0f };
            glm::mat4
                _outerBoxFrame { 1.0f },
                _outerBoxView { 1.0f };
            f32 _outerBoxDepth { 0.0f };
        };

        /// @brief While one lives, the ClipScopes around it cut nothing. For what draws in a space of its own such as a tabbed
        ///        window's content drawn by its host
        class UnclippedScope
        {
        public:
            UnclippedScope();
            UnclippedScope(const UnclippedScope&) = delete;
            UnclippedScope(UnclippedScope&&) = delete;
            auto operator =(const UnclippedScope&) = delete;
            auto operator =(UnclippedScope&&) = delete;
            ~UnclippedScope();

        private:
            u32 _outerPlaneCount { 0 };
        };

        /// @brief Loads, compiles and links a shader program from the given shader source files. A source may pull in other files
        ///        with #include "file", and each define is set right after its #version line, so one source can build several
        ///        programs, such as one per fill.
        Shader(const char* vertexPath, const char* fragmentPath, const char* geometryPath = nullptr, const std::vector<std::string>& defines = {});
        ~Shader();

        /// @brief Activates this shader program for subsequent draw calls with the opacity of the OpacityScopes and the frame of
        ///        the FrameScopes around it.
        void UseProgram();

        /// @brief Sets a bool uniform.
        void SetUniformBool(std::string_view name, bool value) const;

        /// @brief Sets an int32_t uniform.
        void SetUniformInt(std::string_view name, int32_t value) const;

        /// @brief Sets a float uniform.
        void SetUniformFloat(std::string_view name, f32 value) const;

        /// @brief Sets a float array uniform from raw float data.
        void SetUniformFloat(std::string_view name, const f32* values, const u32 count) const;

        /// @brief Sets a vec2 uniform.
        void SetUniformVec2(std::string_view name, const glm::vec2& value) const;

        /// @brief Sets a vec2 (or vec2 array) uniform from raw float data.
        void SetUniformVec2(std::string_view name, const f32* value, const u32 count = 1) const;

        /// @brief Sets a vec2 uniform from separate x, y components.
        void SetUniformVec2(std::string_view name, f32 x, f32 y) const;

        /// @brief Sets a vec3 uniform.
        void SetUniformVec3(std::string_view name, const glm::vec3& value) const;

        /// @brief Sets a vec3 (or vec3 array) uniform from raw float data.
        void SetUniformVec3(std::string_view name, const f32* value, const u32 count = 1) const;

        /// @brief Sets a vec3 uniform from separate x, y, z components.
        void SetUniformVec3(std::string_view name, f32 x, f32 y, f32 z) const;

        /// @brief Sets a vec4 uniform.
        void SetUniformVec4(std::string_view name, const glm::vec4& value) const;

        /// @brief Sets a vec4 (or vec4 array) uniform from raw float data.
        void SetUniformVec4(std::string_view name, const f32* value, const u32 count = 1) const;

        /// @brief Sets a vec4 uniform from separate x, y, z, w components.
        void SetUniformVec4(std::string_view name, f32 x, f32 y, f32 z, f32 w) const;

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

        u32 compileShader(const char* shaderCode, const int32_t shaderType);

        /// @brief Reads a shader source with its #include lines replaced by the files they name, and the defines set right after its #version line. Empty when a file can't be read.
        static std::string loadShaderSource(const char* shaderPath, const std::vector<std::string>& defines);

        /// @brief Appends the file's lines to source, an #include "file" line by that file's lines in turn. The file is named
        ///        relative to the one including it, and a file already included is skipped. That holds even when the first include
        ///        sat in an #if branch the defines drop, since #if is left to the GLSL compiler: a file every branch needs is
        ///        included before the #if.
        static bool appendWithIncludes(const std::filesystem::path& path, std::unordered_set<std::string>& included, std::string& source);

        /// @brief The location of the uniform with the given name. -1 when the program has none
        i32 getUniformLocation(std::string_view name) const;

        /// @brief The four planes from projection's eye through the edges of box (left, bottom, width, height) at depth in the
        ///        layout frame tilts into place (each keeping the side the box lies on)
        static std::array<glm::vec4, 4> clipPlanesOf(const glm::vec4& box, const f32 depth, const glm::mat4& frame, const Projection& projection);

        /// @brief Switches the clip distances in use on and the rest off
        static void enableClipPlanes(const u32 count, const u32 previousCount);

        u32 _programId = 0;
        i32 _projectionLocation = -1;
        mutable std::unordered_map<std::string, i32, NameHash, std::equal_to<>> _uniformLocations {}; //asked from GL once per name

        static inline f32 _drawOpacity { 1.0f }; //the product of the OpacityScopes around what is drawn now
        static inline glm::mat4 _drawFrame { 1.0f }; //the frame of the FrameScopes around what is drawn now
        static constexpr u32 _maxClipPlanes { 8 }; //as many as Clip.glsl measures. GL guarantees 8 clip distances
        static inline std::array<glm::vec4, _maxClipPlanes> _clipPlanes {}; //the ClipScopes' planes in the window before the projection
        static inline u32 _clipPlaneCount { 0 };
        static inline glm::vec4 _clipBox { 0.0f }; //the innermost rectangle, in its frame's untilted layout: the last four planes
        static inline glm::mat4
            _clipBoxFrame { 1.0f },
            _clipBoxView { 1.0f }; //the projection it was seen through
        static inline f32 _clipBoxDepth { 0.0f };
    };
}