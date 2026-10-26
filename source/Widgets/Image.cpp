#include "Image.hpp"
#include "ResourceManager.hpp"

RetroFuturaGUI::Image::Image(const std::string& name, Projection* projection, IWidget* parentWidget, const WidgetTypeID parentWidgetTypeID, GLFWwindow* parentWindow, std::string_view imagePath)
    : Image(name, projection, parentWidget, parentWidgetTypeID, parentWindow)
{
    showTexture(ResourceManager::GetTexture(imagePath));
}

RetroFuturaGUI::Image::Image(const std::string& name, Projection* projection, IWidget* parentWidget, const WidgetTypeID parentWidgetTypeID, GLFWwindow* parentWindow)
    : IWidget(name, projection, parentWidget, parentWidgetTypeID, parentWindow)
{
    _widgetTypeID = WidgetTypeID::Image;
    _plane = std::make_unique<Plane>(projection);

    if(!_plane)
        return;

    _plane->SetColors(_planeColors);
}

void RetroFuturaGUI::Image::SetSize(const glm::vec3& size)
{
    IWidget::SetSize(size);
    FitGeometryToTexture();
}

void RetroFuturaGUI::Image::SetPosition(const glm::vec3& position)
{
    IWidget::SetPosition(position);

    if(!_plane)
        return;

    _plane->SetPosition(position);
}

void RetroFuturaGUI::Image::SetRotation(const glm::vec3& rotation)
{
    IWidget::SetRotation(rotation);

    if(!_plane)
        return;

    _plane->SetRotation(rotation);
}

void RetroFuturaGUI::Image::SetImage(DecodedImage&& image)
{
    _decodedTexture = std::make_unique<Texture>(std::move(image));
    showTexture(_decodedTexture.get());
}

void RetroFuturaGUI::Image::ClearImage()
{
    showTexture(nullptr);
    _decodedTexture.reset();
}

void RetroFuturaGUI::Image::FitGeometryToTexture()
{
    if(!_plane)
        return;

    if(!_texture)
        return;

    if(0.0f >= _textureAspectRatio)
        return;

    //As wide as the box allows, then as tall as that width makes it, so a picture wider than the box shrinks instead of spilling out
    const f32 width { std::min(_size.x, _size.y * _textureAspectRatio) };
    _plane->SetSize(glm::vec2(width, width / _textureAspectRatio));
}

void RetroFuturaGUI::Image::Draw()
{
    if(!_plane)
        return;

    if(!_texture)
        return;

    _plane->Draw();
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

void RetroFuturaGUI::Image::showTexture(Texture* texture)
{
    if(!_plane)
        return;

    _texture = texture;
    _textureAspectRatio = _texture ? _texture->GetAspectRatio() : 0.0f;
    _plane->SetBackgroundImageTextureID(_texture ? _texture->GetID() : 0);
    FitGeometryToTexture();
}
