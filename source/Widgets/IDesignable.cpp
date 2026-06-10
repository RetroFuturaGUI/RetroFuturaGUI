#include "IDesignable.hpp"

void RetroFuturaGUI::IDesignable::SetBackgroundColor(const glm::vec4& color, const ColorState state)
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

    setborderBackgroundColors();
}

void RetroFuturaGUI::IDesignable::SetBackgroundColors(std::span<glm::vec4> colors, const ColorState state)
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

    setborderBackgroundColors();
}

void RetroFuturaGUI::IDesignable::SetBackgroundGradientOffset(const f32 gradientOffset)
{
    if(_rectangle)
        _rectangle->SetGradientOffset(gradientOffset);
}

void RetroFuturaGUI::IDesignable::SetBackgroundGradientAnimationSpeed(const f32 animationSpeed)
{
    if(_rectangle)
        _rectangle->SetGradientAnimationSpeed(animationSpeed);
}

void RetroFuturaGUI::IDesignable::SetBackgroundGradientDegree(const f32 degree)
{
    if(_rectangle)
        _rectangle->SetGradientDegree(degree);
}

void RetroFuturaGUI::IDesignable::SetBackgroundGradientRotationSpeed(const f32 rotationSpeed)
{    
    if(_rectangle)
        _rectangle->SetGradientRotationSpeed(rotationSpeed);
}

const std::vector<glm::vec4>& RetroFuturaGUI::IDesignable::GetBackgroundColors(const ColorState state) const
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

void RetroFuturaGUI::IDesignable::SetBackgroundFillType(const FillType fillType)
{
    _backgroundFillType = fillType;

    if(_rectangle)
        _rectangle->SetFillType(fillType);
}

void RetroFuturaGUI::IDesignable::SetWindowBackgroundImageTextureID(const u32 textureID)
{
    if(_rectangle)
        _rectangle->SetWindowBackgroundImageTextureID(textureID);
}

void RetroFuturaGUI::IDesignable::SetCornerRadii(const glm::vec4& radii)
{
    if(_rectangle)
        _rectangle->SetCornerRadii(radii);

    if(_border)    
        _border->SetCornerRadii(radii);
}

void RetroFuturaGUI::IDesignable::setBackgroundColorElement(std::vector<glm::vec4>& color)
{
    if(_rectangle) 
        _rectangle->SetColor(color);
}

void RetroFuturaGUI::IDesignable::setBorderColorElement(std::vector<glm::vec4>& color)
{
    if(_border) 
        _border->SetColor(color);
}

void RetroFuturaGUI::IDesignable::setborderBackgroundColors()
{
    switch(_borderBackgroundColorState)
    {
        case ColorState::Enabled:
        {
            setBackgroundColorElement(_backgroundColorEnabled);
            setBorderColorElement(_borderColorEnabled);
        } break;
        case ColorState::Clicked:
        {
            setBackgroundColorElement(_backgroundColorClicked);
            setBorderColorElement(_borderColorClicked);
        } break;
        case ColorState::Hover:
        {
            setBackgroundColorElement(_backgroundColorHover);
            setBorderColorElement(_borderColorHover);
        } break;
        default: //Disabled
        {
            setBackgroundColorElement(_backgroundColorDisabled);
            setBorderColorElement(_borderColorDisabled);
        }
    }
}

void RetroFuturaGUI::IDesignable::SetBorderColor(const glm::vec4 & color, const ColorState state)
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

    setborderBackgroundColors();
}

void RetroFuturaGUI::IDesignable::SetBorderColors(std::span<glm::vec4> colors, const ColorState state)
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

    setborderBackgroundColors();
}

const std::vector<glm::vec4>& RetroFuturaGUI::IDesignable::GetBorderColor(const ColorState state) const
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

void RetroFuturaGUI::IDesignable::SetBorderFillType(const FillType fillType)
{
    _borderFillType = fillType;

    if(_border)
        _border->SetFillType(fillType);
}

void RetroFuturaGUI::IDesignable::SetBorderGradientOffset(const f32 gradientOffset)
{
    if(_border) 
        _border->SetGradientOffset(gradientOffset);
}

void RetroFuturaGUI::IDesignable::SetBorderGradientAnimationSpeed(const f32 animationSpeed)
{
    if(_border) 
        _border->SetGradientAnimationSpeed(animationSpeed);
}

void RetroFuturaGUI::IDesignable::SetBorderGradientDegree(const f32 degree)
{
    if(_border) 
        _border->SetGradientDegree(degree);
}

void RetroFuturaGUI::IDesignable::SetBorderGradientRotationSpeed(const f32 rotationSpeed)
{
    if(_border) 
        _border->SetGradientRotationSpeed(rotationSpeed);
}

void RetroFuturaGUI::IDesignable::SetWindowBorderImageTextureID(const u32 textureID)
{
    if(_border)
        _border->SetWindowBackgroundImageTextureID(textureID);
}

void RetroFuturaGUI::IDesignable::drawBackgroundBorder()
{
    if(_rectangle)
        _rectangle->Draw();

    if(_border)
        _border->Draw();
}