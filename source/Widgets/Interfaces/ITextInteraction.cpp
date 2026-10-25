#include "ITextInteraction.hpp"
#include "PlatformBridge.hpp"
#include "DoubleEncodedString.hpp"
#include "Window.hpp"
#include <algorithm>

#if defined(TARGET_PLATFORM_LINUX)
    #define GLFW_EXPOSE_NATIVE_X11
#elif defined(TARGET_PLATFORM_WINDOWS)
    #define GLFW_EXPOSE_NATIVE_WIN32
#endif
#include <GLFW/glfw3native.h>

RetroFuturaGUI::ITextInteraction::~ITextInteraction()
{
    releaseKeyboardFocus();
}

void RetroFuturaGUI::ITextInteraction::takeKeyboardFocus()
{
    _keyboardFocus = this;

    //A key down when the focus comes was typed for something else, like the key that opened this box; it types once pressed again
    _repeatKeySym = PlatformBridge::Input::GetLastKeySym();
    _repeatKeyPressCountSeen = PlatformBridge::Input::GetKeyPressCount(_repeatKeySym);
    _keyRepeatText.clear();

    //Keys arrive at the window that has the focus, which is the host's while this window sits in a tab
#if defined(TARGET_PLATFORM_LINUX)
    PlatformBridge::Input::SetActiveDisplay(glfwGetX11Display());
    PlatformBridge::Input::SetActiveWindow(glfwGetX11Window(Window::KeyboardWindowOf(_parentWindow)));
#elif defined(TARGET_PLATFORM_WINDOWS)
    PlatformBridge::Input::SetActiveWindow(glfwGetWin32Window(Window::KeyboardWindowOf(_parentWindow)));
#endif
}

void RetroFuturaGUI::ITextInteraction::releaseKeyboardFocus()
{
    if(this != _keyboardFocus)
        return;

    _keyboardFocus = nullptr;
    PlatformBridge::Input::SetActiveDisplay(nullptr);
    PlatformBridge::Input::SetActiveWindow(0);
}

bool RetroFuturaGUI::ITextInteraction::hasInputFocus() const
{
    if(this != _keyboardFocus)
        return false;

    const uint64_t activeWindowId { PlatformBridge::Input::GetActiveWindowID() };

    //The window keys arrive at, which is the host's while this window sits in a tab
#if defined(TARGET_PLATFORM_LINUX)
    return activeWindowId == static_cast<uint64_t>(glfwGetX11Window(Window::KeyboardWindowOf(_parentWindow)));
#elif defined(TARGET_PLATFORM_WINDOWS)
    return activeWindowId == reinterpret_cast<uint64_t>(glfwGetWin32Window(Window::KeyboardWindowOf(_parentWindow)));
#else
    return false;
#endif
}

void RetroFuturaGUI::ITextInteraction::updateCaretBlink()
{
    if(!_showCaret)
    {
        resetCaretBlink();
        return;
    }

    const f64 elapsedMilliseconds { std::chrono::duration<f64, std::milli>(std::chrono::high_resolution_clock::now() - _millisecondsPassed).count() };

    if(elapsedMilliseconds < _blinkForMilliseconds)
        return;

    _caretBlinkState = !_caretBlinkState;
    _millisecondsPassed = std::chrono::high_resolution_clock::now();
}

void RetroFuturaGUI::ITextInteraction::resetCaretBlink()
{
    _caretBlinkState = true;
    _millisecondsPassed = std::chrono::high_resolution_clock::now();
}

void RetroFuturaGUI::ITextInteraction::deselect()
{
    _isMarking = false;
    _isSelected = false;
}

void RetroFuturaGUI::ITextInteraction::placeCaretAfterText()
{
    Text* text { activeText() };

    if(!text)
        return;

    deselect();
    _caretPosition = text->GetTextUTF32().size();
    _selectedPositionFirst = _caretPosition;
    _selectedPositionLast = _caretPosition;
    updateCaretPosition();
}

uSize RetroFuturaGUI::ITextInteraction::markedStart() const
{
    return _selectedPositionFirst < _selectedPositionLast ? _selectedPositionFirst : _selectedPositionLast;
}

uSize RetroFuturaGUI::ITextInteraction::markedEnd() const
{
    return _selectedPositionFirst < _selectedPositionLast ? _selectedPositionLast : _selectedPositionFirst;
}

bool RetroFuturaGUI::ITextInteraction::shouldRepeat(const u32 holdFrames)
{
    return holdFrames >= _keyRepeatInitialDelay && 0 == (holdFrames - _keyRepeatInitialDelay) % _keyRepeatInterval;
}

const std::string& RetroFuturaGUI::ITextInteraction::GetCopiedText() const
{
    return _copiedText;
}

void RetroFuturaGUI::ITextInteraction::moveCaret()
{
    if(!activeText())
        return;

    if(!hasInputFocus())
        return;

   if(PlatformBridge::Input::GetKeyboardUseState() == PlatformBridge::KeyboardUseState::KeyReleased)
    {
        _caretKeyWasReleased = true;
        _caretKeyHoldFrames = 0;
        _caretRepeatDirection = CaretDirection::None;
        return;
    }

    if(!_caretKeyWasReleased)
    {
        ++_caretKeyHoldFrames;

        if(shouldRepeat(_caretKeyHoldFrames))
            stepCaret(_caretRepeatDirection);

        return;
    }

    if(PlatformBridge::Input::GetKeyPressState(PB_KEY_LEFT) == PlatformBridge::KeyPressState::Press)
        _caretRepeatDirection = CaretDirection::Left;
    else if(PlatformBridge::Input::GetKeyPressState(PB_KEY_RIGHT) == PlatformBridge::KeyPressState::Press)
        _caretRepeatDirection = CaretDirection::Right;
    else if(PlatformBridge::Input::GetKeyPressState(PB_KEY_UP) == PlatformBridge::KeyPressState::Press)
        _caretRepeatDirection = CaretDirection::Up;
    else if(PlatformBridge::Input::GetKeyPressState(PB_KEY_DOWN) == PlatformBridge::KeyPressState::Press)
        _caretRepeatDirection = CaretDirection::Down;
    else
        return;

    stepCaret(_caretRepeatDirection);
    _caretKeyWasReleased = false;
    _caretKeyHoldFrames = 0;
}

void RetroFuturaGUI::ITextInteraction::stepCaret(const CaretDirection direction)
{
    switch(direction)
    {
        case CaretDirection::Left:
            moveCaretLeft();
            break;
        case CaretDirection::Right:
            moveCaretRight();
            break;
        case CaretDirection::Up:
            moveCaretUp();
            break;
        case CaretDirection::Down:
            moveCaretDown();
            break;
        default: //None
            break;
    }
}

void RetroFuturaGUI::ITextInteraction::moveCaretLeft()
{
    deselect();

    if(0 < _caretPosition)
        --_caretPosition;

    updateCaretPosition();
}

void RetroFuturaGUI::ITextInteraction::moveCaretRight()
{
    Text* text { activeText() };

    if(!text)
        return;

    deselect();

    if(_caretPosition < text->GetGlyphCount())
        ++_caretPosition;

    updateCaretPosition();
}

void RetroFuturaGUI::ITextInteraction::setCaretFromBoundary(const uSize boundary)
{
    _caretPosition = boundary;
    updateCaretPosition();
}

bool RetroFuturaGUI::ITextInteraction::checkForTextCopy()
{
    Text* text { activeText() };

    if((PlatformBridge::Input::IsKeyDown(PB_KEY_CONTROL_L) || PlatformBridge::Input::IsKeyDown(PB_KEY_CONTROL_R))
        && (PlatformBridge::Input::GetKeyPressState(PB_KEY_C) == PlatformBridge::KeyPressState::Press))
    {
        if(_isSelected && !_textCopied && text)
        {
            const uSize
                selectionStart { markedStart() },
                selectionEnd { markedEnd() };
            std::u32string tempCopy { text->GetTextUTF32().substr(selectionStart, selectionEnd - selectionStart) };
            _copiedText = DoubleEncodedString::Utf32ToUtf8(tempCopy);
            PlatformBridge::Clipboard::CopyToClipboard(PlatformBridge::Clipboard::ClipboardDatatype::Text, static_cast<void*>(tempCopy.data()), tempCopy.size() * sizeof(char32_t));
            _textCopied = true;
            emitCopy();
        }

        return true;
    }

    _textCopied = false;
    return false;
}

bool RetroFuturaGUI::ITextInteraction::checkForTextCut()
{
    Text* text { activeText() };

    if((PlatformBridge::Input::IsKeyDown(PB_KEY_CONTROL_L) || PlatformBridge::Input::IsKeyDown(PB_KEY_CONTROL_R))
        && (PlatformBridge::Input::GetKeyPressState(PB_KEY_X) == PlatformBridge::KeyPressState::Press))
    {
        if(!_textCut)
        {
            if(!_isSelected || !text)
                return true;

            const uSize
                selectionStart { markedStart() },
                selectionEnd { markedEnd() };
            std::u32string tempCopy { text->GetTextUTF32().substr(selectionStart, selectionEnd - selectionStart) };
            _copiedText = DoubleEncodedString::Utf32ToUtf8(tempCopy);
            PlatformBridge::Clipboard::CopyToClipboard(PlatformBridge::Clipboard::ClipboardDatatype::Text, static_cast<void*>(tempCopy.data()), tempCopy.size() * sizeof(char32_t));
            _textCut = true;
            text->SetTextUTF32(text->GetTextUTF32().substr(0, selectionStart) + text->GetTextUTF32().substr(selectionEnd));
            _selectedPositionFirst = 0;
            _selectedPositionLast = 0;
            updateSelectedArea();
            _isSelected = false;
            setCaretFromBoundary(selectionStart);
            emitCopy();
            emitChange();
        }

        return true;
    }

    _textCut = false;
    return false;
}

bool RetroFuturaGUI::ITextInteraction::checkForTextPaste()
{
    Text* text { activeText() };

    if((PlatformBridge::Input::IsKeyDown(PB_KEY_CONTROL_L) || PlatformBridge::Input::IsKeyDown(PB_KEY_CONTROL_R))
        && (PlatformBridge::Input::GetKeyPressState(PB_KEY_V) == PlatformBridge::KeyPressState::Press))
    {
        if(!_textPasted && text)
        {
            const uSize
                selectionStart { markedStart() },
                selectionEnd { markedEnd() };
            std::u32string
                middlePart {},
                rightPart {},
                completeText {};
            void* dataPtr { nullptr };
            uSize dataSize { 0 };
            PlatformBridge::Clipboard::PasteFromClipboard(PlatformBridge::Clipboard::ClipboardDatatype::Text, dataPtr, &dataSize);

            if(0 == dataSize)
                return true;

            middlePart = std::u32string(reinterpret_cast<char32_t*>(dataPtr), dataSize / sizeof(char32_t));
            filterPastedText(middlePart);

            if(middlePart.empty()) //nothing pasteable left - keep the selection as it is
            {
                PlatformBridge::Clipboard::ClearClipboardDataBuffer();
                _textPasted = true;
                return true;
            }

            if(_isSelected)
            {
                completeText = text->GetTextUTF32().substr(0, selectionStart);
                rightPart = text->GetTextUTF32().substr(selectionEnd);
                completeText += middlePart + rightPart;
                _isSelected = false;
                text->SetTextUTF32(completeText);
                setCaretFromBoundary(selectionStart + middlePart.size());
                _selectedPositionFirst = 0;
                _selectedPositionLast = 0;
                updateSelectedArea();
            }
            else // insert at the caret
            {
                completeText = text->GetTextUTF32().substr(0, _caretPosition);
                rightPart = text->GetTextUTF32().substr(_caretPosition);
                completeText += middlePart + rightPart;
                text->SetTextUTF32(completeText);
                setCaretFromBoundary(_caretPosition + middlePart.size());
            }

            PlatformBridge::Clipboard::ClearClipboardDataBuffer();
            _textPasted = true;
            emitPaste();
            emitChange();
        }

        return true;
    }

    _textPasted = false;
    return false;
}

void RetroFuturaGUI::ITextInteraction::filterPastedText(std::u32string& text) const
{
    std::ranges::replace(text, U'\n', U' ');
    std::erase_if(text, isInvalidCodepoint);
}

bool RetroFuturaGUI::ITextInteraction::checkForSelectAllText()
{
    Text* text { activeText() };

    if((PlatformBridge::Input::IsKeyDown(PB_KEY_CONTROL_L) || PlatformBridge::Input::IsKeyDown(PB_KEY_CONTROL_R))
        && (PlatformBridge::Input::GetKeyPressState(PB_KEY_A) == PlatformBridge::KeyPressState::Press))
    {
        if(text && (0 != _selectedPositionFirst || _selectedPositionLast != text->GetGlyphCount()))
        {
            _selectedPositionFirst = 0;
            _selectedPositionLast = text->GetGlyphCount();
            _isSelected = true;
            updateSelectedArea();
            setCaretFromBoundary(_selectedPositionLast);
        }

        return true;
    }

    return false;
}

bool RetroFuturaGUI::ITextInteraction::checkForKeyRelease()
{
    if(PlatformBridge::Input::GetKeyboardUseState() == PlatformBridge::KeyboardUseState::KeyReleased)
    {
        _keyHoldFrames = 0;
        _keyRepeatText.clear();
        return true;
    }

    return false;
}

bool RetroFuturaGUI::ITextInteraction::checkForKeyRepeat()
{
    Text* text { activeText() };

    if(!text || _keyRepeatText.empty())
        return false;

    const bool stillSameKeyPress {
        PlatformBridge::Input::GetKeyPressState(_repeatKeySym) != PlatformBridge::KeyPressState::Release
        && PlatformBridge::Input::GetKeyPressCount(_repeatKeySym) == _repeatKeyPressCountSeen
    };

    if(!stillSameKeyPress)
    {
        _keyRepeatText.clear();
        _keyHoldFrames = 0;
        return false;
    }

    ++_keyHoldFrames;

    if(shouldRepeat(_keyHoldFrames))
    {
        std::u32string left { text->GetTextUTF32().substr(0, _caretPosition) };
        std::u32string right { text->GetTextUTF32().substr(_caretPosition) };
        text->SetTextUTF32(left + _keyRepeatText + right);
        _caretPosition += _keyRepeatText.size();
        deselect();
        updateCaretPosition();
        emitChange();
    }

    return true;
}

bool RetroFuturaGUI::ITextInteraction::checkForEnterPress()
{
    Text* text { activeText() };

    if(!text)
        return false;

    const u32 keySym { PlatformBridge::Input::GetLastKeySym() };

    if(PB_KEY_RETURN != keySym && PB_KEY_KP_ENTER != keySym && PB_KEY_ISO_ENTER != keySym)
        return false;

    if(keySym == _repeatKeySym && PlatformBridge::Input::GetKeyPressCount(keySym) == _repeatKeyPressCountSeen)
        return true;

    _repeatKeySym = keySym;
    _repeatKeyPressCountSeen = PlatformBridge::Input::GetKeyPressCount(keySym);

    insertLineBreak();
    emitEnterPressed();
    _enterPressed = true;
    return true;
}

bool RetroFuturaGUI::ITextInteraction::checkForBackspacePress()
{
    Text* text { activeText() };

    if(!text)
        return false;

    if(!PlatformBridge::Input::IsKeyDown(PB_KEY_BACKSPACE))
    {
        _backspaceKeyHoldFrames = 0;
        _backspacePressCountSeen = PlatformBridge::Input::GetKeyPressCount(PB_KEY_BACKSPACE);
        return false;
    }

    const u32 currentPressCount { PlatformBridge::Input::GetKeyPressCount(PB_KEY_BACKSPACE) };
    bool shouldDelete { currentPressCount != _backspacePressCountSeen };

    if(shouldDelete)
    {
        _backspacePressCountSeen = currentPressCount;
        _backspaceKeyHoldFrames = 0;
    }
    else
    {
        ++_backspaceKeyHoldFrames;
        shouldDelete = shouldRepeat(_backspaceKeyHoldFrames);
    }

    if(shouldDelete)
    {
        if(0 == text->GetTextUTF32().size())
            return true;

        if(0 == text->GetTextUTF32().front())
            return true;

        if(_isSelected)
        {
            const uSize
                selectionStart { markedStart() },
                selectionEnd { markedEnd() };
            text->SetTextUTF32(text->GetTextUTF32().substr(0, selectionStart) + text->GetTextUTF32().substr(selectionEnd));
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
            if(0 < _caretPosition)
            {
                std::u32string left { text->GetTextUTF32().substr(0, _caretPosition - 1) };
                std::u32string right { text->GetTextUTF32().substr(_caretPosition) };

                text->SetTextUTF32(left + right);
                --_caretPosition;
                deselect();
                updateCaretPosition();
            }
        }

        emitChange();
        return true;
    }

    return true;
}

bool RetroFuturaGUI::ITextInteraction::checkForTextInput()
{
    Text* text { activeText() };

    if(!text)
        return false;

    if(PlatformBridge::Input::GetLastKeySym() == _repeatKeySym && PlatformBridge::Input::GetKeyPressCount(_repeatKeySym) == _repeatKeyPressCountSeen)
        return false; //the key that was down when the focus came

    std::u32string keyText { DoubleEncodedString::Utf8ToUtf32(PlatformBridge::Input::GetInputString()) };
    std::erase_if(keyText, isInvalidCodepoint);

    if(keyText.empty())
        return false;

    if(_isSelected)
    {
        const uSize
            selectionStart { markedStart() },
            selectionEnd { markedEnd() };
        text->SetTextUTF32(text->GetTextUTF32().substr(0, selectionStart) + keyText + text->GetTextUTF32().substr(selectionEnd));
        _selectedPositionFirst = 0;
        _selectedPositionLast = 0;
        _isSelected = false;
        updateSelectedArea();
        setCaretFromBoundary(selectionStart + keyText.size());
    }
    else
    {
        std::u32string left { text->GetTextUTF32().substr(0, _caretPosition) };
        std::u32string right { text->GetTextUTF32().substr(_caretPosition) };
        text->SetTextUTF32(left + keyText + right);
        _caretPosition += keyText.size();
        deselect();
        updateCaretPosition();
    }

    //The key's text stays until it is released, so it is the repeating key now; otherwise typing over a selection put the character in twice
    _keyRepeatText = keyText;
    _repeatKeySym = PlatformBridge::Input::GetLastKeySym();
    _repeatKeyPressCountSeen = PlatformBridge::Input::GetKeyPressCount(_repeatKeySym);
    _keyHoldFrames = 0;
    emitChange();
    return true;
}

void RetroFuturaGUI::ITextInteraction::editText()
{
    Text* text { activeText() };

    if(!text)
        return;

    if(!hasInputFocus())
        return;

    const uSize length { text->GetTextUTF32().size() };

    if(_caretPosition > length)
        _caretPosition = length;

    if(_selectedPositionFirst > length)
        _selectedPositionFirst = length;

    if(_selectedPositionLast > length)
        _selectedPositionLast = length;

    /* Copy and select-all stay available on read-only text; only the mutating paths are gated, which is why
       the _editingEnabled check sits below them rather than at the top. */
    if(checkForTextCopy())
        return;

    if(checkForSelectAllText())
        return;

    const bool readOnly { isTextReadOnly() };

    if(!readOnly && checkForTextCut())
        return;

    if(!readOnly && checkForTextPaste())
        return;

    if(!_editingEnabled || readOnly)
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

void RetroFuturaGUI::ITextInteraction::emitEnterPressed()
{
    _onEnterPressedAsync.EmitAsync();
    _onEnterPressed.Emit();
}

void RetroFuturaGUI::ITextInteraction::emitEnterRelease()
{
    _onEnterReleasedAsync.EmitAsync();
    _onEnterReleased.Emit();
}

void RetroFuturaGUI::ITextInteraction::emitCopy()
{
    _onCopyAsync.EmitAsync();
    _onCopy.Emit();
}

void RetroFuturaGUI::ITextInteraction::emitPaste()
{
    _onPasteAsync.EmitAsync();
    _onPaste.Emit();
}

void RetroFuturaGUI::ITextInteraction::Connect_OnEnterPressed(const typename Signal<>::Slot& slot, const bool async)
{
    if(async)
        _onEnterPressedAsync.Connect(slot);
    else
        _onEnterPressed.Connect(slot);
}

void RetroFuturaGUI::ITextInteraction::Connect_OnEnterReleased(const typename Signal<>::Slot& slot, const bool async)
{
    if(async)
        _onEnterReleasedAsync.Connect(slot);
    else
        _onEnterReleased.Connect(slot);
}

void RetroFuturaGUI::ITextInteraction::Connect_OnCopy(const typename Signal<>::Slot& slot, const bool async)
{
    if(async)
        _onCopyAsync.Connect(slot);
    else
        _onCopy.Connect(slot);
}

void RetroFuturaGUI::ITextInteraction::Connect_OnPaste(const typename Signal<>::Slot& slot, const bool async)
{
    if(async)
        _onPasteAsync.Connect(slot);
    else
        _onPaste.Connect(slot);
}

void RetroFuturaGUI::ITextInteraction::Disconnect_OnEnterPressed(const typename Signal<>::Slot& slot)
{
    _onEnterPressedAsync.Disconnect(slot);
    _onEnterPressed.Disconnect(slot);
}

void RetroFuturaGUI::ITextInteraction::Disconnect_OnEnterReleased(const typename Signal<>::Slot& slot)
{
    _onEnterReleasedAsync.Disconnect(slot);
    _onEnterReleased.Disconnect(slot);
}

void RetroFuturaGUI::ITextInteraction::Disconnect_OnCopy(const typename Signal<>::Slot& slot)
{
    _onCopyAsync.Disconnect(slot);
    _onCopy.Disconnect(slot);
}

void RetroFuturaGUI::ITextInteraction::Disconnect_OnPaste(const typename Signal<>::Slot& slot)
{
    _onPasteAsync.Disconnect(slot);
    _onPaste.Disconnect(slot);
}
