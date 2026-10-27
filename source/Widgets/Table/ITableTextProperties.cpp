#include "ITableTextProperties.hpp"

 const std::string& RetroFuturaGUI::ITableTextProperties::GetText() const
{
    if(!_text)
        return _content;

    return _text->GetTextUTF8();
}

void RetroFuturaGUI::ITableTextProperties::SetFontFamily(std::string_view fontFamily, const f32 fontSize, const PlatformBridge::Fonts::Slant slant, const PlatformBridge::Fonts::Weight fontWeight)
{
    _fontFamily = fontFamily;
    _fontSize = fontSize;
    _slant = slant;
    _fontWeight = fontWeight;

    if(!_text)
        return;

    _text->SetFontFamily(fontFamily, fontSize, slant, fontWeight);
}

void RetroFuturaGUI::ITableTextProperties::SetTextAlignment(const TextAlignment alignment)
{
    _textAlignment = alignment;

    if(!_text)
        return;

    _text->SetTextAlignment(alignment);
}

void RetroFuturaGUI::ITableTextProperties::SetTextPadding(const f32 padding)
{
    _textPadding = padding;

    if(!_text)
        return;

    _text->SetTextPadding(padding);
}

f32 RetroFuturaGUI::ITableTextProperties::GetTextWidth() const
{
    return _text ? _text->GetWidth() : 0.0f;
}

void RetroFuturaGUI::ITableTextProperties::applyTextSettings()
{
    if(!_text)
        return;

    if(!_fontFamily.empty())
        _text->SetFontFamily(_fontFamily, _fontSize, _slant, _fontWeight);

    _text->SetTextAlignment(_textAlignment);
    _text->SetTextPadding(_textPadding);
    setTextColors();
}

void RetroFuturaGUI::ITableTextProperties::setTextColors()
{
    if(!_text)
        return;

    switch(_textColorState)
    {
        case ColorState::Enabled:
            if(!_textColorEnabled.empty())
                _text->SetColors(_textColorEnabled);
            break;
        case ColorState::Clicked:
            if(!_textColorClicked.empty())
                _text->SetColors(_textColorClicked);
            break;
        case ColorState::Hover:
            if(!_textColorHover.empty())
                _text->SetColors(_textColorHover);
            break;
        default: //Disabled
            if(!_textColorDisabled.empty())
                _text->SetColors(_textColorDisabled);
    }
}