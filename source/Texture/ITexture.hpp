#pragma once
#include "IncludeHelper.hpp"
#include "config.hpp"

namespace RetroFuturaGUI
{
    /// @brief A texture on the GPU. It doesn't draw itself: a Plane shows one as its image, a Mesh as a material map.
    class ITexture
    {
    public:
        ITexture(const ITexture&) = delete;
        ITexture& operator=(const ITexture&) = delete;

        /// @brief Deletes the texture.
        virtual ~ITexture();

        /// @brief Binds the texture to a texture unit, for whatever draws next - a mesh, a widget.
        void Bind(const u32 unit) const;

        /// @brief Returns the pixel resolution of the texture.
        glm::i32vec2 GetResolution() const;

        /// @brief Returns the width-to-height aspect ratio of the texture.
        f32 GetAspectRatio() const;

        /// @brief Returns whether the pixels are stored bottom row first, the way OpenGL expects them.
        bool IsTextureVerticallyFlipped() const;

        /// @brief Returns the OpenGL texture object ID.
        u32 GetID() const;

    protected:
        explicit ITexture(const bool verticallyFlipped);
        ITexture(ITexture&& other) noexcept;
        ITexture& operator=(ITexture&& other) noexcept;

        u32 _id { 0 };
        glm::i32vec2 _resolution { 0 };
        f32 _aspectRatio { 0.0f };
        bool _verticallyFlipped { false };
    };
}