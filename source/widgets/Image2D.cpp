#include "Image2D.hpp"
#include "ShaderManager.hpp"

RetroFuturaGUI::Image2D::Image2D(const GeometryParams2D &geometry, std::string_view imagePath)
: _projection(const_cast<Projection&>(geometry._Projection)), _path(imagePath)
{
    glGenVertexArrays(1, &_vao);
    glGenBuffers(1, &_vbo);
    glGenBuffers(1, &_ebo);

    glBindVertexArray(_vao);

    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(_vertices), _vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(_indices), _indices, GL_STATIC_DRAW);

    // position attribute
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(f32), nullptr);

    // texture coordinates
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(f32), (void*)(3 * sizeof(f32)));
    glEnableVertexAttribArray(1);

    loadTexture();
    glActiveTexture(GL_TEXTURE0);

    _textureAspectRatio = (f32)_imageSize.x / (f32)_imageSize.y; 
    SetSize(geometry._Size);
    FitGeometryToTexture();
    Move(geometry._Position);
    Rotate(geometry._Rotation);

    u32 err;
    while ((err = glGetError()) != GL_NO_ERROR)
        std::println("OpenGL error: ", err);
}

RetroFuturaGUI::Image2D::~Image2D()
{
    glDeleteVertexArrays(1, &_vao);
    glDeleteBuffers(1, &_vbo);
    glDeleteBuffers(1, &_ebo);
}

void RetroFuturaGUI::Image2D::SetSize(const glm::vec2& size)
{
    _quadSize = size;
    _scaleMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(_quadSize, 1.0f));
}

void RetroFuturaGUI::Image2D::Move(const glm::vec2& position)
{
    _position = position;
    _translationMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(_position, 0.0f));
}

void RetroFuturaGUI::Image2D::Rotate(const f32 rotation)
{
    _rotation = rotation;
    _rotationMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(_rotation), glm::vec3(0.0f, 0.0f, 1.0f));
}

void RetroFuturaGUI::Image2D::FitGeometryToTexture()
{
    _quadSize = glm::vec2(_quadSize.y * _textureAspectRatio, _quadSize.y);
    _scaleMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(_quadSize, 1.0f));
}

void RetroFuturaGUI::Image2D::Draw()
{
    ShaderManager::GetTextureFillShader().UseProgram();
    ShaderManager::GetTextureFillShader().SetUniformInt("uTexture", 0);
    ShaderManager::GetTextureFillShader().SetUniformMat4("uProjection", _projection.GetProjectionMatrix());
    ShaderManager::GetTextureFillShader().SetUniformMat4("uPosition", _translationMatrix);
    ShaderManager::GetTextureFillShader().SetUniformMat4("uScaling", _scaleMatrix);
    ShaderManager::GetTextureFillShader().SetUniformMat4("uRotation", _rotationMatrix);


    glBindTexture(GL_TEXTURE_2D, _textureId);
    glBindVertexArray(_vao);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

u32 RetroFuturaGUI::Image2D::GetTextureID()
{
    return _textureId;
}

void RetroFuturaGUI::Image2D::loadTexture()
{
    glGenTextures(1, &_textureId);
    glBindTexture(GL_TEXTURE_2D, _textureId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    _texture = std::make_unique<Texture>(_path, true);

    if(!_texture)
        return;
        
    _colorChannelCount = _texture->GetColorChannelCount();
    _imageSize = _texture->GetResolution();
    u32 format = (_colorChannelCount == 4) ? GL_RGBA : GL_RGB;
    glTexImage2D(GL_TEXTURE_2D, 0, format, _imageSize.x, _imageSize.y, 0, format, GL_UNSIGNED_BYTE, _texture->GetTextureData()->data());
    glGenerateMipmap(GL_TEXTURE_2D);
}