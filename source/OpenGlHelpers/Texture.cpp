#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "Texture.hpp"
#include "ShaderManager.hpp"
#include <print>

RetroFuturaGUI::Texture::Texture(std::string_view path, const bool flipVertically, Projection* projection)
    : _verticallyFlipped(flipVertically), _projection(projection)
{
    loadTexture(path);
    _aspectRatio = static_cast<f32>(_resolution.x) / static_cast<f32>(_resolution.y);
    uploadToGPU();

    if (_projection)
        setupQuad();
}

RetroFuturaGUI::Texture::Texture(Texture&& other) noexcept
    : _id(other._id), _verticallyFlipped(other._verticallyFlipped), _aspectRatio(other._aspectRatio),
      _resolution(other._resolution), _colorChannelCount(other._colorChannelCount),
      _texture(std::move(other._texture)), _path(std::move(other._path)), _type(std::move(other._type)),
      _vao(other._vao), _vbo(other._vbo), _ebo(other._ebo), _projection(other._projection),
      _scalingMatrix(other._scalingMatrix), _translationMatrix(other._translationMatrix), _rotationMatrix(other._rotationMatrix)
{
    other._id = 0;
    other._vao = 0;
    other._vbo = 0;
    other._ebo = 0;
}

RetroFuturaGUI::Texture& RetroFuturaGUI::Texture::operator=(Texture&& other) noexcept
{
    if (this == &other)
        return *this;

    glDeleteTextures(1, &_id);
    glDeleteVertexArrays(1, &_vao);
    glDeleteBuffers(1, &_vbo);
    glDeleteBuffers(1, &_ebo);

    _id = other._id;
    _verticallyFlipped = other._verticallyFlipped;
    _aspectRatio = other._aspectRatio;
    _resolution = other._resolution;
    _colorChannelCount = other._colorChannelCount;
    _texture = std::move(other._texture);
    _path = std::move(other._path);
    _type = std::move(other._type);
    _vao = other._vao;
    _vbo = other._vbo;
    _ebo = other._ebo;
    _projection = other._projection;
    _scalingMatrix = other._scalingMatrix;
    _translationMatrix = other._translationMatrix;
    _rotationMatrix = other._rotationMatrix;

    other._id = 0;
    other._vao = 0;
    other._vbo = 0;
    other._ebo = 0;

    return *this;
}

RetroFuturaGUI::Texture::~Texture()
{
    glDeleteTextures(1, &_id);
    glDeleteVertexArrays(1, &_vao);
    glDeleteBuffers(1, &_vbo);
    glDeleteBuffers(1, &_ebo);
}

void RetroFuturaGUI::Texture::Draw() const
{
    if (_projection)
    {
        ShaderManager::GetTextureFillShader().UseProgram();
        ShaderManager::GetTextureFillShader().SetUniformInt("uTexture", 0);
        ShaderManager::GetTextureFillShader().SetUniformMat4("uProjection", _projection->GetProjectionMatrix());
        ShaderManager::GetTextureFillShader().SetUniformMat4("uPosition", _translationMatrix);
        ShaderManager::GetTextureFillShader().SetUniformMat4("uScaling", _scalingMatrix);
        ShaderManager::GetTextureFillShader().SetUniformMat4("uRotation", _rotationMatrix);
    }

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, _id);

    if (_projection)
    {
        glBindVertexArray(_vao);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
        glBindVertexArray(0);
    }
}

void RetroFuturaGUI::Texture::SetSize(const glm::vec2& size)
{
    _scalingMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(size, 1.0f));
}

void RetroFuturaGUI::Texture::SetPosition(const glm::vec3& position)
{
    _translationMatrix = glm::translate(glm::mat4(1.0f), position);
}

void RetroFuturaGUI::Texture::SetRotation(const glm::vec3& rotation)
{
    glm::vec3 radians = glm::radians(rotation);
    _rotationMatrix =
        glm::rotate(glm::mat4(1.0f), radians.z, glm::vec3(0.0f, 0.0f, 1.0f)) *
        glm::rotate(glm::mat4(1.0f), radians.y, glm::vec3(0.0f, 1.0f, 0.0f)) *
        glm::rotate(glm::mat4(1.0f), radians.x, glm::vec3(1.0f, 0.0f, 0.0f));
}

void RetroFuturaGUI::Texture::setupQuad()
{
    glGenVertexArrays(1, &_vao);
    glBindVertexArray(_vao);

    glGenBuffers(1, &_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(_vertices), _vertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(f32), nullptr);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(f32), (void*)(3 * sizeof(f32)));

    glGenBuffers(1, &_ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(_indices), _indices, GL_STATIC_DRAW);

    glBindVertexArray(0);
}

void RetroFuturaGUI::Texture::uploadToGPU()
{
    if (_texture.empty())
        return;

    glGenTextures(1, &_id);
    glBindTexture(GL_TEXTURE_2D, _id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    u32 format = (_colorChannelCount == 4) ? GL_RGBA : GL_RGB;
    glTexImage2D(GL_TEXTURE_2D, 0, static_cast<i32>(format), _resolution.x, _resolution.y, 0, format, GL_UNSIGNED_BYTE, _texture.data());
    glGenerateMipmap(GL_TEXTURE_2D);

    // The GPU now owns the pixel data; drop the CPU-side copy instead of holding both forever
    _texture.clear();
    _texture.shrink_to_fit();
}

bool RetroFuturaGUI::Texture::IsTextureVerticallyFlipped() const
{
    return _verticallyFlipped;
}

glm::i32vec2 RetroFuturaGUI::Texture::GetResolution() const
{
    return _resolution;
}

i32 RetroFuturaGUI::Texture::GetColorChannelCount() const
{
    return _colorChannelCount;
}

f32 RetroFuturaGUI::Texture::GetAspectRatio() const
{
    return _aspectRatio;
}

std::vector<u8>* RetroFuturaGUI::Texture::GetTextureData()
{
    return &_texture;
}

void RetroFuturaGUI::Texture::loadTexture(std::string_view path)
{
    _path = path;
    stbi_set_flip_vertically_on_load(_verticallyFlipped);
    u8* tempData = stbi_load(_path.data(), &_resolution.x, &_resolution.y, &_colorChannelCount, 0);

    if (tempData)
        _texture = std::vector<u8>(tempData, tempData + _resolution.x * _resolution.y * _colorChannelCount);
    else
        std::println("Error loading image.");

    if(tempData)
        stbi_image_free(tempData);
}

std::string_view RetroFuturaGUI::Texture::GetPath() const
{
    return _path;
}

void RetroFuturaGUI::Texture::SetType(std::string_view type)
{
    _type = type;
}

std::string_view RetroFuturaGUI::Texture::GetType() const
{
    return _type;
}

u32 RetroFuturaGUI::Texture::GetID() const
{
    return _id;
}