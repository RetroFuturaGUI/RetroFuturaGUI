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
    _focusKeySym = PlatformBridge::Input::GetLastKeySym();
    _focusKeyPressCountSeen = PlatformBridge::Input::GetKeyPressCount(_focusKeySym);
    _enterPressCountSeen = getEnterPressCount();

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
    Text* text { getActiveText() };

    if(!text)
        return;

    deselect();
    _caretPosition = text->GetTextUTF32().size();
    _selectedPositionFirst = _caretPosition;
    _selectedPositionLast = _caretPosition;
    updateCaretPosition();
}

uSize RetroFuturaGUI::ITextInteraction::getMarkedStart() const
{
    return _selectedPositionFirst < _selectedPositionLast ? _selectedPositionFirst : _selectedPositionLast;
}

uSize RetroFuturaGUI::ITextInteraction::getMarkedEnd() const
{
    return _selectedPositionFirst < _selectedPositionLast ? _selectedPositionLast : _selectedPositionFirst;
}

uSize RetroFuturaGUI::ITextInteraction::getKeyStrokeCount(const u32 key) const
{
    if(key == _focusKeySym && PlatformBridge::Input::GetKeyPressCount(key) == _focusKeyPressCountSeen)
        return 0; //the key that was down when the focus came

    return PlatformBridge::Input::GetKeyStrokeCount(key);
}

u32 RetroFuturaGUI::ITextInteraction::getEnterPressCount()
{
    return PlatformBridge::Input::GetKeyPressCount(PB_KEY_RETURN)
        + PlatformBridge::Input::GetKeyPressCount(PB_KEY_KP_ENTER)
        + PlatformBridge::Input::GetKeyPressCount(PB_KEY_ISO_ENTER);
}

const std::string& RetroFuturaGUI::ITextInteraction::GetCopiedText() const
{
    return _copiedText;
}

void RetroFuturaGUI::ITextInteraction::CopySelectedText()
{
    Text* text { getActiveText() };

    if(!_isSelected)
        return;

    if(!text)
        return;

    const uSize
        selectionStart { getMarkedStart() },
        selectionEnd { getMarkedEnd() };
    std::u32string copiedText { text->GetTextUTF32().substr(selectionStart, selectionEnd - selectionStart) };
    _copiedText = DoubleEncodedString::Utf32ToUtf8(copiedText);
    PlatformBridge::Clipboard::CopyToClipboard(PlatformBridge::Clipboard::ClipboardDatatype::Text, copiedText.data(), copiedText.size() * sizeof(char32_t));
    _textCopied = true;
    emitCopy();
}

void RetroFuturaGUI::ITextInteraction::CutSelectedText()
{
    if(isTextReadOnly())
        return;

    Text* text { getActiveText() };

    if(!_isSelected)
        return;

    if(!text)
        return;

    const uSize
        selectionStart { getMarkedStart() },
        selectionEnd { getMarkedEnd() };
    std::u32string copiedText { text->GetTextUTF32().substr(selectionStart, selectionEnd - selectionStart) };
    _copiedText = DoubleEncodedString::Utf32ToUtf8(copiedText);
    PlatformBridge::Clipboard::CopyToClipboard(PlatformBridge::Clipboard::ClipboardDatatype::Text, copiedText.data(), copiedText.size() * sizeof(char32_t));
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

void RetroFuturaGUI::ITextInteraction::PasteSelectedText()
{
    if(isTextReadOnly())
        return;

    Text* text { getActiveText() };

    if(!text)
        return;

    const uSize
        selectionStart { getMarkedStart() },
        selectionEnd { getMarkedEnd() };
    std::u32string
        pastedText {},
        rightPart {},
        completeText {};
    void* data { nullptr };
    uSize dataSize { 0 };
    PlatformBridge::Clipboard::PasteFromClipboard(PlatformBridge::Clipboard::ClipboardDatatype::Text, data, &dataSize);

    if(0 == dataSize)
        return;

    pastedText = std::u32string(static_cast<char32_t*>(data), dataSize / sizeof(char32_t));
    filterPastedText(pastedText);

    if(pastedText.empty())
    {
        PlatformBridge::Clipboard::ClearClipboardDataBuffer();
        _textPasted = true;
        return;
    }

    if(_isSelected)
    {
        completeText = text->GetTextUTF32().substr(0, selectionStart);
        rightPart = text->GetTextUTF32().substr(selectionEnd);
        completeText += pastedText + rightPart;
        _isSelected = false;
        text->SetTextUTF32(completeText);
        setCaretFromBoundary(selectionStart + pastedText.size());
        _selectedPositionFirst = 0;
        _selectedPositionLast = 0;
        updateSelectedArea();
    }
    else
    {
        completeText = text->GetTextUTF32().substr(0, _caretPosition);
        rightPart = text->GetTextUTF32().substr(_caretPosition);
        completeText += pastedText + rightPart;
        text->SetTextUTF32(completeText);
        setCaretFromBoundary(_caretPosition + pastedText.size());
    }

    PlatformBridge::Clipboard::ClearClipboardDataBuffer();
    _textPasted = true;
    emitPaste();
    emitChange();
}

void RetroFuturaGUI::ITextInteraction::moveCaret()
{
    if(!getActiveText())
        return;

    if(!hasInputFocus())
        return;

    //One step per stroke, so a held arrow key moves at the system's repeat rate
    const uSize
        leftStrokeCount { getKeyStrokeCount(PB_KEY_LEFT) },
        rightStrokeCount { getKeyStrokeCount(PB_KEY_RIGHT) },
        upStrokeCount { getKeyStrokeCount(PB_KEY_UP) },
        downStrokeCount { getKeyStrokeCount(PB_KEY_DOWN) };

    for(uSize stroke = 0; stroke < leftStrokeCount; ++stroke)
        moveCaretLeft();

    for(uSize stroke = 0; stroke < rightStrokeCount; ++stroke)
        moveCaretRight();

    for(uSize stroke = 0; stroke < upStrokeCount; ++stroke)
        moveCaretUp();

    for(uSize stroke = 0; stroke < downStrokeCount; ++stroke)
        moveCaretDown();
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
    Text* text { getActiveText() };

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
    if((PlatformBridge::Input::IsKeyDown(PB_KEY_CONTROL_L) || PlatformBridge::Input::IsKeyDown(PB_KEY_CONTROL_R))
        && (PlatformBridge::Input::GetKeyPressState(PB_KEY_C) == PlatformBridge::KeyPressState::Press))
    {
        if(!_textCopied)
            CopySelectedText();

        return true;
    }

    _textCopied = false;
    return false;
}

bool RetroFuturaGUI::ITextInteraction::checkForTextCut()
{
    if((PlatformBridge::Input::IsKeyDown(PB_KEY_CONTROL_L) || PlatformBridge::Input::IsKeyDown(PB_KEY_CONTROL_R))
        && (PlatformBridge::Input::GetKeyPressState(PB_KEY_X) == PlatformBridge::KeyPressState::Press))
    {
        if(!_textCut)
            CutSelectedText();

        return true;
    }

    _textCut = false;
    return false;
}

bool RetroFuturaGUI::ITextInteraction::checkForTextPaste()
{
    if((PlatformBridge::Input::IsKeyDown(PB_KEY_CONTROL_L) || PlatformBridge::Input::IsKeyDown(PB_KEY_CONTROL_R))
        && (PlatformBridge::Input::GetKeyPressState(PB_KEY_V) == PlatformBridge::KeyPressState::Press))
    {
        if(!_textPasted)
            PasteSelectedText();

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
    Text* text { getActiveText() };

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

void RetroFuturaGUI::ITextInteraction::checkForEnterPress()
{
    const u32 enterPressCount { getEnterPressCount() };

    //Once per press: holding Enter repeats the line break (which comes with the typed text, not the signal!)
    if(enterPressCount == _enterPressCountSeen)
        return;

    _enterPressCountSeen = enterPressCount;
    emitEnterPressed();
    _enterPressed = true;
}

void RetroFuturaGUI::ITextInteraction::checkForTextInput()
{
    if(PlatformBridge::Input::GetLastKeySym() == _focusKeySym && PlatformBridge::Input::GetKeyPressCount(_focusKeySym) == _focusKeyPressCountSeen)
        return; //the key that was down when the focus came

    const std::u32string keyText { DoubleEncodedString::Utf8ToUtf32(PlatformBridge::Input::GetInputString()) };
    std::u32string characters {};

    //In the order typed, the system's repeats of held keys included: Backspace comes as \b and Enter as \r
    for(const char32_t codepoint : keyText)
    {
        if(U'\b' != codepoint && U'\r' != codepoint)
        {
            if(!isInvalidCodepoint(codepoint))
                characters += codepoint;

            continue;
        }

        insertText(characters);
        characters.clear();

        if(U'\b' == codepoint)
            deleteBackward();
        else
            insertLineBreak();
    }

    insertText(characters);
}

void RetroFuturaGUI::ITextInteraction::insertText(const std::u32string& characters)
{
    if(characters.empty())
        return;

    Text* text { getActiveText() };

    if(!text)
        return;

    if(_isSelected)
    {
        const uSize
            selectionStart { getMarkedStart() },
            selectionEnd { getMarkedEnd() };
        text->SetTextUTF32(text->GetTextUTF32().substr(0, selectionStart) + characters + text->GetTextUTF32().substr(selectionEnd));
        _selectedPositionFirst = 0;
        _selectedPositionLast = 0;
        _isSelected = false;
        updateSelectedArea();
        setCaretFromBoundary(selectionStart + characters.size());
    }
    else
    {
        std::u32string left { text->GetTextUTF32().substr(0, _caretPosition) };
        std::u32string right { text->GetTextUTF32().substr(_caretPosition) };
        text->SetTextUTF32(left + characters + right);
        _caretPosition += characters.size();
        deselect();
        updateCaretPosition();
    }

    emitChange();
}

void RetroFuturaGUI::ITextInteraction::deleteBackward()
{
    Text* text { getActiveText() };

    if(!text)
        return;

    if(0 == text->GetTextUTF32().size())
        return;

    if(0 == text->GetTextUTF32().front())
        return;

    if(_isSelected)
    {
        const uSize
            selectionStart { getMarkedStart() },
            selectionEnd { getMarkedEnd() };
        text->SetTextUTF32(text->GetTextUTF32().substr(0, selectionStart) + text->GetTextUTF32().substr(selectionEnd));
        _selectedPositionFirst = 0;
        _selectedPositionLast = 0;
        _isSelected = false;
        updateSelectedArea();
        setCaretFromBoundary(selectionStart);
        emitChange();
        return;
    }

    if(0 == _caretPosition)
        return;

    std::u32string left { text->GetTextUTF32().substr(0, _caretPosition - 1) };
    std::u32string right { text->GetTextUTF32().substr(_caretPosition) };

    text->SetTextUTF32(left + right);
    --_caretPosition;
    deselect();
    updateCaretPosition();
    emitChange();
}

void RetroFuturaGUI::ITextInteraction::editText()
{
    Text* text { getActiveText() };

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

    checkForTextInput();
    checkForEnterPress(); //after the text, so a multiline box's slot sees the line break
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
