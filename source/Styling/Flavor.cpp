#ifdef RETROFUTURAGUI_MODULES
module;
#include "ModuleExternals.hpp"
module RetroFuturaGUI;
#else
#include "Flavor.hpp"
#include "StylingManager.hpp"
#endif

void RetroFuturaGUI::FlavorPart::SetColors(std::span<const glm::vec4> colors, const ColorState state)
{
    _colors[static_cast<uSize>(state)].assign(colors.begin(), colors.end());
    StylingManager::markChanged();
}

const std::vector<glm::vec4>& RetroFuturaGUI::FlavorPart::GetColors(const ColorState state) const
{
    return _colors[static_cast<uSize>(state)];
}

void RetroFuturaGUI::FlavorPart::SetCornerRadii(const glm::vec4& radii)
{
    _cornerRadii = radii;
    StylingManager::markChanged();
}

void RetroFuturaGUI::FlavorPart::ResetCornerRadii()
{
    _cornerRadii.reset();
    StylingManager::markChanged();
}

const std::optional<glm::vec4>& RetroFuturaGUI::FlavorPart::GetCornerRadii() const
{
    return _cornerRadii;
}

void RetroFuturaGUI::FlavorPart::SetCutCorners(const CutCorners corners)
{
    _cutCorners = corners;
    StylingManager::markChanged();
}

void RetroFuturaGUI::FlavorPart::ResetCutCorners()
{
    _cutCorners.reset();
    StylingManager::markChanged();
}

const std::optional<RetroFuturaGUI::CutCorners>& RetroFuturaGUI::FlavorPart::GetCutCorners() const
{
    return _cutCorners;
}

void RetroFuturaGUI::FlavorPart::SetWidth(const f32 width)
{
    _width = width;
    StylingManager::markChanged();
}

void RetroFuturaGUI::FlavorPart::ResetWidth()
{
    _width.reset();
    StylingManager::markChanged();
}

const std::optional<f32>& RetroFuturaGUI::FlavorPart::GetWidth() const
{
    return _width;
}

void RetroFuturaGUI::FlavorPart::SetFillType(const FillType fillType)
{
    _fillType = fillType;
    StylingManager::markChanged();
}

void RetroFuturaGUI::FlavorPart::ResetFillType()
{
    _fillType.reset();
    StylingManager::markChanged();
}

const std::optional<RetroFuturaGUI::FillType>& RetroFuturaGUI::FlavorPart::GetFillType() const
{
    return _fillType;
}

void RetroFuturaGUI::FlavorPart::SetGradientDegree(const f32 gradientDegree)
{
    _gradientDegree = gradientDegree;
    StylingManager::markChanged();
}

void RetroFuturaGUI::FlavorPart::ResetGradientDegree()
{
    _gradientDegree.reset();
    StylingManager::markChanged();
}

const std::optional<f32>& RetroFuturaGUI::FlavorPart::GetGradientDegree() const
{
    return _gradientDegree;
}

void RetroFuturaGUI::FlavorPart::SetGradientOffset(const f32 gradientOffset)
{
    _gradientOffset = gradientOffset;
    StylingManager::markChanged();
}

void RetroFuturaGUI::FlavorPart::ResetGradientOffset()
{
    _gradientOffset.reset();
    StylingManager::markChanged();
}

const std::optional<f32>& RetroFuturaGUI::FlavorPart::GetGradientOffset() const
{
    return _gradientOffset;
}

void RetroFuturaGUI::FlavorPart::SetGradientAnimationSpeed(const f32 gradientAnimationSpeed)
{
    _gradientAnimationSpeed = gradientAnimationSpeed;
    StylingManager::markChanged();
}

void RetroFuturaGUI::FlavorPart::ResetGradientAnimationSpeed()
{
    _gradientAnimationSpeed.reset();
    StylingManager::markChanged();
}

const std::optional<f32>& RetroFuturaGUI::FlavorPart::GetGradientAnimationSpeed() const
{
    return _gradientAnimationSpeed;
}

void RetroFuturaGUI::FlavorPart::SetGradientRotationSpeed(const f32 gradientRotationSpeed)
{
    _gradientRotationSpeed = gradientRotationSpeed;
    StylingManager::markChanged();
}

void RetroFuturaGUI::FlavorPart::ResetGradientRotationSpeed()
{
    _gradientRotationSpeed.reset();
    StylingManager::markChanged();
}

const std::optional<f32>& RetroFuturaGUI::FlavorPart::GetGradientRotationSpeed() const
{
    return _gradientRotationSpeed;
}

RetroFuturaGUI::FlavorPart& RetroFuturaGUI::Flavor::GetBackground()
{
    return _background;
}

const RetroFuturaGUI::FlavorPart& RetroFuturaGUI::Flavor::GetBackground() const
{
    return _background;
}

RetroFuturaGUI::FlavorPart& RetroFuturaGUI::Flavor::GetBorder()
{
    return _border;
}

const RetroFuturaGUI::FlavorPart& RetroFuturaGUI::Flavor::GetBorder() const
{
    return _border;
}

RetroFuturaGUI::FlavorPart& RetroFuturaGUI::Flavor::GetText()
{
    return _text;
}

const RetroFuturaGUI::FlavorPart& RetroFuturaGUI::Flavor::GetText() const
{
    return _text;
}

const RetroFuturaGUI::FlavorPart& RetroFuturaGUI::Flavor::GetPart(const StylePart part) const
{
    switch(part)
    {
        case StylePart::Border:
            return _border;
        case StylePart::Text:
            return _text;
        default: //Background
            return _background;
    }
}