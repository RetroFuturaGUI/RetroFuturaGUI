#include "Image2D.hpp"

RetroFuturaGUI::Image2D::Image2D(const std::string& name, Projection* projection, IWidget* parentWidget, const WidgetTypeID parentWidgetTypeID, GLFWwindow* parentWindow, std::string_view imagePath)
    : IWidget(name, projection, parentWidget, parentWidgetTypeID, parentWindow)
{
    _widgetTypeID = WidgetTypeID::Image;
    _texture = std::make_unique<Texture>(imagePath, true, projection);
    _textureAspectRatio = _texture->GetAspectRatio();
}

void RetroFuturaGUI::Image2D::SetSize(const glm::vec3& size)
{
    IWidget::SetSize(size);
    _quadSize = glm::vec2(size.x, size.y);
    FitGeometryToTexture();
}

void RetroFuturaGUI::Image2D::SetPosition(const glm::vec3& position)
{
    IWidget::SetPosition(position);
    _texture->SetPosition(position);
}

void RetroFuturaGUI::Image2D::SetRotation(const f32 rotation)
{
    IWidget::SetRotation(rotation);
    _texture->SetRotation(rotation);
}

void RetroFuturaGUI::Image2D::FitGeometryToTexture()
{
    _quadSize = glm::vec2(_quadSize.y * _textureAspectRatio, _quadSize.y);
    _texture->SetSize(_quadSize);
}

void RetroFuturaGUI::Image2D::Draw()
{
    if (_texture)
        _texture->Draw();
}

u32 RetroFuturaGUI::Image2D::GetTextureID()
{
    return _texture->GetID();
}