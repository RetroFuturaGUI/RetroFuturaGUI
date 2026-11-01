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

void RetroFuturaGUI::ITextProperties::SetTextBackgroundImageFit(const ImageFit fit)
{
    if(!_text)
        return;

    _text->SetBackgroundImageFit(fit);
}

void RetroFuturaGUI::ITextProperties::SetTextMaskTextureID(const u32 textureID)
{
    if(!_text)
        return;

    _text->SetMaskTextureID(textureID);
}

void RetroFuturaGUI::ITextProperties::SetTextMaskMode(const MaskMode mode)
{
    if(!_text)
        return;

    _text->SetMaskMode(mode);
}

void RetroFuturaGUI::ITextProperties::SetTextMaskFit(const ImageFit fit)
{
    if(!_text)
        return;

    _text->SetMaskFit(fit);
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

void RetroFuturaGUI::ITextProperties::SetTextColorGlyphTint(const f32 tint)
{
    if(!_text)
        return;

    _text->SetColorGlyphTint(tint);
}

void RetroFuturaGUI::ITextProperties::SetTextBorderWidth(const f32 width)
{
    if(!_text)
        return;

    _text->SetBorderWidth(width);
}

void RetroFuturaGUI::ITextProperties::SetTextBorderColors(std::span<const glm::vec4> colors, const ColorState state)
{
    switch(state)
    {
        case ColorState::Clicked:
            _textBorderColorClicked.assign(colors.begin(), colors.end());
        break;
        case ColorState::Disabled:
            _textBorderColorDisabled.assign(colors.begin(), colors.end());
        break;
        case ColorState::Hover:
            _textBorderColorHover.assign(colors.begin(), colors.end());
        break;
        default: // Enabled
            _textBorderColorEnabled.assign(colors.begin(), colors.end());
    }

    setTextColors();
}

const std::vector<glm::vec4>& RetroFuturaGUI::ITextProperties::GetTextBorderColors(const ColorState state) const
{
    switch(state)
    {
        case ColorState::Clicked:
            return _textBorderColorClicked;
        case ColorState::Disabled:
            return _textBorderColorDisabled;
        case ColorState::Hover:
            return _textBorderColorHover;
        default: // Enabled
            return _textBorderColorEnabled;
    }
}

void RetroFuturaGUI::ITextProperties::SetTextBorderFillType(const FillType fillType)
{
    if(!_text)
        return;

    _text->GetBorder().SetFillType(fillType);
}

void RetroFuturaGUI::ITextProperties::SetTextBorderGradientOffset(const f32 gradientOffset)
{
    if(!_text)
        return;

    _text->GetBorder().SetGradientOffset(gradientOffset);
}

void RetroFuturaGUI::ITextProperties::SetTextBorderGradientAnimationSpeed(const f32 animationSpeed)
{
    if(!_text)
        return;

    _text->GetBorder().SetGradientAnimationSpeed(animationSpeed);
}

void RetroFuturaGUI::ITextProperties::SetTextBorderGradientDegree(const f32 degree)
{
    if(!_text)
        return;

    _text->GetBorder().SetGradientDegree(degree);
}

void RetroFuturaGUI::ITextProperties::SetTextBorderGradientRotationSpeed(const f32 rotationSpeed)
{
    if(!_text)
        return;

    _text->GetBorder().SetGradientRotationSpeed(rotationSpeed);
}

void RetroFuturaGUI::ITextProperties::SetTextBorderBackgroundImageTextureID(const u32 textureID)
{
    if(!_text)
        return;

    _text->GetBorder().SetBackgroundImageTextureID(textureID);
}

void RetroFuturaGUI::ITextProperties::SetTextBorderBackgroundImagePadding(const f32 padding)
{
    if(!_text)
        return;

    _text->GetBorder().SetBackgroundImagePadding(padding);
}

void RetroFuturaGUI::ITextProperties::SetTextBorderBackgroundImageFit(const ImageFit fit)
{
    if(!_text)
        return;

    _text->GetBorder().SetBackgroundImageFit(fit);
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

void RetroFuturaGUI::ITextProperties::SetFontFamily(std::string_view fontFamily, const f32 fontSize, const FontSizeUnit sizeUnit, const PlatformBridge::Fonts::Slant slant, const PlatformBridge::Fonts::Weight fontWeight)
{
    if(!_text)
        return;

    _text->SetFontFamily(fontFamily, fontSize, sizeUnit, slant, fontWeight);
}

f32 RetroFuturaGUI::ITextProperties::GetFontSize(const FontSizeUnit sizeUnit) const
{
    if(!_text)
        return 0.0f;

    return _text->GetFontSize(sizeUnit);
}

void RetroFuturaGUI::ITextProperties::SetTextAlignment(const TextAlignment alignment)
{
    if(!_text)
        return;

    _text->SetTextAlignment(alignment);
}

void RetroFuturaGUI::ITextProperties::SetTextFlow(const TextFlow textFlow)
{
    if(!_text)
        return;

    _text->SetTextFlow(textFlow);
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

void RetroFuturaGUI::ITextProperties::setTextColorElements(std::vector<glm::vec4>& color, std::vector<glm::vec4>& borderColor)
{
    if(!_text)
        return;

    _text->SetColors(color);
    _text->GetBorder().SetColors(borderColor);
}

void RetroFuturaGUI::ITextProperties::setTextColors()
{
    switch(_textColorState)
    {
        case ColorState::Enabled:
        {
            setTextColorElements(_textColorEnabled, _textBorderColorEnabled);
        } break;
        case ColorState::Clicked:
        {
            setTextColorElements(_textColorClicked, _textBorderColorClicked);
        } break;
        case ColorState::Hover:
        {
            setTextColorElements(_textColorHover, _textBorderColorHover);
        } break;
        default: //Disabled
        {
            setTextColorElements(_textColorDisabled, _textBorderColorDisabled);
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