#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "Texture.hpp"
#include <print>

RetroFuturaGUI::Texture::Texture(std::string_view path, const bool flipVertically)
: _verticallyFlipped(flipVertically)
{
    loadTexture(path);
    _aspectRatio = static_cast<f32>(_resolution.x) / static_cast<f32>(_resolution.y);
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
    stbi_set_flip_vertically_on_load(_verticallyFlipped);
    u8* tempData = stbi_load(path.data(), &_resolution.x, &_resolution.y, &_colorChannelCount, 0);

    if (tempData)
        _texture = std::vector<u8>(tempData, tempData + _resolution.x * _resolution.y * _colorChannelCount);
    else
        std::println("Error loading image.");

    if(tempData)
        stbi_image_free(tempData);
}
