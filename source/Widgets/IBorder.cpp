#include "IBorder.hpp"

void RetroFuturaGUI::IBorder::SetBorderColor(const glm::vec4 & color, const ColorState state)
{
        switch(state)
    {
        case ColorState::Clicked:
            _borderColorClicked.clear();
            _borderColorClicked.resize(1, color);
        break;
        case ColorState::Disabled:
            _borderColorDisabled.clear();
            _borderColorDisabled.resize(1, color);
        break;
        case ColorState::Hover:
            _borderColorClicked.clear();
            _borderColorClicked.resize(1, color);
        break;
        default: // Enabled
            _borderColorEnabled.clear();
            _borderColorEnabled.resize(1, color);
    }

    setBorderColors();
}

void RetroFuturaGUI::IBorder::SetBorderColors(std::span<glm::vec4> colors, const ColorState state)
{
        switch(state)
    {
        case ColorState::Clicked:
            _borderColorClicked.assign(colors.begin(), colors.end());
        break;
        case ColorState::Disabled:
            _borderColorDisabled.assign(colors.begin(), colors.end());
        break;
        case ColorState::Hover:
            _borderColorHover.assign(colors.begin(), colors.end());
        break;
        default: // Enabled
            _borderColorEnabled.assign(colors.begin(), colors.end());
    }

    setBorderColors();
}

const std::vector<glm::vec4>& RetroFuturaGUI::IBorder::GetBorderColor(const ColorState state) const
{
    switch(state)
    {
        case ColorState::Clicked:
            return _borderColorClicked;
        case ColorState::Disabled:
            return _borderColorDisabled;
        case ColorState::Hover:
            return _borderColorHover;
        default: // Enabled
            return _borderColorEnabled;
    }
}

void RetroFuturaGUI::IBorder::SetBorderFillType(const FillType fillType)
{
    _borderFillType = fillType;

    if(_border)
        _border->SetFillType(fillType);
}

void RetroFuturaGUI::IBorder::SetBorderGradientOffset(const f32 gradientOffset)
{
    if(_border) 
        _border->SetGradientOffset(gradientOffset);
}

void RetroFuturaGUI::IBorder::SetBorderGradientAnimationSpeed(const f32 animationSpeed)
{
    if(_border) 
        _border->SetGradientAnimationSpeed(animationSpeed);
}

void RetroFuturaGUI::IBorder::SetBorderGradientDegree(const f32 degree)
{
    if(_border) 
        _border->SetGradientDegree(degree);
}

void RetroFuturaGUI::IBorder::SetBorderGradientRotationSpeed(const f32 rotationSpeed)
{
    if(_border) 
        _border->SetGradientRotationSpeed(rotationSpeed);
}

void RetroFuturaGUI::IBorder::SetWindowBorderImageTextureID(const u32 textureID)
{
    if(_border)
        _border->SetWindowBackgroundImageTextureID(textureID);
}

void RetroFuturaGUI::IBorder::drawBorder()
{
    if(_border)
        _border->Draw();
}

void RetroFuturaGUI::IBorder::setBorderColors()
{
    switch(_borderColorState)
    {
        case ColorState::Enabled:
            setBorderColorElement(_borderColorEnabled);
        break;
        case ColorState::Clicked:
            setBorderColorElement(_borderColorClicked);
        break;
        case ColorState::Hover:
            setBorderColorElement(_borderColorHover);
        break;
        default: //Disabled
            setBorderColorElement(_borderColorDisabled);
    }
}

void RetroFuturaGUI::IBorder::setBorderColorElement(std::vector<glm::vec4>& color)
{
    if(_border) 
        _border->SetColors(color);
}

void RetroFuturaGUI::IBorder::SetBorderWidth(const f32 borderWidth)
{
    if(_border)    
        _border->SetBorderWidth(borderWidth);
}

void RetroFuturaGUI::IBorder::setBorderCornerRadii(const glm::vec4& radii)
{
    if(_border)    
        _border->SetCornerRadii(radii);
}