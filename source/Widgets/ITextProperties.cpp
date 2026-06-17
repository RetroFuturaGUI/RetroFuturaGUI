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
        _text->SetText(text);

    _onTextChangeAsync.EmitAsync();
    _onTextChange.Emit();
}

void RetroFuturaGUI::ITextProperties::SetReadOnly(const bool readOnly)
{
    _readOnly = readOnly;
}

bool RetroFuturaGUI::ITextProperties::IsReadOnly() const
{
    return _readOnly;
}

const std::string& RetroFuturaGUI::ITextProperties::GetText() const
{
    return _text->GetText();
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

void RetroFuturaGUI::ITextProperties::editText()
{
    if(!_editingEnabled || !_text)
        return;

    if(_parentWindow != InputManager::GetFocusedWindow())
        return;

    if(PlatformBridge::Keyboard::GetKeyboardUseState() == PlatformBridge::KeyboardUseState::KeyReleased)
    {
        _keyWasReleased = true;
        _keyHoldFrames = 0;
        _keyRepeatText.clear();
        return;
    }

    auto emitChange = [this]() 
    {
        _onTextChangeAsync.EmitAsync();
        _onTextChange.Emit();
    };

    if(!_keyWasReleased)
    {
        if (!_keyRepeatText.empty())
        {
            ++_keyHoldFrames;
            if (_keyHoldFrames >= _keyRepeatInitialDelay && (_keyHoldFrames - _keyRepeatInitialDelay) % _keyRepeatInterval == 0)
            {
                _text->SetText(_text->GetText() + _keyRepeatText);
                emitChange();
            }
        }
        return;
    }

    if(PlatformBridge::Keyboard::GetKeyPressState(PB_KEY_BACKSPACE) == PlatformBridge::KeyPressState::Press)
    {
        if(_text->GetText().size() == 0)
            return;

        uSize numDelete { 1 };
        
        for(uSize count { 0 }; ((u8)_text->GetText()[_text->GetText().size() - 1 - count] & (u8)0b11000000) == (u8)0b10000000; ++count)
            ++numDelete;
    
        _text->SetText(_text->GetText().substr(0, _text->GetText().size() - numDelete));
        _keyWasReleased = false;
        _keyHoldFrames = 0;
        _keyRepeatText.clear();
        emitChange();
        return;
    }

    const std::string keyText = PlatformBridge::Keyboard::GetInputString();
    if (!keyText.empty())
    {
        _text->SetText(_text->GetText() + keyText);
        _keyRepeatText = keyText;
        _keyWasReleased = false;
        _keyHoldFrames = 0;
        emitChange();
    }

    return;
}

void RetroFuturaGUI::ITextProperties::drawText()
{
    if(_text)
        _text->Draw();
}