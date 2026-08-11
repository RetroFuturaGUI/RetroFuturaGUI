#include "SvgImage.hpp"

RetroFuturaGUI::SvgImage::SvgImage(const std::string& name, Projection* projection, IWidget* parentWidget, const WidgetTypeID parentWidgetTypeID, GLFWwindow* parentWindow, std::string_view SvgImagePath)
    : IWidget(name, projection, parentWidget, parentWidgetTypeID, parentWindow)
{
    _widgetTypeID = WidgetTypeID::SvgImage;
    _svgTexture = std::make_unique<SvgTexture>(SvgImagePath, true, projection);
    _svgTextureAspectRatio = _svgTexture->GetAspectRatio();
}

void RetroFuturaGUI::SvgImage::SetSize(const glm::vec3& size)
{
    IWidget::SetSize(size);
    _quadSize = glm::vec2(size.x, size.y);
    FitGeometryToTexture();
}

void RetroFuturaGUI::SvgImage::SetPosition(const glm::vec3& position)
{
    IWidget::SetPosition(position);
    _svgTexture->SetPosition(position);
}

void RetroFuturaGUI::SvgImage::SetRotation(const glm::vec3& rotation)
{
    IWidget::SetRotation(rotation);

    if(_svgTexture)
        _svgTexture->SetRotation(rotation);
}

void RetroFuturaGUI::SvgImage::FitGeometryToTexture()
{
    _quadSize = glm::vec2(_quadSize.y * _svgTextureAspectRatio, _quadSize.y);
    _svgTexture->SetSize(_quadSize);
}

void RetroFuturaGUI::SvgImage::Draw()
{
    if (_svgTexture)
        _svgTexture->Draw();
}