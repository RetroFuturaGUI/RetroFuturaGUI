#include "SvgImage.hpp"
#include "ResourceManager.hpp"

RetroFuturaGUI::SvgImage::SvgImage(const std::string& name, Projection* projection, IWidget* parentWidget, const WidgetTypeID parentWidgetTypeID, GLFWwindow* parentWindow, std::string_view SvgImagePath)
    : IWidget(name, projection, parentWidget, parentWidgetTypeID, parentWindow)
{
    _widgetTypeID = WidgetTypeID::SvgImage;
    _svgTexture = ResourceManager::CreateSvgTexture(SvgImagePath, projection);

    if(!_svgTexture)
        return;

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

    if(!_svgTexture)
        return;

    _svgTexture->SetPosition(position);
}

void RetroFuturaGUI::SvgImage::SetRotation(const glm::vec3& rotation)
{
    if(!_svgTexture)
        return;

    IWidget::SetRotation(rotation);

    _svgTexture->SetRotation(rotation);
}

void RetroFuturaGUI::SvgImage::FitGeometryToTexture()
{
    if(!_svgTexture)
        return;

    _quadSize = glm::vec2(_quadSize.y * _svgTextureAspectRatio, _quadSize.y);
    _svgTexture->SetSize(_quadSize);
}

void RetroFuturaGUI::SvgImage::Draw()
{
    if(!_svgTexture)
        return;

    _svgTexture->Draw();
}

void RetroFuturaGUI::SvgImage::SetPathFill(std::string_view name, const SvgPathFill& fill)
{
    if(!_svgTexture)
        return;

    _svgTexture->SetPathFill(name, fill);
}

void RetroFuturaGUI::SvgImage::ClearPathFill(std::string_view name)
{
    if(!_svgTexture)
        return;

    _svgTexture->ClearPathFill(name);
}

const std::vector<RetroFuturaGUI::SvgPathInfo>& RetroFuturaGUI::SvgImage::GetNamedPaths() const
{
    static const std::vector<SvgPathInfo> empty;
    return _svgTexture ? _svgTexture->GetNamedPaths() : empty;
}