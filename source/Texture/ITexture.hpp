#pragma once
#include "IncludeHelper.hpp"
#include "Projection.hpp"
#include "config.hpp"

namespace RetroFuturaGUI
{
    class ITexture
    {
    public:
        ITexture(const ITexture&) = delete;
        ITexture& operator=(const ITexture&) = delete;

        /// @brief Deletes the texture and the standalone quad.
        virtual ~ITexture();

        /// @brief Binds the texture to a texture unit, for whatever draws next - a mesh, a widget.
        void Bind(const u32 unit) const;

        /// @brief Draws the texture as a standalone quad. Requires a Projection, otherwise does nothing.
        void Draw() const;

        /// @brief Sets the size of the standalone quad.
        void SetSize(const glm::vec2& size);

        /// @brief Sets the world position of the standalone quad.
        void SetPosition(const glm::vec3& position);

        /// @brief Sets the rotation of the standalone quad.
        void SetRotation(const glm::vec3& rotation);

        /// @brief Returns the pixel resolution of the texture.
        glm::i32vec2 GetResolution() const;

        /// @brief Returns the width-to-height aspect ratio of the texture.
        f32 GetAspectRatio() const;

        /// @brief Returns whether the pixels are stored bottom row first, the way OpenGL expects them.
        bool IsTextureVerticallyFlipped() const;

        /// @brief Returns the OpenGL texture object ID.
        u32 GetID() const;

    protected:
        /// @param projection Only needed if the texture draws itself as a standalone quad (e.g. Image).
        /// Textures used purely as material maps (e.g. Mesh) can omit it.
        explicit ITexture(Projection* projection, const bool verticallyFlipped);
        ITexture(ITexture&& other) noexcept;
        ITexture& operator=(ITexture&& other) noexcept;

        /// @brief Builds the standalone quad's buffers. Does nothing without a Projection.
        void setupQuad();

        u32 _id { 0 };
        glm::i32vec2 _resolution { 0 };
        f32 _aspectRatio { 0.0f };
        bool _verticallyFlipped { false };
        Projection* _projection { nullptr };

    private:
        void releaseGPUResources();

        // Standalone quad rendering (only used with a Projection)
        static constexpr f32 Vertices[(3 + 2) * 4] =
        {   //   positions     | tex coords
            -0.5f, -0.5f, 0.0f, 0.0f, 0.0f,
             0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
             0.5f,  0.5f, 0.0f, 1.0f, 1.0f,
            -0.5f,  0.5f, 0.0f, 0.0f, 1.0f
        };
        static constexpr u32 Indices[6] =
        {
            0, 1, 2,
            2, 3, 0
        };
        u32
            _vao { 0 },
            _vbo { 0 },
            _ebo { 0 };
        glm::mat4
            _scalingMatrix { 1.0f },
            _translationMatrix { 1.0f },
            _rotationMatrix { 1.0f };
    };
}
