#include "MultilineTextBox.hpp"

RetroFuturaGUI::MultilineTextBox::MultilineTextBox(const std::string& name, Projection* projection, IWidget* parentWidget, const WidgetTypeID parentWidgetTypeID, GLFWwindow* parentWindow)
   : ITextBox(name, projection, parentWidget, parentWidgetTypeID, parentWindow)
{
    _widgetTypeID = WidgetTypeID::MultilineTextBox;

    if(_text)
        _text->SetAnchorTop(true);

    if(_placeholderText)
        _placeholderText->SetAnchorTop(true);
}

void RetroFuturaGUI::MultilineTextBox::moveCaretUp()
{
    Text* text { activeText() };

    if(!text)
        return;

    deselect();
    const uSize currentLine { text->GetBoundaryLine(_caretPosition) };

    if(0 == currentLine)
        _caretPosition = 0; //already on the first line: go to the start of the text
    else
        _caretPosition = text->GetBoundaryOnLine(currentLine - 1, text->GetBoundaryPosition(_caretPosition, 0.0f).x); //the caret size only affects y

    updateCaretPosition();
}

void RetroFuturaGUI::MultilineTextBox::moveCaretDown()
{
    Text* text { activeText() };

    if(!text)
        return;

    deselect();
    const uSize currentLine { text->GetBoundaryLine(_caretPosition) };
    _caretPosition = text->GetBoundaryOnLine(currentLine + 1, text->GetBoundaryPosition(_caretPosition, 0.0f).x); //past the last line, GetBoundaryOnLine returns the end of the text
    updateCaretPosition();
}

void RetroFuturaGUI::MultilineTextBox::insertLineBreak()
{
    Text* text { activeText() };

    if(!text)
        return;

    std::u32string left { text->GetTextUTF32().substr(0, _caretPosition) };
    std::u32string right { text->GetTextUTF32().substr(_caretPosition) };
    text->SetTextUTF32(left + U'\n' + right);
    ++_caretPosition;
    deselect();
    updateCaretPosition();
    emitChange();
    _keyRepeatText = U"\n"; //from the next frame on, checkForKeyRepeat repeats the line break while Enter is held
    _keyHoldFrames = 0;
}

void RetroFuturaGUI::MultilineTextBox::filterPastedText(std::u32string& text) const
{
    std::erase_if(text, [](const char32_t codepoint) { return U'\n' != codepoint && isInvalidCodepoint(codepoint); });
}