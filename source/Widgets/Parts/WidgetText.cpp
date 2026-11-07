#include "WidgetText.hpp"

RetroFuturaGUI::WidgetText::WidgetText(Projection* projection)
    : _text(projection), _outline(_text)
{
}

void RetroFuturaGUI::WidgetText::SetString(std::string_view string, const bool emitSignal)
{
    _text.SetTextUTF8(string);
    notifyChange(WidgetTextChange::String);

    if(!emitSignal)
        return;

    EmitChange();
}

const std::string& RetroFuturaGUI::WidgetText::GetString() const
{
    return _text.GetTextUTF8();
}

const char* RetroFuturaGUI::WidgetText::GetCString() const
{
    return _text.GetTextUTF8().c_str();
}

void RetroFuturaGUI::WidgetText::Connect_OnTextChange(const typename Signal<>::Slot& slot, const bool async)
{
    if(async)
        _onTextChangeAsync.Connect(slot);
    else
        _onTextChange.Connect(slot);
}

void RetroFuturaGUI::WidgetText::Disconnect_OnTextChange(const typename Signal<>::Slot& slot)
{
    _onTextChange.Disconnect(slot);
    _onTextChangeAsync.Disconnect(slot);
}

void RetroFuturaGUI::WidgetText::SetFontFamily(std::string_view fontFamily, const f32 fontSize, const FontSizeUnit sizeUnit, const PlatformBridge::Fonts::Slant slant, const PlatformBridge::Fonts::Weight fontWeight)
{
    _text.SetFontFamily(fontFamily, fontSize, sizeUnit, slant, fontWeight);
    notifyChange(WidgetTextChange::Font);

    if(!_follower)
        return;

    _follower->SetFontFamily(fontFamily, fontSize, sizeUnit, slant, fontWeight);
}

f32 RetroFuturaGUI::WidgetText::GetFontSize(const FontSizeUnit sizeUnit) const
{
    return _text.GetFontSize(sizeUnit);
}

void RetroFuturaGUI::WidgetText::SetAlignment(const TextAlignment alignment)
{
    _text.SetTextAlignment(alignment);

    if(!_follower)
        return;

    _follower->SetAlignment(alignment);
}

void RetroFuturaGUI::WidgetText::SetFlow(const TextFlow textFlow)
{
    _text.SetTextFlow(textFlow);
}

void RetroFuturaGUI::WidgetText::SetPadding(const f32 padding)
{
    _text.SetTextPadding(padding);
    notifyChange(WidgetTextChange::Padding);

    if(!_follower)
        return;

    _follower->SetPadding(padding);
}

f32 RetroFuturaGUI::WidgetText::GetPadding() const
{
    return _text.GetPadding();
}

f32 RetroFuturaGUI::WidgetText::GetWidth() const
{
    return _text.GetWidth();
}

void RetroFuturaGUI::WidgetText::SetColors(std::span<const glm::vec4> colors, const ColorState state)
{
    colorsOf(_colors, state).assign(colors.begin(), colors.end());
    setColors();
}

const std::vector<glm::vec4>& RetroFuturaGUI::WidgetText::GetColors(const ColorState state) const
{
    return colorsOf(_colors, state);
}

void RetroFuturaGUI::WidgetText::SetFillType(const FillType fillType)
{
    _text.SetFillType(fillType);
}

void RetroFuturaGUI::WidgetText::SetGradientOffset(const f32 gradientOffset)
{
    _text.SetGradientOffset(gradientOffset);
}

void RetroFuturaGUI::WidgetText::SetGradientAnimationSpeed(const f32 animationSpeed)
{
    _text.SetGradientAnimationSpeed(animationSpeed);
}

void RetroFuturaGUI::WidgetText::SetGradientDegree(const f32 degree)
{
    _text.SetGradientDegree(degree);
}

void RetroFuturaGUI::WidgetText::SetGradientRotationSpeed(const f32 rotationSpeed)
{
    _text.SetGradientRotationSpeed(rotationSpeed);
}

void RetroFuturaGUI::WidgetText::SetColorGlyphTint(const f32 tint)
{
    _text.SetColorGlyphTint(tint);
}

void RetroFuturaGUI::WidgetText::SetWindowBackgroundImageTextureID(const u32 textureID)
{
    _text.SetWindowBackgroundImageTextureID(textureID);
}

void RetroFuturaGUI::WidgetText::SetImageTextureID(const u32 textureID)
{
    _text.SetBackgroundImageTextureID(textureID);
}

void RetroFuturaGUI::WidgetText::SetImagePadding(const f32 padding)
{
    _text.SetBackgroundImagePadding(padding);
}

void RetroFuturaGUI::WidgetText::SetImageFit(const ImageFit fit)
{
    _text.SetBackgroundImageFit(fit);
}

void RetroFuturaGUI::WidgetText::SetMaskTextureID(const u32 textureID)
{
    _text.SetMaskTextureID(textureID);
}

void RetroFuturaGUI::WidgetText::SetMaskMode(const MaskMode mode)
{
    _text.SetMaskMode(mode);
}

void RetroFuturaGUI::WidgetText::SetMaskFit(const ImageFit fit)
{
    _text.SetMaskFit(fit);
}

void RetroFuturaGUI::WidgetText::SetMaskSpan(const MaskSpan maskSpan)
{
    _text.SetMaskSpan(maskSpan);
}

void RetroFuturaGUI::WidgetText::SetRasterPattern(const RasterPattern rasterPattern)
{
    _text.SetRasterPattern(rasterPattern);
}

void RetroFuturaGUI::WidgetText::SetPrimaryRasterColor(const glm::vec4& color)
{
    _text.SetPrimaryRasterColor(color);
}

void RetroFuturaGUI::WidgetText::SetSecondaryRasterColor(const glm::vec4& color)
{
    _text.SetSecondaryRasterColor(color);
}

void RetroFuturaGUI::WidgetText::SetRasterSpacing(const f32 spacing)
{
    _text.SetRasterSpacing(spacing);
}

void RetroFuturaGUI::WidgetText::SetRasterDegree(const f32 degree)
{
    _text.SetRasterDegree(degree);
}

void RetroFuturaGUI::WidgetText::SetPrimaryRasterWidthTransfer(std::span<const f32> widthTransfer)
{
    _primaryRasterWidthTransfer.assign(widthTransfer.begin(), widthTransfer.end());
    _text.SetPrimaryRasterWidthTransfer(_primaryRasterWidthTransfer);
}

void RetroFuturaGUI::WidgetText::SetDotTransparencyTransfer(const f32 transparencyTransfer)
{
    _text.SetDotTransparencyTransfer(transparencyTransfer);
}

void RetroFuturaGUI::WidgetText::SetRasterAnimationSpeed(const f32 animationSpeed)
{
    _text.SetRasterAnimationSpeed(animationSpeed);
}

void RetroFuturaGUI::WidgetText::SetFogAlpha(const f32 alpha)
{
    _text.SetFogAlpha(alpha);
}

void RetroFuturaGUI::WidgetText::SetFogSpeed(const f32 speed)
{
    _text.SetFogSpeed(speed);
}

void RetroFuturaGUI::WidgetText::SetFogDensity(std::span<const f32> density)
{
    _fogDensity.assign(density.begin(), density.end());
    _text.SetFogDensity(_fogDensity);
}

void RetroFuturaGUI::WidgetText::SetFogClearing(const f32 clearing)
{
    _text.SetFogClearing(clearing);
}

void RetroFuturaGUI::WidgetText::Draw()
{
    _text.Draw();
}

void RetroFuturaGUI::WidgetText::SetSize(const glm::vec2& size)
{
    _text.SetParentSize(size);
}

void RetroFuturaGUI::WidgetText::SetPosition(const glm::vec3& position)
{
    _text.SetPosition(position);
}

void RetroFuturaGUI::WidgetText::SetColorState(const ColorState state)
{
    _colorState = state;
    setColors();
}

void RetroFuturaGUI::WidgetText::SetTransitionSettings(const TransitionSettings* settings)
{
    _text.SetTransitionSettings(settings);
}

void RetroFuturaGUI::WidgetText::SetChangeHook(const std::function<void(WidgetTextChange)>& hook)
{
    _changeHook = hook;
}

void RetroFuturaGUI::WidgetText::SetFollower(WidgetText* follower)
{
    _follower = follower;
}

void RetroFuturaGUI::WidgetText::EmitChange()
{
    _onTextChangeAsync.EmitAsync();
    _onTextChange.Emit();
}

RetroFuturaGUI::Outline& RetroFuturaGUI::WidgetText::GetOutline()
{
    return _outline;
}

const RetroFuturaGUI::Outline& RetroFuturaGUI::WidgetText::GetOutline() const
{
    return _outline;
}

RetroFuturaGUI::Text& RetroFuturaGUI::WidgetText::GetElement()
{
    return _text;
}

const RetroFuturaGUI::Text& RetroFuturaGUI::WidgetText::GetElement() const
{
    return _text;
}

std::vector<glm::vec4>& RetroFuturaGUI::WidgetText::colorsOf(StateColors& colors, const ColorState state)
{
    switch(state)
    {
        case ColorState::Clicked:
            return colors._Clicked;
        case ColorState::Disabled:
            return colors._Disabled;
        case ColorState::Hover:
            return colors._Hover;
        case ColorState::Selected:
            return colors._Selected;
        default: // Enabled
            return colors._Enabled;
    }
}

const std::vector<glm::vec4>& RetroFuturaGUI::WidgetText::colorsOf(const StateColors& colors, const ColorState state)
{
    switch(state)
    {
        case ColorState::Clicked:
            return colors._Clicked;
        case ColorState::Disabled:
            return colors._Disabled;
        case ColorState::Hover:
            return colors._Hover;
        case ColorState::Selected:
            return colors._Selected;
        default: // Enabled
            return colors._Enabled;
    }
}

const std::vector<glm::vec4>& RetroFuturaGUI::WidgetText::shownColorsOf(const StateColors& colors, const ColorState state)
{
    if(ColorState::Selected == state && colors._Selected.empty())
        return colors._Enabled;

    return colorsOf(colors, state);
}

void RetroFuturaGUI::WidgetText::setColors()
{
    _text.SetColors(shownColorsOf(_colors, _colorState));
    _outline.SetColorState(_colorState);
}

void RetroFuturaGUI::WidgetText::notifyChange(const WidgetTextChange change)
{
    if(!_changeHook)
        return;

    _changeHook(change);
}