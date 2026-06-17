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
        if(_caretPosition >= _text->GetGlyphCount())
            _caretPosition = _text->GetGlyphCount() - 1;

        if(_caretPosition != 0)
        {
            --_caretPosition;
            _caretRelativePosition = CaretRelativePosition::Right;
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

        if(_caretPosition < _text->GetGlyphCount() -1)
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

    //std::println("Pos: {}, Relative: {}", _caretPosition, _caretRelativePosition == CaretRelativePosition::Left ? "Left" : "Right");
}