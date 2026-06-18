#include "ITextEditable.hpp"
#include "InputManager.hpp"

void RetroFuturaGUI::ITextEditable::moveCaret()
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

    if(!_keyWasReleased)
    {
        ++_keyHoldFrames;

        if (_keyHoldFrames >= _keyRepeatInitialDelay && (_keyHoldFrames - _keyRepeatInitialDelay) % _keyRepeatInterval == 0)
            moveCaretImpl();
        
        return;
    }

    moveCaretImpl();

    _keyWasReleased = false;
    _keyHoldFrames = 0;
}

void RetroFuturaGUI::ITextEditable::moveCaretImpl()
{
    if(PlatformBridge::Keyboard::GetKeyPressState(PB_KEY_LEFT) == PlatformBridge::KeyPressState::Press)
    {
        if(CaretRelativePosition::Right == _caretRelativePosition)
        {
            _caretRelativePosition = CaretRelativePosition::Left;
            _caret->SetPosition(_text->GetGlyphPosition(_caretPosition, _caretRelativePosition, _caret->GetSize().y));
        }
        else if(_caretPosition >= _text->GetGlyphCount())
        {
            _caretPosition = _text->GetGlyphCount() - 1;
        }
        else if(_caretPosition != 0)
        {
            --_caretPosition;
            _caretRelativePosition = CaretRelativePosition::Left;
            _caret->SetPosition(_text->GetGlyphPosition(_caretPosition, _caretRelativePosition, _caret->GetSize().y));
        }
        else if(_caretPosition == 0)
        {
            _caretRelativePosition = CaretRelativePosition::Left;
            _caret->SetPosition(_text->GetGlyphPosition(_caretPosition, _caretRelativePosition, _caret->GetSize().y));
        }
    }
    else if(PlatformBridge::Keyboard::GetKeyPressState(PB_KEY_RIGHT) == PlatformBridge::KeyPressState::Press)
    {

        if(CaretRelativePosition::Left == _caretRelativePosition)
        {
            _caretRelativePosition = CaretRelativePosition::Right;
            _caret->SetPosition(_text->GetGlyphPosition(_caretPosition, _caretRelativePosition, _caret->GetSize().y));
        }
        else if(_caretPosition < _text->GetGlyphCount() -1)
        {

            ++_caretPosition;
            _caretRelativePosition = CaretRelativePosition::Right;
            _caret->SetPosition(_text->GetGlyphPosition(_caretPosition, _caretRelativePosition, _caret->GetSize().y));
        }
        else if(_caretPosition >= _text->GetGlyphCount() - 1)
        {
            _caretPosition = _text->GetGlyphCount() -1;
            _caretRelativePosition = CaretRelativePosition::Right;
            _caret->SetPosition(_text->GetGlyphPosition(_caretPosition, CaretRelativePosition::Right, _caret->GetSize().y));
        }
        else if(_caretPosition == 0 && _caretRelativePosition == CaretRelativePosition::Left)
        {
            _caretRelativePosition = CaretRelativePosition::Right;
            _caret->SetPosition(_text->GetGlyphPosition(_caretPosition, _caretRelativePosition, _caret->GetSize().y));
        }
        else if(_caretPosition == 0 && _caretRelativePosition == CaretRelativePosition::Right)
        {
            ++_caretPosition;
            _caret->SetPosition(_text->GetGlyphPosition(_caretPosition, CaretRelativePosition::Right, _caret->GetSize().y));
        }
    }

    std::println("Pos: {}, Relative: {}", _caretPosition, _caretRelativePosition == CaretRelativePosition::Left ? "Left" : "Right");
}

void RetroFuturaGUI::ITextEditable::editText()
{
    if(!_editingEnabled || !_text)
        return;

    if(_parentWindow != InputManager::GetFocusedWindow())
        return;

    if(_enterPressed)
    {
        emitEnterRelease();
        _enterPressed = false;
    }

    if(PlatformBridge::Keyboard::GetKeyboardUseState() == PlatformBridge::KeyboardUseState::KeyReleased)
    {
        _keyWasReleased = true;
        _keyHoldFrames = 0;
        _keyRepeatText.clear();
        return;
    }

    if(!_keyWasReleased)
    {
        if (!_keyRepeatText.empty())
        {
            ++_keyHoldFrames;
            if (_keyHoldFrames >= _keyRepeatInitialDelay && (_keyHoldFrames - _keyRepeatInitialDelay) % _keyRepeatInterval == 0)
            {
                _text->SetTextUTF32(_text->GetTextUTF32() + _keyRepeatText);
                emitChange();
            }
        }
        return;
    }

    if(PlatformBridge::Keyboard::GetKeyPressState(PB_KEY_RETURN) == PlatformBridge::KeyPressState::Press
        || PlatformBridge::Keyboard::GetKeyPressState(PB_KEY_KP_ENTER) == PlatformBridge::KeyPressState::Press
        || PlatformBridge::Keyboard::GetKeyPressState(PB_KEY_ISO_ENTER) == PlatformBridge::KeyPressState::Press)
    {
        emitEnterPressed();
        _enterPressed = true;
        return;
    }

    if(PlatformBridge::Keyboard::GetKeyPressState(PB_KEY_BACKSPACE) == PlatformBridge::KeyPressState::Press)
    {
        if(_text->GetTextUTF32().size() == 0)
            return;

        if(_text->GetTextUTF32().front() == 0)
            return;

        uSize cut { CaretRelativePosition::Left == _caretRelativePosition ? _caretPosition : _caretPosition + 1 };
    
        if(0 < cut)
        {
            std::u32string left { _text->GetTextUTF32().substr(0, cut - 1) };
            std::u32string right { _text->GetTextUTF32().substr(cut) };
            //std::println("{}🐺{}", DoubleEncodedString::Utf32ToUtf8(left), DoubleEncodedString::Utf32ToUtf8(right));

            _text->SetTextUTF32(left + right);
            --_caretPosition;
            _caret->SetPosition(_text->GetGlyphPosition(_caretPosition, _caretRelativePosition, _caret->GetSize().y));
            _keyWasReleased = false;
            _keyHoldFrames = 0;
            _keyRepeatText.clear();
            emitChange();
        }

        return;
    }

    const std::u32string keyText = DoubleEncodedString::Utf8ToUtf32(PlatformBridge::Keyboard::GetInputString());

    if (!keyText.empty())
    {
        _text->SetTextUTF32(_text->GetTextUTF32() + keyText);
        _keyRepeatText = keyText;
        _keyWasReleased = false;
        _keyHoldFrames = 0;
        emitChange();
    }

    return;
}

void RetroFuturaGUI::ITextEditable::SetReadOnly(const bool readOnly)
{
    _readOnly = readOnly;
}

bool RetroFuturaGUI::ITextEditable::IsReadOnly() const
{
    return _readOnly;
}

void RetroFuturaGUI::ITextEditable::SetCaretColors(std::span<glm::vec4> colors)
{
    if(_caret)
        _caret->SetColors(colors);
}

void RetroFuturaGUI::ITextEditable::SetCaretFillType(const FillType fillType)
{
    if(_caret)
        _caret->SetFillType(fillType);
}

void RetroFuturaGUI::ITextEditable::SetCaretGradientAnimationSpeed(const f32 speed)
{
    if(_caret)
        _caret->SetGradientAnimationSpeed(speed);
}

void RetroFuturaGUI::ITextEditable::Connect_OnEnterPressed(const typename Signal<>::Slot& slot, const bool async)
{
    if (async)
        _onEnterPressedAsync.Connect(slot);
    else
        _onEnterPressed.Connect(slot);
}

void RetroFuturaGUI::ITextEditable::Connect_OnEnterReleased(const typename Signal<>::Slot& slot, const bool async)
{
    if (async)
        _onEnterReleasedAsync.Connect(slot);
    else
        _onEnterReleased.Connect(slot);
}

void RetroFuturaGUI::ITextEditable::Disconnect_OnEnterPressed(const typename Signal<>::Slot& slot)
{
    _onEnterPressed.Disconnect(slot);
    _onEnterPressedAsync.Disconnect(slot);
}

void RetroFuturaGUI::ITextEditable::Disconnect_OnEnterRelease(const typename Signal<>::Slot& slot)
{
    _onEnterReleased.Disconnect(slot);
    _onEnterReleasedAsync.Disconnect(slot);
}

void RetroFuturaGUI::ITextEditable::emitEnterRelease()
{
    _onEnterReleasedAsync.EmitAsync();
    _onEnterReleased.Emit();
}

void RetroFuturaGUI::ITextEditable::emitEnterPressed()
{
    _onEnterPressedAsync.EmitAsync();
    _onEnterPressed.Emit();
}

void RetroFuturaGUI::ITextEditable::emitChange()
{
    if(_enterPressed)
        return;

    _onTextChangeAsync.EmitAsync();
    _onTextChange.Emit();
}