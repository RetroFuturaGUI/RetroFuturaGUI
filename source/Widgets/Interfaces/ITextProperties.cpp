#include "ITextProperties.hpp"

void RetroFuturaGUI::ITextProperties::SetTextColors(std::span<const glm::vec4> colors, const ColorState state)
{
    switch(state)
    {
        case ColorState::Clicked:
            _textColorClicked.assign(colors.begin(), colors.end());
        break;
        case ColorState::Disabled:
            _textColorDisabled.assign(colors.begin(), colors.end());
        break;
        case ColorState::Hover:
            _textColorHover.assign(colors.begin(), colors.end());
        break;
        default: // Enabled
            _textColorEnabled.assign(colors.begin(), colors.end());
    }

    setTextColors();
}

const std::vector<glm::vec4>& RetroFuturaGUI::ITextProperties::GetTextColors(const ColorState state) const
{
    switch(state)
    {
        case ColorState::Clicked:
            return _textColorClicked;
        case ColorState::Disabled:
            return _textColorDisabled;
        case ColorState::Hover:
            return _textColorHover;
        default: // Enabled
            return _textColorEnabled;
    }
}

void RetroFuturaGUI::ITextProperties::SetTextFillType(const FillType fillType)
{
    if(!_text)
        return;

    _text->SetFillType(fillType);
}

void RetroFuturaGUI::ITextProperties::SetTextGradientOffset(const f32 gradientOffset)
{
    if(!_text)
        return;

    _text->SetGradientOffset(gradientOffset);
}

void RetroFuturaGUI::ITextProperties::SetTextGradientAnimationSpeed(const f32 animationSpeed)
{
    if(!_text)
        return;

    _text->SetGradientAnimationSpeed(animationSpeed);
}

void RetroFuturaGUI::ITextProperties::SetTextGradientDegree(const f32 degree)
{
    if(!_text)
        return;

    _text->SetGradientDegree(degree);
}

void RetroFuturaGUI::ITextProperties::SetTextGradientRotationSpeed(const f32 rotationSpeed)
{
    if(!_text)
        return;

    _text->SetGradientRotationSpeed(rotationSpeed);
}

void RetroFuturaGUI::ITextProperties::SetTextWindowBackgroundImageTextureID(const u32 textureID)
{
    if(!_text)
        return;

    _text->SetWindowBackgroundImageTextureID(textureID);
}

void RetroFuturaGUI::ITextProperties::SetTextBackgroundImageTextureID(const u32 textureID)
{
    if(!_text)
        return;

    _text->SetBackgroundImageTextureID(textureID);
}

void RetroFuturaGUI::ITextProperties::SetTextBackgroundImagePadding(const f32 padding)
{
    if(!_text)
        return;

    _text->SetBackgroundImagePadding(padding);
}

void RetroFuturaGUI::ITextProperties::SetTextRasterPattern(const RasterPattern rasterPattern)
{
    if(!_text)
        return;

    _text->SetRasterPattern(rasterPattern);
}

void RetroFuturaGUI::ITextProperties::SetTextPrimaryRasterColor(const glm::vec4& color)
{
    if(!_text)
        return;

    _text->SetPrimaryRasterColor(color);
}

void RetroFuturaGUI::ITextProperties::SetTextSecondaryRasterColor(const glm::vec4& color)
{
    if(!_text)
        return;

    _text->SetSecondaryRasterColor(color);
}

void RetroFuturaGUI::ITextProperties::SetTextRasterSpacing(const f32 spacing)
{
    if(!_text)
        return;

    _text->SetRasterSpacing(spacing);
}

void RetroFuturaGUI::ITextProperties::SetTextRasterDegree(const f32 degree)
{
    if(!_text)
        return;

    _text->SetRasterDegree(degree);
}

void RetroFuturaGUI::ITextProperties::SetTextPrimaryRasterWidthTransfer(std::span<const f32> widthTransfer)
{
    _textPrimaryRasterWidthTransfer.assign(widthTransfer.begin(), widthTransfer.end());

    if(!_text)
        return;

    _text->SetPrimaryRasterWidthTransfer(_textPrimaryRasterWidthTransfer);
}

void RetroFuturaGUI::ITextProperties::SetTextDotTransparencyTransfer(const f32 transparencyTransfer)
{
    if(!_text)
        return;

    _text->SetDotTransparencyTransfer(transparencyTransfer);
}

void RetroFuturaGUI::ITextProperties::SetTextRasterAnimationSpeed(const f32 animationSpeed)
{
    if(!_text)
        return;

    _text->SetRasterAnimationSpeed(animationSpeed);
}

void RetroFuturaGUI::ITextProperties::SetTextFogAlpha(const f32 alpha)
{
    if(!_text)
        return;

    _text->SetFogAlpha(alpha);
}

void RetroFuturaGUI::ITextProperties::SetTextFogSpeed(const f32 speed)
{
    if(!_text)
        return;

    _text->SetFogSpeed(speed);
}

void RetroFuturaGUI::ITextProperties::SetTextFogDensity(std::span<const f32> density)
{
    _textFogDensity.assign(density.begin(), density.end());

    if(!_text)
        return;

    _text->SetFogDensity(_textFogDensity);
}

void RetroFuturaGUI::ITextProperties::SetTextFogClearing(const f32 clearing)
{
    if(!_text)
        return;

    _text->SetFogClearing(clearing);
}

void RetroFuturaGUI::ITextProperties::SetText(std::string_view text, const bool emitSignal)
{
    if(!_text)
        return;

    _text->SetTextUTF8(text);

    if(!emitSignal)
        return;
    
    _onTextChangeAsync.EmitAsync();
    _onTextChange.Emit();
}

const std::string& RetroFuturaGUI::ITextProperties::GetText() const
{
    if(!_text)
        return _dummy;
    
    return _text->GetTextUTF8();
}

void RetroFuturaGUI::ITextProperties::SetFontFamily(std::string_view fontFamily, const f32 fontSize, const PlatformBridge::Fonts::Slant slant, const PlatformBridge::Fonts::Weight fontWeight)
{
    if(!_text)
        return;

    _text->SetFontFamily(fontFamily, fontSize, slant, fontWeight);
}

void RetroFuturaGUI::ITextProperties::SetTextAlignment(const TextAlignment alignment)
{
    if(!_text)
        return;

    _text->SetTextAlignment(alignment);
}
        
void RetroFuturaGUI::ITextProperties::SetTextPadding(const f32 padding)
{
    if(!_text)
        return;

    _text->SetTextPadding(padding);
}

f32 RetroFuturaGUI::ITextProperties::GetTextPadding() const
{
    return _text ? _text->GetPadding() : 0.0f;
}

f32 RetroFuturaGUI::ITextProperties::GetTextWidth() const
{
    return _text ? _text->GetWidth() : 0.0f;
}

void RetroFuturaGUI::ITextProperties::setTextColorElement(std::vector<glm::vec4>& color)
{
    if(!_text)
        return;

    _text->SetColors(color);
}

void RetroFuturaGUI::ITextProperties::setTextColors()
{
    switch(_textColorState)
    {
        case ColorState::Enabled:
        {
            setTextColorElement(_textColorEnabled);
        } break;
        case ColorState::Clicked:
        {
            setTextColorElement(_textColorClicked);
        } break;
        case ColorState::Hover:
        {
            setTextColorElement(_textColorHover);
        } break;
        default: //Disabled
        {
            setTextColorElement(_textColorDisabled);
        }
    }
}

void RetroFuturaGUI::ITextProperties::drawText()
{
    if(!_text)
        return;

    _text->Draw();
}

void RetroFuturaGUI::ITextProperties::Connect_OnTextChange(const typename Signal<>::Slot &slot, const bool async)
{
    if (async)
        _onTextChangeAsync.Connect(slot);
    else
        _onTextChange.Connect(slot);
}

void RetroFuturaGUI::ITextProperties::Disconnect_OnTextChange(const typename Signal<>::Slot &slot)
{
    _onTextChange.Disconnect(slot);
    _onTextChangeAsync.Disconnect(slot);
}