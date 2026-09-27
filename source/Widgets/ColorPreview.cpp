#include "ColorPreview.hpp"
#include "IncludeHelper.hpp"
#include "Rectangle.hpp"
#include <memory>
#include <span>

RetroFuturaGUI::ColorPreview::ColorPreview(std::string_view name, Projection* projection, IHierarchyNode* parentWidget, const WidgetTypeID parentWidgetTypeID, GLFWwindow* parentWindow)
    : IWidget(name, projection, parentWidget, parentWidgetTypeID, parentWindow)
{
    _widgetTypeID = WidgetTypeID::ColorPreview;
    _checkeredBackground = std::make_unique<Rectangle>(&_projection);
    _whiteBackground = std::make_unique<Rectangle>(&_projection);
    _colorRect = std::make_unique<Rectangle>(&_projection);
    _border = std::make_unique<Rectangle>(&_projection);

    if(!_checkeredBackground)
        return;

    if(!_whiteBackground)
        return;

    if(!_colorRect)
        return;

    if(!_border)
        return;

    _checkeredBackground->SetRectangleMode(RectangleMode::Plane);
    _checkeredBackground->SetColors(_checkeredFillColor);
    _checkeredBackground->SetRasterPattern(RasterPattern::Checkered);
    _checkeredBackground->SetPrimaryRasterColor(_primaryCheckeredColor);
    _checkeredBackground->SetSecondaryRasterColor(_secondaryCheckeredColor);
    _checkeredBackground->SetPrimaryRasterWidthTransfer(_checkeredSquareWidth);
    _checkeredBackground->SetRasterDegree(0.0f);
    _whiteBackground->SetRectangleMode(RectangleMode::Plane);
    _whiteBackground->SetColors(_whiteBackgroundColor);
    _colorRect->SetRectangleMode(RectangleMode::Plane);
    _colorRect->SetColors(std::span<glm::vec4>(&_previewColor, 1));
    _border->SetRectangleMode(RectangleMode::Border);
    setBorderColors();
}

void RetroFuturaGUI::ColorPreview::Draw()
{
    if(!_checkeredBackground)
        return;

    if(!_whiteBackground)
        return;

    if(!_colorRect)
        return;

    _checkeredBackground->Draw();

    if(_useWhiteBackgroundRect)
        _whiteBackground->Draw();

    _colorRect->Draw();
    drawBorder();
}

const glm::vec4& RetroFuturaGUI::ColorPreview::GetColor() const
{
    return _previewColor;
}

void RetroFuturaGUI::ColorPreview::SetPreviewColor(const glm::vec4& color)
{
    _previewColor = color;

    if(!_colorRect)
        return;

    _colorRect->SetColors(std::span<glm::vec4>(&_previewColor, 1));
}

void RetroFuturaGUI::ColorPreview::SetSize(const glm::vec3& size)
{
    IWidget::SetSize(size);
    alignColorPreview();
}

void RetroFuturaGUI::ColorPreview::SetPosition(const glm::vec3& position)
{
    IWidget::SetPosition(position);
    alignColorPreview();
}

void RetroFuturaGUI::ColorPreview::SetRotation(const glm::vec3& rotation)
{
    IWidget::SetRotation(rotation);

    if(!_checkeredBackground)
        return;

    if(!_whiteBackground)
        return;

    if(!_colorRect)
        return;

    if(!_border)
        return;

    _checkeredBackground->SetRotation(rotation);
    _whiteBackground->SetRotation(rotation);
    _colorRect->SetRotation(rotation);
    _border->SetRotation(rotation);
}

void RetroFuturaGUI::ColorPreview::EnableDualPreviewBackground(const bool enable)
{
    _useWhiteBackgroundRect = enable;
    alignColorPreview();
}

void RetroFuturaGUI::ColorPreview::SetDualPreviewAlignment(const DualPrevieAlignment alignment)
{
    _previewAlignment = alignment;
    alignColorPreview();
}

void RetroFuturaGUI::ColorPreview::alignColorPreview()
{
    if(!_checkeredBackground)
        return;

    if(!_whiteBackground)
        return;

    if(!_colorRect)
        return;

    if(!_border)
        return;

    _colorRect->SetSize(_size);
    _colorRect->SetPosition(_position + glm::vec3(0.0f, 0.0f, 0.01f));
    _border->SetSize(_size);
    _border->SetPosition(_position + glm::vec3(0.0f, 0.0f, 0.02f));

    if(!_useWhiteBackgroundRect)
    {
        _checkeredBackground->SetSize(glm::vec2(_size.x, _size.y));
        _checkeredBackground->SetPosition(_position);
        return;
    }

    const bool horizontal { _previewAlignment == DualPrevieAlignment::Horizontal };
    const glm::vec2 halfSize { horizontal ? glm::vec2(_size.x * 0.5f, _size.y) : glm::vec2(_size.x, _size.y * 0.5f) };

    const glm::vec3 checkeredOffset { horizontal
        ? glm::vec3(-halfSize.x * 0.5f, 0.0f, 0.0f)
        : glm::vec3(0.0f, halfSize.y * 0.5f, 0.0f) };

    _checkeredBackground->SetSize(halfSize);
    _checkeredBackground->SetPosition(_position + checkeredOffset);
    _whiteBackground->SetSize(halfSize);
    _whiteBackground->SetPosition(_position - checkeredOffset);
}