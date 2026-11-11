#include "ColorPreview.hpp"
#include "IncludeHelper.hpp"
#include <memory>
#include <span>

RetroFuturaGUI::ColorPreview::ColorPreview(std::string_view name, Projection* projection, IHierarchyNode* parentWidget, const WidgetTypeID parentWidgetTypeID, GLFWwindow* parentWindow)
    : IWidget(name, projection, parentWidget, parentWidgetTypeID, parentWindow)
{
    _widgetTypeID = WidgetTypeID::ColorPreview;
    useFlavors(nullptr, &_border, nullptr);
    _checkeredBackground = std::make_unique<Plane>(&_projection);
    _colorRect = std::make_unique<Plane>(&_projection);
    _opaqueColorRect = std::make_unique<Plane>(&_projection);
    _border.SetTransitionSettings(&_transitionSettings); //the preview color is a value, not a style, so it switches at once

    if(!_checkeredBackground)
        return;

    if(!_colorRect)
        return;

    if(!_opaqueColorRect)
        return;

    _checkeredBackground->SetColors(_checkeredFillColor);
    _checkeredBackground->SetRasterPattern(RasterPattern::Checkered);
    _checkeredBackground->SetPrimaryRasterColor(_primaryCheckeredColor);
    _checkeredBackground->SetSecondaryRasterColor(_secondaryCheckeredColor);
    _checkeredBackground->SetPrimaryRasterWidthTransfer(_checkeredSquareWidth);
    _checkeredBackground->SetRasterDegree(0.0f);
    _colorRect->SetColors(std::span<const glm::vec4>(&_previewColor, 1));
    _opaqueColorRect->SetColors(std::span<const glm::vec4>(&_opaquePreviewColor, 1));
}

void RetroFuturaGUI::ColorPreview::Draw()
{
    if(!_checkeredBackground)
        return;

    if(!_colorRect)
        return;

    if(!_opaqueColorRect)
        return;

    _checkeredBackground->Draw();
    _colorRect->Draw();

    if(_isDualPreviewEnabled)
        _opaqueColorRect->Draw();

    _border.Draw();
}

const glm::vec4& RetroFuturaGUI::ColorPreview::GetColor() const
{
    return _previewColor;
}

void RetroFuturaGUI::ColorPreview::SetPreviewColor(const glm::vec4& color)
{
    _previewColor = color;
    _opaquePreviewColor = glm::vec4(glm::vec3(color), 1.0f);

    if(!_colorRect)
        return;

    if(!_opaqueColorRect)
        return;

    _colorRect->SetColors(std::span<const glm::vec4>(&_previewColor, 1));
    _opaqueColorRect->SetColors(std::span<const glm::vec4>(&_opaquePreviewColor, 1));
}

void RetroFuturaGUI::ColorPreview::applySize(const glm::vec3& size)
{
    IWidget::applySize(size);
    alignColorPreview();
}

void RetroFuturaGUI::ColorPreview::applyPosition(const glm::vec3& position)
{
    IWidget::applyPosition(position);
    alignColorPreview();
}

void RetroFuturaGUI::ColorPreview::EnableDualPreview(const bool enable)
{
    _isDualPreviewEnabled = enable;
    alignColorPreview();
}

void RetroFuturaGUI::ColorPreview::SetDualPreviewAlignment(const DualPreviewAlignment alignment)
{
    _previewAlignment = alignment;
    alignColorPreview();
}

void RetroFuturaGUI::ColorPreview::alignColorPreview()
{
    if(!_checkeredBackground)
        return;

    if(!_colorRect)
        return;

    if(!_opaqueColorRect)
        return;

    _border.SetSize(_size);
    _border.SetPosition(_position + glm::vec3(0.0f, 0.0f, 0.02f));

    if(!_isDualPreviewEnabled)
    {
        _checkeredBackground->SetSize(glm::vec2(_size.x, _size.y));
        _checkeredBackground->SetPosition(_position);
        _colorRect->SetSize(_size);
        _colorRect->SetPosition(_position + glm::vec3(0.0f, 0.0f, 0.01f));
        return;
    }

    const bool horizontal { _previewAlignment == DualPreviewAlignment::Horizontal };
    const glm::vec2 halfSize { horizontal ? glm::vec2(_size.x * 0.5f, _size.y) : glm::vec2(_size.x, _size.y * 0.5f) };

    const glm::vec3 checkeredOffset { horizontal
        ? glm::vec3(-halfSize.x * 0.5f, 0.0f, 0.0f)
        : glm::vec3(0.0f, halfSize.y * 0.5f, 0.0f) };

    //The checkerboard and the color over it take one half, the opaque color the other.
    _checkeredBackground->SetSize(halfSize);
    _checkeredBackground->SetPosition(_position + checkeredOffset);
    _colorRect->SetSize(halfSize);
    _colorRect->SetPosition(_position + checkeredOffset + glm::vec3(0.0f, 0.0f, 0.01f));
    _opaqueColorRect->SetSize(halfSize);
    _opaqueColorRect->SetPosition(_position - checkeredOffset + glm::vec3(0.0f, 0.0f, 0.01f));
}

RetroFuturaGUI::Border& RetroFuturaGUI::ColorPreview::GetBorder()
{
    return _border;
}

const RetroFuturaGUI::Border& RetroFuturaGUI::ColorPreview::GetBorder() const
{
    return _border;
}