#include "Outline.hpp"

RetroFuturaGUI::Outline::Outline(Text& text)
    : _text(text)
{
}

void RetroFuturaGUI::Outline::SetWidth(const f32 width)
{
    _text.SetOutlineWidth(width);
}

f32 RetroFuturaGUI::Outline::GetWidth() const
{
    return _text.GetOutlineWidth();
}

void RetroFuturaGUI::Outline::SetColors(std::span<const glm::vec4> colors, const ColorState state)
{
    colorsOf(state).assign(colors.begin(), colors.end());
    SetColorState(_colorState);
}

const std::vector<glm::vec4>& RetroFuturaGUI::Outline::GetColors(const ColorState state) const
{
    return colorsOf(state);
}

void RetroFuturaGUI::Outline::SetFillType(const FillType fillType)
{
    _text.GetOutline().SetFillType(fillType);
}

void RetroFuturaGUI::Outline::SetGradientOffset(const f32 gradientOffset)
{
    _text.GetOutline().SetGradientOffset(gradientOffset);
}

void RetroFuturaGUI::Outline::SetGradientAnimationSpeed(const f32 animationSpeed)
{
    _text.GetOutline().SetGradientAnimationSpeed(animationSpeed);
}

void RetroFuturaGUI::Outline::SetGradientDegree(const f32 degree)
{
    _text.GetOutline().SetGradientDegree(degree);
}

void RetroFuturaGUI::Outline::SetGradientRotationSpeed(const f32 rotationSpeed)
{
    _text.GetOutline().SetGradientRotationSpeed(rotationSpeed);
}

void RetroFuturaGUI::Outline::SetImageTextureID(const u32 textureID)
{
    _text.GetOutline().SetBackgroundImageTextureID(textureID);
}

void RetroFuturaGUI::Outline::SetImagePadding(const f32 padding)
{
    _text.GetOutline().SetBackgroundImagePadding(padding);
}

void RetroFuturaGUI::Outline::SetImageFit(const ImageFit fit)
{
    _text.GetOutline().SetBackgroundImageFit(fit);
}

void RetroFuturaGUI::Outline::SetColorState(const ColorState state)
{
    _colorState = state;
    _text.GetOutline().SetColors(shownColorsOf(state));
}

std::vector<glm::vec4>& RetroFuturaGUI::Outline::colorsOf(const ColorState state)
{
    switch(state)
    {
        case ColorState::Clicked:
            return _colorsClicked;
        case ColorState::Disabled:
            return _colorsDisabled;
        case ColorState::Hover:
            return _colorsHover;
        case ColorState::Selected:
            return _colorsSelected;
        default: // Enabled
            return _colorsEnabled;
    }
}

const std::vector<glm::vec4>& RetroFuturaGUI::Outline::colorsOf(const ColorState state) const
{
    switch(state)
    {
        case ColorState::Clicked:
            return _colorsClicked;
        case ColorState::Disabled:
            return _colorsDisabled;
        case ColorState::Hover:
            return _colorsHover;
        case ColorState::Selected:
            return _colorsSelected;
        default: // Enabled
            return _colorsEnabled;
    }
}

const std::vector<glm::vec4>& RetroFuturaGUI::Outline::shownColorsOf(const ColorState state) const
{
    if(ColorState::Selected == state && _colorsSelected.empty())
        return _colorsEnabled;

    return colorsOf(state);
}