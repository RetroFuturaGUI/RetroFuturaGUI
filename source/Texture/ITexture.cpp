#include "ITexture.hpp"

RetroFuturaGUI::ITexture::ITexture(const bool verticallyFlipped)
    : _verticallyFlipped(verticallyFlipped)
{}

RetroFuturaGUI::ITexture::ITexture(ITexture&& other) noexcept
    : _id(other._id), _resolution(other._resolution), _aspectRatio(other._aspectRatio), _verticallyFlipped(other._verticallyFlipped)
{
    other._id = 0;
}

RetroFuturaGUI::ITexture& RetroFuturaGUI::ITexture::operator=(ITexture&& other) noexcept
{
    if(this == &other)
        return *this;

    glDeleteTextures(1, &_id);

    _id = other._id;
    _resolution = other._resolution;
    _aspectRatio = other._aspectRatio;
    _verticallyFlipped = other._verticallyFlipped;

    other._id = 0;

    return *this;
}

RetroFuturaGUI::ITexture::~ITexture()
{
    glDeleteTextures(1, &_id);
}

void RetroFuturaGUI::ITexture::Bind(const u32 unit) const
{
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, _id);
}

glm::i32vec2 RetroFuturaGUI::ITexture::GetResolution() const
{
    return _resolution;
}

f32 RetroFuturaGUI::ITexture::GetAspectRatio() const
{
    return _aspectRatio;
}

bool RetroFuturaGUI::ITexture::IsTextureVerticallyFlipped() const
{
    return _verticallyFlipped;
}

u32 RetroFuturaGUI::ITexture::GetID() const
{
    return _id;
}