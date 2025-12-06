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
        glm::vec2 _quadSize = glm::vec2(1.0f);
        glm::vec2 _textureSize = glm::vec2(1.0f);
        f32 _textureAspectRatio = 0.0f;
        glm::mat4 _scaleMatrix;
        glm::vec2 _position;
        glm::mat4 _translationMatrix;
        f32 _rotation;
        glm::mat4 _rotationMatrix;
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
        u32 _textureId = 0;
        u32 _vao;
        u32 _vbo;
        u32 _ebo;
        glm::i32vec2 _imageSize = glm::u32vec2(0);
        i32 _colorChannelCount = 0;

        void loadTexture();
    };
}