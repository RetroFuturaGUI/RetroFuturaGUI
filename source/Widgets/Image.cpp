#include "Image.hpp"

RetroFuturaGUI::Image::Image(const std::string& name, Projection* projection, IWidget* parentWidget, const WidgetTypeID parentWidgetTypeID, GLFWwindow* parentWindow, std::string_view imagePath)
    : IWidget(name, projection, parentWidget, parentWidgetTypeID, parentWindow)
{
    _widgetTypeID = WidgetTypeID::Image;
    _texture = std::make_unique<Texture>(imagePath, true, projection);
    _textureAspectRatio = _texture->GetAspectRatio();
}

RetroFuturaGUI::Image::Image(const std::string& name, Projection* projection, IWidget* parentWidget, const WidgetTypeID parentWidgetTypeID, GLFWwindow* parentWindow)
    : IWidget(name, projection, parentWidget, parentWidgetTypeID, parentWindow)
{
    _widgetTypeID = WidgetTypeID::Image;
}

void RetroFuturaGUI::Image::SetSize(const glm::vec3& size)
{
    IWidget::SetSize(size);
    FitGeometryToTexture();
}

void RetroFuturaGUI::Image::SetPosition(const glm::vec3& position)
{
    IWidget::SetPosition(position);

    if(!_texture)
        return;

    _texture->SetPosition(position);
}

void RetroFuturaGUI::Image::SetRotation(const glm::vec3& rotation)
{
    IWidget::SetRotation(rotation);

    if(!_texture)
        return;

    _texture->SetRotation(rotation);
}

void RetroFuturaGUI::Image::SetImage(DecodedImage&& image)
{
    _texture = std::make_unique<Texture>(std::move(image), &_projection);
    _textureAspectRatio = _texture->GetAspectRatio();
    _texture->SetPosition(_position);
    _texture->SetRotation(_rotation);
    FitGeometryToTexture();
}

void RetroFuturaGUI::Image::ClearImage()
{
    _texture.reset();
    _textureAspectRatio = 0.0f;
}

void RetroFuturaGUI::Image::FitGeometryToTexture()
{
    if(!_texture)
        return;

    if(0.0f >= _textureAspectRatio)
        return;

    //As wide as the box allows, then as tall as that width makes it, so a picture wider than the box shrinks instead of spilling out
    const f32 width { std::min(_size.x, _size.y * _textureAspectRatio) };
    _texture->SetSize(glm::vec2(width, width / _textureAspectRatio));
}

void RetroFuturaGUI::Image::Draw()
{
    if(!_texture)
        return;

    _texture->Draw();
}

u32 RetroFuturaGUI::Image::GetTextureID()
{
    if(!_texture)
        return 0;

    return _texture->GetID();
}

f32 RetroFuturaGUI::Image::GetAspectRatio() const
{
    return _textureAspectRatio;
}

glm::i32vec2 RetroFuturaGUI::Image::GetResolution() const
{
    if(!_texture)
        return glm::i32vec2(0);

    return _texture->GetResolution();
}