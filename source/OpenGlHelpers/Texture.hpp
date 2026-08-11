#pragma once
#include "IncludeHelper.hpp"
#include "Projection.hpp"
#include "config.hpp"
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

    class Texture
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
        ~Texture();

        /// @brief Draws the texture as a standalone quad. Requires the Texture to have been constructed with a Projection.
        void Draw() const;

        /// @brief Sets the size of the standalone quad.
        void SetSize(const glm::vec2& size);

        /// @brief Sets the world position of the standalone quad.
        void SetPosition(const glm::vec3& position);

        /// @brief Sets the rotation of the standalone quad.
        void SetRotation(const glm::vec3& rotation);

        /// @brief Returns whether the texture data was flipped vertically on load.
        bool IsTextureVerticallyFlipped() const;

        /// @brief Returns the pixel resolution of the texture.
        glm::i32vec2 GetResolution() const;

        /// @brief Returns the number of color channels in the texture.
        i32 GetColorChannelCount() const;

        /// @brief Returns the width-to-height aspect ratio of the texture.
        f32 GetAspectRatio() const;

        /// @brief Returns a pointer to the raw texture pixel data.
        std::vector<u8>* GetTextureData();

        /// @brief Returns the filesystem path the texture was loaded from.
        std::string_view GetPath() const;

        /// @brief Sets a free-form type tag for the texture (e.g. "diffuse", "specular").
        void SetType(std::string_view type);

        /// @brief Returns the type tag of the texture.
        std::string_view GetType() const;

        /// @brief Returns the OpenGL texture object ID.
        u32 GetID() const;

    private:
        void loadTexture(std::string_view path);
        void loadSVG(std::string_view path);
        void loadRasterImage(std::string_view path);
        void uploadToGPU();
        void setupQuad();

        u32 _id = 0;
        ImageFormat _format { ImageFormat::Unknown };
        bool _verticallyFlipped = false;
        f32 _aspectRatio = 0.0f;
        glm::i32vec2 _resolution = glm::u32vec2(0);
        i32 _colorChannelCount = 0;
        std::vector<u8> _texture;
        std::string
            _path {},
            _type {};

    // Standalone quad rendering (only used when constructed with a Projection)
        f32 _vertices[(3 + 2) * 4] =
        {   //   positions     | tex coords
            -0.5f, -0.5f, 0.0f, 0.0f, 0.0f,
             0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
             0.5f,  0.5f, 0.0f, 1.0f, 1.0f,
            -0.5f,  0.5f, 0.0f, 0.0f, 1.0f
        };
        u32 _indices[6] =
        {
            0, 1, 2,
            2, 3, 0
        };
        u32
            _vao { 0 },
            _vbo { 0 },
            _ebo { 0 };
        Projection* _projection { nullptr };
        glm::mat4
            _scalingMatrix { 1.0f },
            _translationMatrix { 1.0f },
            _rotationMatrix { 1.0f };
    };
}