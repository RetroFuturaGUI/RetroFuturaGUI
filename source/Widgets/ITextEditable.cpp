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
                _text->SetTextUTF32(_text->GetTextUTF32() + _keyRepeatText);
                emitChange();
            }
        }
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