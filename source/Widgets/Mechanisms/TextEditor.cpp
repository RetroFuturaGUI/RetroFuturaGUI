#include "TextEditor.hpp"
#include "PlatformBridge.hpp"
#include "DoubleEncodedString.hpp"
#include "Window.hpp"
#include <algorithm>
#include <utility>

#if defined(TARGET_PLATFORM_LINUX)
    #define GLFW_EXPOSE_NATIVE_X11
#elif defined(TARGET_PLATFORM_WINDOWS)
    #define GLFW_EXPOSE_NATIVE_WIN32
#endif
#include <GLFW/glfw3native.h>

RetroFuturaGUI::TextEditor::~TextEditor()
{
    ReleaseKeyboardFocus();
}

RetroFuturaGUI::TextEditor::TextEditor(Projection* projection, GLFWwindow* parentWindow, const TextLines lines, TextEditorHooks hooks)
    : _parentWindow(parentWindow), _lines(lines), _hooks(std::move(hooks)), _caret(projection), _selectedArea(projection)
{
    _caret.SetSize(glm::vec2(_caretThickness, 0.0f)); //updateCaretPosition sets caret size by text size
}

RetroFuturaGUI::Text* RetroFuturaGUI::TextEditor::getActiveText() const
{
    return _hooks._GetActiveText();
}

RetroFuturaGUI::TextEditArea RetroFuturaGUI::TextEditor::getTextArea() const
{
    return _hooks._GetTextArea();
}

bool RetroFuturaGUI::TextEditor::isTextReadOnly() const
{
    return _hooks._IsReadOnly();
}

void RetroFuturaGUI::TextEditor::emitChange()
{
    _hooks._OnChange();
}

void RetroFuturaGUI::TextEditor::SetTransitionSettings(const TransitionSettings* settings)
{
    _caret.SetTransitionSettings(settings);
    _selectedArea.SetTransitionSettings(settings);
}

void RetroFuturaGUI::TextEditor::Interact()
{
    editText();
    moveCaret();

    if(_caret.NeverBlinks())
        return;

    updateCaretBlink();
}

void RetroFuturaGUI::TextEditor::BeginEditing()
{
    _editingEnabled = true;
    _showCaret = true;
    TakeKeyboardFocus();
}

void RetroFuturaGUI::TextEditor::EndEditing()
{
    _editingEnabled = false;
    _showCaret = false;
}

bool RetroFuturaGUI::TextEditor::IsEditing() const
{
    return _editingEnabled;
}

void RetroFuturaGUI::TextEditor::StartMarking(const uSize boundary)
{
    _isMarking = true;
    _selectedPositionFirst = _selectedPositionLast = boundary;
    setCaretFromBoundary(boundary);
    updateSelectedArea();
}

void RetroFuturaGUI::TextEditor::MarkTo(const uSize boundary)
{
    _selectedPositionLast = boundary;
    setCaretFromBoundary(boundary);
    updateSelectedArea();
}

void RetroFuturaGUI::TextEditor::StopMarking()
{
    _isMarking = false;
}

bool RetroFuturaGUI::TextEditor::IsMarking() const
{
    return _isMarking;
}

void RetroFuturaGUI::TextEditor::SelectFromStart(const uSize characterCount)
{
    _isMarking = false;
    _selectedPositionFirst = 0;
    _selectedPositionLast = characterCount;
    _isSelected = 0 < characterCount;
    setCaretFromBoundary(characterCount);
    updateSelectedArea();
    resetCaretBlink();
}

void RetroFuturaGUI::TextEditor::ClearSelection()
{
    _selectedPositionFirst = 0;
    _selectedPositionLast = 0;
    deselect();
}

bool RetroFuturaGUI::TextEditor::WasEnterPressed() const
{
    return _enterPressed;
}

void RetroFuturaGUI::TextEditor::insertLineBreak()
{
    if(TextLines::Single == _lines)
        return;

    Text* text { getActiveText() };

    if(!text)
        return;

    std::u32string left { text->GetTextUTF32().substr(0, _caretPosition) };
    std::u32string right { text->GetTextUTF32().substr(_caretPosition) };
    text->SetTextUTF32(left + U'\n' + right);
    ++_caretPosition;
    deselect();
    updateCaretPosition();
    emitChange();
}

void RetroFuturaGUI::TextEditor::filterPastedText(std::u32string& text) const
{
    if(TextLines::Multiple == _lines)
    {
        std::erase_if(text, [](const char32_t codepoint) { return U'\n' != codepoint && isInvalidCodepoint(codepoint); });
        return;
    }

    std::ranges::replace(text, U'\n', U' ');
    std::erase_if(text, isInvalidCodepoint);
}

void RetroFuturaGUI::TextEditor::TakeKeyboardFocus()
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

void RetroFuturaGUI::TextEditor::ReleaseKeyboardFocus()
{
    if(this != _keyboardFocus)
        return;

    _keyboardFocus = nullptr;
    PlatformBridge::Input::SetActiveDisplay(nullptr);
    PlatformBridge::Input::SetActiveWindow(0);
}

bool RetroFuturaGUI::TextEditor::HasKeyboardFocus() const
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

void RetroFuturaGUI::TextEditor::resetCaretBlink()
{
    _caretBlinkState = true;
    _millisecondsPassed = std::chrono::high_resolution_clock::now();
}

void RetroFuturaGUI::TextEditor::deselect()
{
    _isMarking = false;
    _isSelected = false;
}

void RetroFuturaGUI::TextEditor::PlaceCaretAfterText()
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

uSize RetroFuturaGUI::TextEditor::getMarkedStart() const
{
    return _selectedPositionFirst < _selectedPositionLast ? _selectedPositionFirst : _selectedPositionLast;
}

uSize RetroFuturaGUI::TextEditor::getMarkedEnd() const
{
    return _selectedPositionFirst < _selectedPositionLast ? _selectedPositionLast : _selectedPositionFirst;
}

uSize RetroFuturaGUI::TextEditor::getKeyStrokeCount(const u32 key) const
{
    if(key == _focusKeySym && PlatformBridge::Input::GetKeyPressCount(key) == _focusKeyPressCountSeen)
        return 0; //the key that was down when the focus came

    return PlatformBridge::Input::GetKeyStrokeCount(key);
}

u32 RetroFuturaGUI::TextEditor::getEnterPressCount()
{
    return PlatformBridge::Input::GetKeyPressCount(PB_KEY_RETURN)
        + PlatformBridge::Input::GetKeyPressCount(PB_KEY_KP_ENTER)
        + PlatformBridge::Input::GetKeyPressCount(PB_KEY_ISO_ENTER);
}

const std::string& RetroFuturaGUI::TextEditor::GetCopiedText() const
{
    return _copiedText;
}

const char* RetroFuturaGUI::TextEditor::GetSelection() const
{
    const Text* text { getActiveText() };

    if(!text)
        return nullptr;

    if(!_isSelected)
        return nullptr;

    //The marks count characters and the string is UTF-8, so the bytes before the selection are measured by encoding them
    const uSize byteOffset { DoubleEncodedString::Utf32ToUtf8(text->GetTextUTF32().substr(0, getMarkedStart())).size() };
    return text->GetTextUTF8().c_str() + byteOffset;
}

std::string RetroFuturaGUI::TextEditor::GetSelectedString() const
{
    const Text* text { getActiveText() };

    if(!text)
        return {};

    if(!_isSelected)
        return {};

    const uSize selectionStart { getMarkedStart() };
    return DoubleEncodedString::Utf32ToUtf8(text->GetTextUTF32().substr(selectionStart, getMarkedEnd() - selectionStart));
}

void RetroFuturaGUI::TextEditor::CopySelectedText()
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

void RetroFuturaGUI::TextEditor::CutSelectedText()
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

void RetroFuturaGUI::TextEditor::PasteSelectedText()
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

void RetroFuturaGUI::TextEditor::moveCaret()
{
    if(!getActiveText())
        return;

    if(!HasKeyboardFocus())
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

void RetroFuturaGUI::TextEditor::moveCaretLeft()
{
    Text* text { getActiveText() };

    if(!text)
        return;

    //The way the glyphs are drawn, so right-to-left text is walked as it reads; in columns, to the next column that way
    deselect();
    _caretPosition = text->GetBoundaryBeside(_caretPosition, CaretMove::Left);
    updateCaretPosition();
}

void RetroFuturaGUI::TextEditor::moveCaretRight()
{
    Text* text { getActiveText() };

    if(!text)
        return;

    deselect();
    _caretPosition = text->GetBoundaryBeside(_caretPosition, CaretMove::Right);
    updateCaretPosition();
}

void RetroFuturaGUI::TextEditor::moveCaretUp()
{
    Text* text { getActiveText() };

    if(!text)
        return;

    //A single line has nowhere to go up or down, unless it runs down a column.
    if(TextLines::Single == _lines && TextFlow::Horizontal == text->GetTextFlow())
        return;

    deselect();
    _caretPosition = text->GetBoundaryBeside(_caretPosition, CaretMove::Up);
    updateCaretPosition();
}

void RetroFuturaGUI::TextEditor::moveCaretDown()
{
    Text* text { getActiveText() };

    if(!text)
        return;

    if(TextLines::Single == _lines && TextFlow::Horizontal == text->GetTextFlow())
        return;

    deselect();
    _caretPosition = text->GetBoundaryBeside(_caretPosition, CaretMove::Down);
    updateCaretPosition();
}

void RetroFuturaGUI::TextEditor::setCaretFromBoundary(const uSize boundary)
{
    _caretPosition = boundary;
    updateCaretPosition();
}

bool RetroFuturaGUI::TextEditor::checkForTextCopy()
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

bool RetroFuturaGUI::TextEditor::checkForTextCut()
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

bool RetroFuturaGUI::TextEditor::checkForTextPaste()
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

bool RetroFuturaGUI::TextEditor::checkForSelectAllText()
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

void RetroFuturaGUI::TextEditor::checkForEnterPress()
{
    const u32 enterPressCount { getEnterPressCount() };

    //Once per press: holding Enter repeats the line break (which comes with the typed text, not the signal!)
    if(enterPressCount == _enterPressCountSeen)
        return;

    _enterPressCountSeen = enterPressCount;
    emitEnterPressed();
    _enterPressed = true;
}

void RetroFuturaGUI::TextEditor::checkForTextInput()
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

void RetroFuturaGUI::TextEditor::insertText(const std::u32string& characters)
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

void RetroFuturaGUI::TextEditor::deleteBackward()
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

void RetroFuturaGUI::TextEditor::editText()
{
    Text* text { getActiveText() };

    if(!text)
        return;

    if(!HasKeyboardFocus())
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

void RetroFuturaGUI::TextEditor::emitEnterPressed()
{
    _onEnterPressedAsync.EmitAsync();
    _onEnterPressed.Emit();
}

void RetroFuturaGUI::TextEditor::emitEnterRelease()
{
    _onEnterReleasedAsync.EmitAsync();
    _onEnterReleased.Emit();
}

void RetroFuturaGUI::TextEditor::emitCopy()
{
    _onCopyAsync.EmitAsync();
    _onCopy.Emit();
}

void RetroFuturaGUI::TextEditor::emitPaste()
{
    _onPasteAsync.EmitAsync();
    _onPaste.Emit();
}

void RetroFuturaGUI::TextEditor::Connect_OnEnterPressed(const typename Signal<>::Slot& slot, const bool async)
{
    if(async)
        _onEnterPressedAsync.Connect(slot);
    else
        _onEnterPressed.Connect(slot);
}

void RetroFuturaGUI::TextEditor::Connect_OnEnterReleased(const typename Signal<>::Slot& slot, const bool async)
{
    if(async)
        _onEnterReleasedAsync.Connect(slot);
    else
        _onEnterReleased.Connect(slot);
}

void RetroFuturaGUI::TextEditor::Connect_OnCopy(const typename Signal<>::Slot& slot, const bool async)
{
    if(async)
        _onCopyAsync.Connect(slot);
    else
        _onCopy.Connect(slot);
}

void RetroFuturaGUI::TextEditor::Connect_OnPaste(const typename Signal<>::Slot& slot, const bool async)
{
    if(async)
        _onPasteAsync.Connect(slot);
    else
        _onPaste.Connect(slot);
}

void RetroFuturaGUI::TextEditor::Disconnect_OnEnterPressed(const typename Signal<>::Slot& slot)
{
    _onEnterPressedAsync.Disconnect(slot);
    _onEnterPressed.Disconnect(slot);
}

void RetroFuturaGUI::TextEditor::Disconnect_OnEnterReleased(const typename Signal<>::Slot& slot)
{
    _onEnterReleasedAsync.Disconnect(slot);
    _onEnterReleased.Disconnect(slot);
}

void RetroFuturaGUI::TextEditor::Disconnect_OnCopy(const typename Signal<>::Slot& slot)
{
    _onCopyAsync.Disconnect(slot);
    _onCopy.Disconnect(slot);
}

void RetroFuturaGUI::TextEditor::Disconnect_OnPaste(const typename Signal<>::Slot& slot)
{
    _onPasteAsync.Disconnect(slot);
    _onPaste.Disconnect(slot);
}

void RetroFuturaGUI::TextEditor::updateCaretBlink()
{
    if(!_showCaret)
    {
        resetCaretBlink();
        return;
    }

    const f64 elapsedMilliseconds { std::chrono::duration<f64, std::milli>(std::chrono::high_resolution_clock::now() - _millisecondsPassed).count() };

    if(elapsedMilliseconds < _caret.GetBlinkTime())
        return;

    _caretBlinkState = !_caretBlinkState;
    _millisecondsPassed = std::chrono::high_resolution_clock::now();
}

void RetroFuturaGUI::TextEditor::updateCaretPosition()
{
    Text* text { getActiveText() };

    if(!text)
        return;

    const f32 fullHeight { getCaretHeight() };
    glm::vec3 caretPosition { text->GetBoundaryPosition(_caretPosition, fullHeight) };

    //In a column the caret lies across it, as wide as the column, between the characters above and below
    if(TextFlow::Horizontal != text->GetTextFlow())
    {
        const f32
            halfLength { text->GetLineHeight() * 0.5f },
            clippedLeftX { ClampToTextBounds(caretPosition.x - halfLength) },
            clippedRightX { ClampToTextBounds(caretPosition.x + halfLength) },
            clippedWidth { clippedRightX - clippedLeftX };

        _isCaretInView = 0.0f < clippedWidth && clampToTextBoundsY(caretPosition.y) == caretPosition.y;
        _caret.SetSize(glm::vec2(clippedWidth, _caretThickness));
        _caret.SetPosition(glm::vec3(clippedLeftX + clippedWidth * 0.5f, caretPosition.y, caretPosition.z));
        resetCaretBlink();
        return;
    }

    caretPosition.x = keepCaretVisible(caretPosition.x, _caretThickness * 0.5f);

    const f32
        clippedTopY { clampToTextBoundsY(caretPosition.y + fullHeight * 0.5f) },
        clippedBottomY { clampToTextBoundsY(caretPosition.y - fullHeight * 0.5f) },
        clippedHeight { clippedTopY - clippedBottomY };

    _isCaretInView = 0.0f < clippedHeight; //false while the caret's line is outside the text area
    caretPosition.y = clippedBottomY + clippedHeight * 0.5f;
    _caret.SetSize(glm::vec2(_caretThickness, clippedHeight));
    _caret.SetPosition(caretPosition);
    resetCaretBlink();
}

void RetroFuturaGUI::TextEditor::updateSelectedArea()
{
    _isSelected = false;
    _selectedLineAreas.clear();

    Text* text { getActiveText() };

    if(!text)
        return;

    const uSize
        left { getMarkedStart() },
        right { getMarkedEnd() };

    if(left == right) //nothing selected
        return;

    //One box per stretch drawn without a break, where the caret sits (it may be clipped shorter): mixed right-to-left and
    //left-to-right text splits a line's selection, and in columns the boxes run down them
    for(const TextSelectionArea& area : text->GetSelectionAreas(left, right, getCaretHeight()))
    {
        const f32
            clippedLeftX { ClampToTextBounds(area._Center.x - area._Size.x * 0.5f) },
            clippedRightX { ClampToTextBounds(area._Center.x + area._Size.x * 0.5f) },
            width { clippedRightX - clippedLeftX },
            clippedTopY { clampToTextBoundsY(area._Center.y + area._Size.y * 0.5f) },
            clippedBottomY { clampToTextBoundsY(area._Center.y - area._Size.y * 0.5f) },
            height { clippedTopY - clippedBottomY };

        if(0.0f >= width || 0.0f >= height) //skip whatever is outside the visible text area
            continue;

        _selectedLineAreas.push_back(SelectedArea::LineArea { ._Center = glm::vec2(clippedLeftX + width * 0.5f, clippedBottomY + height * 0.5f), ._Size = glm::vec2(width, height) });
    }

    _isSelected = !_selectedLineAreas.empty();
}

void RetroFuturaGUI::TextEditor::DrawSelectedArea()
{
    if(!_isSelected)
        return;

    _selectedArea.Draw(_selectedLineAreas);
}

void RetroFuturaGUI::TextEditor::DrawCaret()
{
    if(!_showCaret)
        return;

    if(!_isCaretInView)
        return;

    if(_caret.NeverBlinks() || _caretBlinkState)
        _caret.Draw();
}

f32 RetroFuturaGUI::TextEditor::ClampToTextBounds(const f32 worldX, const f32 halfExtent) const
{
    const TextEditArea area { getTextArea() };
    const f32
        left { area._Left + halfExtent },
        right { area._Right - halfExtent };

    if(left > right) //the requested extent is wider than the text area itself
        return (area._Left + area._Right) * 0.5f;

    if(worldX < left)
        return left;

    if(worldX > right)
        return right;

    return worldX;
}

f32 RetroFuturaGUI::TextEditor::clampToTextBoundsY(const f32 worldY) const
{
    const TextEditArea area { getTextArea() };

    if(worldY < area._Bottom)
        return area._Bottom;

    if(worldY > area._Top)
        return area._Top;

    return worldY;
}

f32 RetroFuturaGUI::TextEditor::keepCaretVisible(const f32 worldX, const f32 halfExtent)
{
    Text* text { getActiveText() };

    if(!text)
        return worldX;

    const TextEditArea area { getTextArea() };
    const f32
        left { area._Left + halfExtent },
        right { area._Right - halfExtent },
        scrollOffset { text->GetScrollOffset() };

    if(left > right) //the requested extent is wider than the text area itself
        return (area._Left + area._Right) * 0.5f;

    if(worldX > right) //scroll the text left so the caret lands exactly on the edge
    {
        text->SetScrollOffset(scrollOffset + worldX - right);
        return right;
    }

    if(worldX < left && 0.0f < scrollOffset) //scroll back right, as far as there's room to
    {
        const f32
            wantedScrollOffset { scrollOffset + worldX - left },
            newScrollOffset { 0.0f < wantedScrollOffset ? wantedScrollOffset : 0.0f };

        text->SetScrollOffset(newScrollOffset);
        return worldX + scrollOffset - newScrollOffset; //shift the caret by as much as the text scrolled
    }

    return ClampToTextBounds(worldX, halfExtent); //nothing left to scroll - fall back to a hard clamp
}

f32 RetroFuturaGUI::TextEditor::getCaretHeight() const
{
    Text* text { getActiveText() };

    if(!text)
        return 0.0f;

    return text->GetGlyphSize() * _caretHeightFactor;
}

RetroFuturaGUI::Caret& RetroFuturaGUI::TextEditor::GetCaret()
{
    return _caret;
}

const RetroFuturaGUI::Caret& RetroFuturaGUI::TextEditor::GetCaret() const
{
    return _caret;
}

RetroFuturaGUI::SelectedArea& RetroFuturaGUI::TextEditor::GetSelectedArea()
{
    return _selectedArea;
}

const RetroFuturaGUI::SelectedArea& RetroFuturaGUI::TextEditor::GetSelectedArea() const
{
    return _selectedArea;
}