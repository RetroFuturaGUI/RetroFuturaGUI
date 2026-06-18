#include "IBackground.hpp"

void RetroFuturaGUI::IBackground::SetBackgroundColor(const glm::vec4& color, const ColorState state)
{
    switch(state)
    {
        case ColorState::Clicked:
            _backgroundColorClicked.clear();
            _backgroundColorClicked.resize(1, color);
        break;
        case ColorState::Disabled:
            _backgroundColorDisabled.clear();
            _backgroundColorDisabled.resize(1, color);
        break;
        case ColorState::Hover:
            _backgroundColorHover.clear();
            _backgroundColorHover.resize(1, color);
        break;
        default: // Enabled
            _backgroundColorEnabled.clear();
            _backgroundColorEnabled.resize(1, color);
    }

    setBackgroundColors();
}

void RetroFuturaGUI::IBackground::SetBackgroundColors(std::span<glm::vec4> colors, const ColorState state)
{
    switch(state)
    {
        case ColorState::Clicked:
            _backgroundColorClicked.assign(colors.begin(), colors.end());
        break;
        case ColorState::Disabled:
            _backgroundColorDisabled.assign(colors.begin(), colors.end());
        break;
        case ColorState::Hover:
            _backgroundColorHover.assign(colors.begin(), colors.end());
        break;
        default: // Enabled
            _backgroundColorEnabled.assign(colors.begin(), colors.end());
    }

    setBackgroundColors();
}

void RetroFuturaGUI::IBackground::SetBackgroundGradientOffset(const f32 gradientOffset)
{
    if(_background)
        _background->SetGradientOffset(gradientOffset);
}

void RetroFuturaGUI::IBackground::SetBackgroundGradientAnimationSpeed(const f32 animationSpeed)
{
    if(_background)
        _background->SetGradientAnimationSpeed(animationSpeed);
}

void RetroFuturaGUI::IBackground::SetBackgroundGradientDegree(const f32 degree)
{
    if(_background)
        _background->SetGradientDegree(degree);
}

void RetroFuturaGUI::IBackground::SetBackgroundGradientRotationSpeed(const f32 rotationSpeed)
{    
    if(_background)
        _background->SetGradientRotationSpeed(rotationSpeed);
}

const std::vector<glm::vec4>& RetroFuturaGUI::IBackground::GetBackgroundColors(const ColorState state) const
{
    switch(state)
    {
        case ColorState::Clicked:
            return _backgroundColorClicked;
        case ColorState::Disabled:
            return _backgroundColorDisabled;
        case ColorState::Hover:
            return _backgroundColorHover;
        default: // Enabled
            return _backgroundColorEnabled;
    }
}

void RetroFuturaGUI::IBackground::SetBackgroundFillType(const FillType fillType)
{
    _backgroundFillType = fillType;

    if(_background)
        _background->SetFillType(fillType);
}

void RetroFuturaGUI::IBackground::SetWindowBackgroundImageTextureID(const u32 textureID)
{
    if(_background)
        _background->SetWindowBackgroundImageTextureID(textureID);
}

void RetroFuturaGUI::IBackground::setBackgroundCornerRadii(const glm::vec4& radii)
{
    if(_background)
        _background->SetCornerRadii(radii);
}

void RetroFuturaGUI::IBackground::setBackgroundColorElement(std::vector<glm::vec4>& color)
{
    if(_background) 
        _background->SetColors(color);
}

void RetroFuturaGUI::IBackground::setBackgroundColors()
{
    switch(_backgroundColorState)
    {
        case ColorState::Enabled:
            setBackgroundColorElement(_backgroundColorEnabled);
        break;
        case ColorState::Clicked:
            setBackgroundColorElement(_backgroundColorClicked);
        break;
        case ColorState::Hover:
            setBackgroundColorElement(_backgroundColorHover);
        break;
        default: //Disabled
            setBackgroundColorElement(_backgroundColorDisabled);
    }
}

void RetroFuturaGUI::IBackground::drawBackground()
{
    if(_background)
        _background->Draw();
}