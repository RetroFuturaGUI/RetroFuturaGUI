#pragma once
#include "ITexture.hpp"
#include <string>
#include <string_view>
#include <vector>

namespace RetroFuturaGUI
{
    enum ImageFormat : i32
    {
        Unknown = -1,
        PNG = 0,
        JPG,
        JPEG = JPG,
        SVG
    };

    /// @brief A texture loaded once from an image file (PNG, JPG or SVG) and uploaded to the GPU.
    class Texture final : public ITexture
    {
    public:
        /// @brief Loads a texture from disk and uploads it to the GPU.
        /// @param projection Only needed if this texture will draw itself as a standalone quad (e.g. Image).
        /// Textures used purely as material maps (e.g. Mesh) can omit it.
        Texture(std::string_view path, const bool flipVertically = true, Projection* projection = nullptr);
        Texture(const Texture&) = delete;
        Texture& operator=(const Texture&) = delete;
        Texture(Texture&& other) noexcept;
        Texture& operator=(Texture&& other) noexcept;
        ~Texture() override = default; // the GL objects are ITexture's

        /// @brief Returns the number of color channels in the texture.
        i32 GetColorChannelCount() const;

        /// @brief Returns the raw pixel data - empty once uploaded, the GPU keeps the only copy.
        std::vector<u8>* GetTextureData();

        /// @brief Returns the filesystem path the texture was loaded from.
        std::string_view GetPath() const;

        /// @brief Sets a free-form type tag for the texture (e.g. "diffuse", "specular").
        void SetType(std::string_view type);

        /// @brief Returns the type tag of the texture.
        std::string_view GetType() const;

    private:
        void loadTexture(std::string_view path);
        void loadSVG(std::string_view path);
        void loadRasterImage(std::string_view path);
        void uploadToGPU();

        ImageFormat _format { ImageFormat::Unknown };
        i32 _colorChannelCount { 0 };
        std::vector<u8> _texture {};
        std::string
            _path {},
            _type {};
    };
}