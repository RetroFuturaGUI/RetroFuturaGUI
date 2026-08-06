#pragma once
#include "IncludeHelper.hpp"
#include "Projection.hpp"
#include <string>
#include <string_view>
#include <vector>

namespace RetroFuturaGUI
{
    class Texture
    {
    public:
        // projection is only needed if this texture will draw itself as a standalone quad
        // (e.g. Image). Textures used purely as material maps (e.g. Mesh) can omit it
        Texture(std::string_view path, const bool flipVertically = true, Projection* projection = nullptr);
        Texture(const Texture&) = delete;
        Texture& operator=(const Texture&) = delete;
        Texture(Texture&& other) noexcept;
        Texture& operator=(Texture&& other) noexcept;
        ~Texture();

        void Draw() const;
        void SetSize(const glm::vec2& size);
        void SetPosition(const glm::vec3& position);
        void SetRotation(const f32 rotation);

        bool IsTextureVerticallyFlipped() const;
        glm::i32vec2 GetResolution() const;
        i32 GetColorChannelCount() const;
        f32 GetAspectRatio() const;
        std::vector<u8>* GetTextureData();
        std::string_view GetPath() const;
        void SetType(std::string_view type);
        std::string_view GetType() const;
        u32 GetID() const;

    private:
        void loadTexture(std::string_view path);
        void uploadToGPU();
        void setupQuad();

        u32 _id = 0;
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