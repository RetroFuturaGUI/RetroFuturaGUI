#pragma once
#include "PlatformBridge.hpp"
#include "ShaderManager.hpp"
#include "Texture.hpp"

namespace RetroFuturaGUI
{
    class Image2D
    {
    public:
        Image2D(const GeometryParams2D& geometry);
        ~Image2D();
        void Resize(const glm::vec2& size);
        void Move(const glm::vec2& position);
        void Rotate(const f32 rotation);
        void FitGeometryToTexture();
        void Draw();

    private:
        // Geometry
        Projection& _projection;
        glm::vec2 
            _quadSize { 1.0f },
            _textureSize { 1.0f },
            _position { 0.0f };
        glm::mat4
            _scaleMatrix { 0.0f },
            _translationMatrix { 0.0f },
            _rotationMatrix { 0.0f };
        f32 
            _textureAspectRatio { 0.0f },
            _rotation { 0.0f };
        std::unique_ptr<Texture> _texture;

        float _vertices[(3 + 2) * 4] =
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

        // data
        std::string _path;
        u32 
            _textureId { 0 },
            _vao { 0 },
            _vbo { 0 },
            _ebo { 0 },
            _colorChannelCount { 0 };
        glm::i32vec2 _imageSize { 0, 0 };

        void loadTexture();
    };
}