#include "ITableTextProperties.hpp"

 const std::string& RetroFuturaGUI::ITableTextProperties::GetText() const
{
    if(!_text)
        return _dummyText;

    return _text->GetTextUTF8();
}

void RetroFuturaGUI::ITableTextProperties::SetFontFamily(std::string_view fontFamily, const f32 fontSize, const PlatformBridge::Fonts::Slant slant, const PlatformBridge::Fonts::Weight fontWeight)
{
    if(_text)
        _text->SetFontFamily(fontFamily, fontSize, slant, fontWeight);
}

void RetroFuturaGUI::ITableTextProperties::SetTextAlignment(const TextAlignment alignment)
{
    _textAlignment = alignment;

    if(_text)
        _text->SetTextAlignment(alignment);
}

void RetroFuturaGUI::ITableTextProperties::SetTextPadding(const f32 padding)
{
    if(_text)
        _text->SetTextPadding(padding);
}

f32 RetroFuturaGUI::ITableTextProperties::GetTextWidth() const
{
    return _text ? _text->GetWidth() : 0.0f;
}

void RetroFuturaGUI::ITableTextProperties::setTextColors()
{
    if(!_text)
        return;

    switch(_textColorState)
    {
        case ColorState::Enabled:
            _text->SetColor(_textColorEnabled.front());
            break;
        case ColorState::Clicked:
            _text->SetColor(_textColorClicked.front());
            break;
        case ColorState::Hover:
            _text->SetColor(_textColorHover.front());
            break;
        default: //Disabled
            _text->SetColor(_textColorDisabled.front());
    }
}