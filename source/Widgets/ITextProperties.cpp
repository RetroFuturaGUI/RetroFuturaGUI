#include "ITextProperties.hpp"
#include "InputManager.hpp"

void RetroFuturaGUI::ITextProperties::SetTextColor(const glm::vec4& color, const ColorState state)
{
    switch(state)
    {
        case ColorState::Clicked:
            _textColorClicked.resize(1, color);
        break;
        case ColorState::Disabled:
            _textColorDisabled.resize(1, color);
        break;
        case ColorState::Hover:
            _textColorHover.resize(1, color);
        break;
        default: // Enabled
            _textColorEnabled.resize(1, color);
    }

    setTextColors();
}

std::vector<glm::vec4> RetroFuturaGUI::ITextProperties::GetTextColor(const ColorState state) const
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

void RetroFuturaGUI::ITextProperties::SetText(std::string_view text)
{
    if(_text)
        _text->SetTextUTF8(text);

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
    if(_text)
        _text->SetFontFamily(fontFamily, fontSize, slant, fontWeight);
}

void RetroFuturaGUI::ITextProperties::SetTextAlignment(const TextAlignment alignment)
{
    if(_text)
        _text->SetTextAlignment(alignment);
}
        
void RetroFuturaGUI::ITextProperties::SetTextPadding(const f32 padding)
{
    if(_text)
        _text->SetTextPadding(padding);
}

void RetroFuturaGUI::ITextProperties::setTextColorElement(std::vector<glm::vec4>& color)
{
    if(_text) 
        _text->SetColor(color.front());
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
    if(_text)
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