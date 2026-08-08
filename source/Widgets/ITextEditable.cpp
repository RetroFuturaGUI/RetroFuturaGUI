#include "ITextEditable.hpp"
#include "Clipboard.hpp"
#include "InputManager.hpp"
#include "PlatformBridge.hpp"

void RetroFuturaGUI::ITextEditable::moveCaret()
{
    if(!_editingEnabled || !_text)
        return;

    if(_parentWindow != InputManager::GetFocusedWindow())
        return;

    if(PlatformBridge::Input::GetKeyboardUseState() == PlatformBridge::KeyboardUseState::KeyReleased)
    {
        _keyWasReleased = true;
        _keyHoldFrames = 0;
        _caretRepeatDirection = 0;
        return;
    }

    if(!_keyWasReleased)
    {
        if (_caretRepeatDirection != 0)
        {
            ++_keyHoldFrames;

            if (_keyHoldFrames >= _keyRepeatInitialDelay && (_keyHoldFrames - _keyRepeatInitialDelay) % _keyRepeatInterval == 0)
            {
                if(_caretRepeatDirection < 0)
                    moveCaretLeft();
                else
                    moveCaretRight();
            }
        }

        return;
    }

    if(PlatformBridge::Input::GetKeyPressState(PB_KEY_LEFT) == PlatformBridge::KeyPressState::Press)
    {
        moveCaretLeft();
        _caretRepeatDirection = -1;
    }
    else if(PlatformBridge::Input::GetKeyPressState(PB_KEY_RIGHT) == PlatformBridge::KeyPressState::Press)
    {
        moveCaretRight();
        _caretRepeatDirection = 1;
    }

    _keyWasReleased = false;
    _keyHoldFrames = 0;
}

void RetroFuturaGUI::ITextEditable::moveCaretLeft()
{
    deselect();

    if(CaretRelativePosition::Right == _caretRelativePosition)
    {
        _caretRelativePosition = CaretRelativePosition::Left;
        updateCaretPosition();
    }
    else if(_caretPosition >= _text->GetGlyphCount())
    {
        _caretPosition = _text->GetGlyphCount() - 1;
    }
    else if(_caretPosition != 0)
    {
        --_caretPosition;
        _caretRelativePosition = CaretRelativePosition::Left;
        updateCaretPosition();
    }
    else if(_caretPosition == 0)
    {
        _caretRelativePosition = CaretRelativePosition::Left;
        updateCaretPosition();
    }

    std::println("Pos: {}, Relative: {}", _caretPosition, _caretRelativePosition == CaretRelativePosition::Left ? "Left" : "Right");
}

void RetroFuturaGUI::ITextEditable::moveCaretRight()
{
    deselect();

    if(CaretRelativePosition::Left == _caretRelativePosition)
    {
        _caretRelativePosition = CaretRelativePosition::Right;
        updateCaretPosition();
    }
    else if(_caretPosition < _text->GetGlyphCount() -1)
    {
        ++_caretPosition;
        _caretRelativePosition = CaretRelativePosition::Right;
        updateCaretPosition();
    }
    else if(_caretPosition >= _text->GetGlyphCount() - 1)
    {
        _caretPosition = _text->GetGlyphCount() -1;
        _caretRelativePosition = CaretRelativePosition::Right;
        updateCaretPosition();
    }
    else if(_caretPosition == 0 && _caretRelativePosition == CaretRelativePosition::Left)
    {
        _caretRelativePosition = CaretRelativePosition::Right;
        updateCaretPosition();
    }
    else if(_caretPosition == 0 && _caretRelativePosition == CaretRelativePosition::Right)
    {
        ++_caretPosition;
        updateCaretPosition();
    }

    std::println("Pos: {}, Relative: {}", _caretPosition, _caretRelativePosition == CaretRelativePosition::Left ? "Left" : "Right");
}

void RetroFuturaGUI::ITextEditable::updateCaretPosition()
{
    glm::vec3 caretPosition { _text->GetGlyphPosition(_caretPosition, _caretRelativePosition, _caret->GetSize().y) };
    caretPosition.x = keepCaretVisible(caretPosition.x, _caret->GetSize().x * 0.5f);
    _caret->SetPosition(caretPosition);
}

void RetroFuturaGUI::ITextEditable::setCaretFromBoundary(const uSize boundary)
{
    if(boundary == 0)
    {
        _caretPosition = 0;
        _caretRelativePosition = CaretRelativePosition::Left;
    }
    else
    {
        _caretPosition = boundary - 1;
        _caretRelativePosition = CaretRelativePosition::Right;
    }

    glm::vec3 caretPosition { _text->GetGlyphPosition(_caretPosition, _caretRelativePosition, _caret->GetSize().y) };
    caretPosition.x = keepCaretVisible(caretPosition.x, _caret->GetSize().x * 0.5f);
    _caret->SetPosition(caretPosition);
}

void RetroFuturaGUI::ITextEditable::deselect()
{
    _isMarking = false;
    _isSelected = false;
}

void RetroFuturaGUI::ITextEditable::drawSelectedArea()
{
    if(!_selectedArea)
        return;

    if(_isSelected)
        _selectedArea->Draw();
}

void RetroFuturaGUI::ITextEditable::updateSelectedArea()
{
    const uSize
        left { _selectedPositionFirst < _selectedPositionLast ? _selectedPositionFirst : _selectedPositionLast },
        right { _selectedPositionFirst < _selectedPositionLast ? _selectedPositionLast : _selectedPositionFirst };

    if(!_text || !_selectedArea || left == right) //nothing selected
    {
        _isSelected = false;
        return;
    }

    const glm::vec3
        leftPosition { _text->GetBoundaryPosition(left, _caret->GetSize().y) },
        rightPosition { _text->GetBoundaryPosition(right, _caret->GetSize().y) };
    const f32
        clippedLeftX { clampToTextBounds(leftPosition.x) },
        clippedRightX { clampToTextBounds(rightPosition.x) },
        width { clippedRightX - clippedLeftX };

    if(width <= 0.0f) //selection sits entirely outside the visible text area
    {
        _isSelected = false;
        return;
    }

    _selectedArea->SetSize(glm::vec2(width, _caret->GetSize().y));
    _selectedArea->SetPosition(glm::vec3(clippedLeftX + width * 0.5f, leftPosition.y, _selectedArea->GetPosition().z));
    _isSelected = true;
}


bool RetroFuturaGUI::ITextEditable::checkForTextCopy()
{
    if((PlatformBridge::Input::IsKeyDown(PB_KEY_CONTROL_L) || PlatformBridge::Input::IsKeyDown(PB_KEY_CONTROL_R))
        && (PlatformBridge::Input::GetKeyPressState(PB_KEY_C) == PlatformBridge::KeyPressState::Press))
    {
        if(_isSelected && !_textCopied)
        {
            const uSize
                selectionStart { _selectedPositionFirst < _selectedPositionLast ? _selectedPositionFirst : _selectedPositionLast },
                selectionEnd { _selectedPositionFirst < _selectedPositionLast ? _selectedPositionLast : _selectedPositionFirst };
            std::u32string tempCopy = _text->GetTextUTF32().substr(selectionStart, selectionEnd - selectionStart);
            _copiedText = DoubleEncodedString::Utf32ToUtf8(tempCopy);
            PlatformBridge::Clipboard::CopyToClipboard(PlatformBridge::Clipboard::ClipboardDatatype::Text, static_cast<void*>(tempCopy.data()), tempCopy.size() * sizeof(char32_t));
            _textCopied = true;
            emitCopy();
            return true;
        }
    }
    
    _textCopied = false;
    return false;
}

bool RetroFuturaGUI::ITextEditable::checkForTextCut()
{
    if((PlatformBridge::Input::IsKeyDown(PB_KEY_CONTROL_L) || PlatformBridge::Input::IsKeyDown(PB_KEY_CONTROL_R))
        && (PlatformBridge::Input::GetKeyPressState(PB_KEY_X) == PlatformBridge::KeyPressState::Press))
    {
        if(!_textCopied)
        {
            if(!_isSelected)
                return true;

            const uSize
                selectionStart { _selectedPositionFirst < _selectedPositionLast ? _selectedPositionFirst : _selectedPositionLast },
                selectionEnd { _selectedPositionFirst < _selectedPositionLast ? _selectedPositionLast : _selectedPositionFirst };
            std::u32string tempCopy = _text->GetTextUTF32().substr(selectionStart, selectionEnd - selectionStart);
            _copiedText = DoubleEncodedString::Utf32ToUtf8(tempCopy);
            PlatformBridge::Clipboard::CopyToClipboard(PlatformBridge::Clipboard::ClipboardDatatype::Text, static_cast<void*>(tempCopy.data()), tempCopy.size() * sizeof(char32_t));
            _textCopied = true;
            _text->SetTextUTF32(_text->GetTextUTF32().substr(0, selectionStart) + _text->GetTextUTF32().substr(selectionEnd));
            _selectedPositionFirst = 0;
            _selectedPositionLast = 0;
            updateSelectedArea();
            _isSelected = false;
            setCaretFromBoundary(selectionStart);
            emitCopy();
            emitChange();
            return true;
        }
    }

     _textCopied = false;
     return false;
}

bool RetroFuturaGUI::ITextEditable::checkForTextPaste()
{
if((PlatformBridge::Input::IsKeyDown(PB_KEY_CONTROL_L) || PlatformBridge::Input::IsKeyDown(PB_KEY_CONTROL_R))
        && (PlatformBridge::Input::GetKeyPressState(PB_KEY_V) == PlatformBridge::KeyPressState::Press))
    {
        if(!_textPasted)
        {
            const uSize
                selectionStart { _selectedPositionFirst < _selectedPositionLast ? _selectedPositionFirst : _selectedPositionLast },
                selectionEnd { _selectedPositionFirst < _selectedPositionLast ? _selectedPositionLast : _selectedPositionFirst };
            std::u32string 
                middlePart {},
                rightPart {},
                completeText {};
            void* dataPtr = nullptr;
            uSize dataSize = 0;
            PlatformBridge::Clipboard::PasteFromClipboard(PlatformBridge::Clipboard::ClipboardDatatype::Text, dataPtr, &dataSize);
            middlePart = std::u32string(reinterpret_cast<char32_t*>(dataPtr), dataSize / sizeof(char32_t));

            if(_isSelected)
            {
                completeText = _text->GetTextUTF32().substr(0, selectionStart);
                rightPart = _text->GetTextUTF32().substr(selectionEnd);
                completeText += middlePart + rightPart;
                _isSelected = false;
                _text->SetTextUTF32(completeText);
                setCaretFromBoundary(_selectedPositionLast);
                _selectedPositionFirst = 0;
                _selectedPositionLast = 0;
                updateSelectedArea();
            }
            else //insert at caret position
            {
                uSize caretPositionAbsolute = _caretRelativePosition == CaretRelativePosition::Left ? _caretPosition : _caretPosition + 1;
                completeText = _text->GetTextUTF32().substr(0, caretPositionAbsolute);
                rightPart = _text->GetTextUTF32().substr(caretPositionAbsolute);
                completeText += middlePart + rightPart;
                _text->SetTextUTF32(completeText);
                setCaretFromBoundary(caretPositionAbsolute + middlePart.size());
            }

            PlatformBridge::Clipboard::ClearClipboardDataBuffer();
            _textPasted = true;
            emitPaste();
            emitChange();
            return true;
        }
    }
    
    _textPasted = false;
    return false;
}

bool RetroFuturaGUI::ITextEditable::checkForSelectAllText()
{
    if((PlatformBridge::Input::IsKeyDown(PB_KEY_CONTROL_L) || PlatformBridge::Input::IsKeyDown(PB_KEY_CONTROL_R))
            && (PlatformBridge::Input::GetKeyPressState(PB_KEY_A) == PlatformBridge::KeyPressState::Press))
    {
        if(_selectedPositionFirst != 0 || _selectedPositionLast != _text->GetGlyphCount())
        {
            _selectedPositionFirst = 0;
            _selectedPositionLast = _text->GetGlyphCount();
            setCaretFromBoundary(_selectedPositionLast);
            updateSelectedArea();
            return true;
        }
    }

    return false;
}

bool RetroFuturaGUI::ITextEditable::checkForKeyRelease()
{
    if(PlatformBridge::Input::GetKeyboardUseState() == PlatformBridge::KeyboardUseState::KeyReleased)
    {
        _keyWasReleased = true;
        _keyHoldFrames = 0;
        _keyRepeatText.clear();
        return true;;
    }

    return false;
}

bool RetroFuturaGUI::ITextEditable::checkForKeyRepeat()
{
    if(!_keyWasReleased)
    {
        if (!_keyRepeatText.empty())
        {
            ++_keyHoldFrames;

            if (_keyHoldFrames >= _keyRepeatInitialDelay && (_keyHoldFrames - _keyRepeatInitialDelay) % _keyRepeatInterval == 0)
            {
                const uSize cut { CaretRelativePosition::Left == _caretRelativePosition ? _caretPosition : _caretPosition + 1 };
                std::u32string left { _text->GetTextUTF32().substr(0, cut) };
                std::u32string right { _text->GetTextUTF32().substr(cut) };
                _text->SetTextUTF32(left + _keyRepeatText + right);
                ++_caretPosition;
                deselect();
                updateCaretPosition();
                emitChange();
            }
        }
        return true;
    }

    return false;
}

bool RetroFuturaGUI::ITextEditable::checkForEnterPress()
{
    if(PlatformBridge::Input::GetKeyPressState(PB_KEY_RETURN) == PlatformBridge::KeyPressState::Press
        || PlatformBridge::Input::GetKeyPressState(PB_KEY_KP_ENTER) == PlatformBridge::KeyPressState::Press
        || PlatformBridge::Input::GetKeyPressState(PB_KEY_ISO_ENTER) == PlatformBridge::KeyPressState::Press)
    {
        emitEnterPressed();
        _enterPressed = true;
        return true;
    }

    return false;
}

bool RetroFuturaGUI::ITextEditable::checkForBackspacePress()
{
    if(PlatformBridge::Input::GetKeyPressState(PB_KEY_BACKSPACE) == PlatformBridge::KeyPressState::Press)
    {
        if(_text->GetTextUTF32().size() == 0)
            return true;

        if(_text->GetTextUTF32().front() == 0)
            return true;

        if(_isSelected)
        {
            const uSize
                selectionStart { _selectedPositionFirst < _selectedPositionLast ? _selectedPositionFirst : _selectedPositionLast },
                selectionEnd { _selectedPositionFirst < _selectedPositionLast ? _selectedPositionLast : _selectedPositionFirst };
            _text->SetTextUTF32(_text->GetTextUTF32().substr(0, selectionStart) + _text->GetTextUTF32().substr(selectionEnd));
            _selectedPositionFirst = 0;
            _selectedPositionLast = 0;
            _isSelected = false;
            updateSelectedArea();
            setCaretFromBoundary(selectionStart);
            emitChange();
            return true;
        }
        else
        {
            uSize cut { CaretRelativePosition::Left == _caretRelativePosition ? _caretPosition : _caretPosition + 1 };
        
            if(0 < cut)
            {
                std::u32string left { _text->GetTextUTF32().substr(0, cut - 1) };
                std::u32string right { _text->GetTextUTF32().substr(cut) };
                //std::println("{}🐺{}", DoubleEncodedString::Utf32ToUtf8(left), DoubleEncodedString::Utf32ToUtf8(right));

                _text->SetTextUTF32(left + right);
                --_caretPosition;
                deselect();
                updateCaretPosition();
                _keyWasReleased = false;
                _keyHoldFrames = 0;
                _keyRepeatText.clear();
            }
        }

        emitChange();
        return true;
    }

    return false;
}

bool RetroFuturaGUI::ITextEditable::checkForTextInput()
{
    const std::u32string keyText = DoubleEncodedString::Utf8ToUtf32(PlatformBridge::Input::GetInputString());

    if (!keyText.empty())
    {
        if(_text->GetTextUTF32().size() == 0)
            return true;

        if(_text->GetTextUTF32().front() == 0)
            return true;

        if(_isSelected)
        {
            const uSize
                selectionStart { _selectedPositionFirst < _selectedPositionLast ? _selectedPositionFirst : _selectedPositionLast },
                selectionEnd { _selectedPositionFirst < _selectedPositionLast ? _selectedPositionLast : _selectedPositionFirst };
            _text->SetTextUTF32(_text->GetTextUTF32().substr(0, selectionStart) + keyText + _text->GetTextUTF32().substr(selectionEnd));
            _selectedPositionFirst = 0;
            _selectedPositionLast = 0;
            _isSelected = false;
            updateSelectedArea();
            setCaretFromBoundary(selectionStart + 1);
        }
        else
        {
            uSize cut { CaretRelativePosition::Left == _caretRelativePosition ? _caretPosition : _caretPosition + 1 };
            std::u32string left { _text->GetTextUTF32().substr(0, cut) };
            std::u32string right { _text->GetTextUTF32().substr(cut) };
            //std::println("{}🐺{}🐺{}", DoubleEncodedString::Utf32ToUtf8(left), DoubleEncodedString::Utf32ToUtf8(keyText), DoubleEncodedString::Utf32ToUtf8(right));
            _text->SetTextUTF32(left + keyText + right);
            ++_caretPosition;
            deselect();
            updateCaretPosition();
            _keyRepeatText = keyText;
            _keyWasReleased = false;
            _keyHoldFrames = 0;
        }
        
        emitChange();
        return true;
    }

    return false;
}

void RetroFuturaGUI::ITextEditable::editText()
{
    if(!_text)
        return;

    if(_parentWindow != InputManager::GetFocusedWindow())
        return;

    if(checkForTextCopy())
        return;

    if(checkForTextCut())
        return;
    
    if(checkForTextPaste())
        return;

    if(checkForSelectAllText())
        return;

    if(!_editingEnabled)
        return;

    if(_parentWindow != InputManager::GetFocusedWindow())
        return;

    if(_enterPressed)
    {
        emitEnterRelease();
        _enterPressed = false;
    }

    if(checkForKeyRelease())
        return;

    if(checkForKeyRepeat())
        return;

    if(checkForEnterPress())
        return;

    if(checkForBackspacePress())
        return;

    if(checkForTextInput())
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

void RetroFuturaGUI::ITextEditable::SetSelectedAreaColors(std::span<glm::vec4> colors)
{
    _selectedAreaColors.assign(colors.begin(), colors.end());

    if(_selectedArea)
        _selectedArea->SetColors(_selectedAreaColors);
}

void RetroFuturaGUI::ITextEditable::SetSelectedAreaFillType(const FillType fillType)
{
    if(_selectedArea)
        _selectedArea->SetFillType(fillType);
}

void RetroFuturaGUI::ITextEditable::SetSelectedAreaGradientAnimationSpeed(const f32 speed)
{
    if(_selectedArea)
        _selectedArea->SetGradientAnimationSpeed(speed);
}

void RetroFuturaGUI::ITextEditable::SetSelectedAreaGradientOffset(const f32 gradientOffset)
{
    if(_selectedArea)
        _selectedArea->SetGradientOffset(gradientOffset);
}

void RetroFuturaGUI::ITextEditable::SetSelectedAreaGradientDegree(const f32 degree)
{
    if(_selectedArea)
        _selectedArea->SetGradientDegree(degree);
}

void RetroFuturaGUI::ITextEditable::SetSelectedAreaGradientRotationSpeed(const f32 rotationSpeed)
{
    if(_selectedArea)
        _selectedArea->SetGradientRotationSpeed(rotationSpeed);
}

void RetroFuturaGUI::ITextEditable::SetSelectedAreaCornerRadii(const glm::vec4& radii)
{
    if(_selectedArea)
        _selectedArea->SetCornerRadii(radii);
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

void RetroFuturaGUI::ITextEditable::Connect_OnCopy(const typename Signal<>::Slot& slot, const bool async)
{
    if (async)
        _onCopyAsync.Connect(slot);
    else
        _onCopy.Connect(slot);
}

void RetroFuturaGUI::ITextEditable::Connect_OnPaste(const typename Signal<>::Slot& slot, const bool async)
{
    if (async)
        _onPasteAsync.Connect(slot);
    else
        _onPaste.Connect(slot);
}

void RetroFuturaGUI::ITextEditable::Disconnect_OnEnterPressed(const typename Signal<>::Slot& slot)
{
    _onEnterPressed.Disconnect(slot);
    _onEnterPressedAsync.Disconnect(slot);
}

void RetroFuturaGUI::ITextEditable::Disconnect_OnEnterReleased(const typename Signal<>::Slot& slot)
{
    _onEnterReleased.Disconnect(slot);
    _onEnterReleasedAsync.Disconnect(slot);
}

void RetroFuturaGUI::ITextEditable::Disconnect_OnCopy(const typename Signal<>::Slot& slot)
{
    _onCopy.Disconnect(slot);
    _onCopyAsync.Disconnect(slot);
}

void RetroFuturaGUI::ITextEditable::Disconnect_OnPaste(const typename Signal<>::Slot& slot)
{
    _onPaste.Disconnect(slot);
    _onPasteAsync.Disconnect(slot);
}

const std::string& RetroFuturaGUI::ITextEditable::GetCopiedText() const
{
    return _copiedText;
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

void RetroFuturaGUI::ITextEditable::emitCopy()
{
    _onCopyAsync.EmitAsync();
    _onCopy.Emit();
}

void RetroFuturaGUI::ITextEditable::emitPaste()
{
    _onPasteAsync.EmitAsync();
    _onPaste.Emit();
}