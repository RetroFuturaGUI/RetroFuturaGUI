#include "MultilineTextBox.hpp"

RetroFuturaGUI::MultilineTextBox::MultilineTextBox(const std::string& name, Projection* projection, IWidget* parentWidget, const WidgetTypeID parentWidgetTypeID, GLFWwindow* parentWindow)
   : ITextBox(name, projection, parentWidget, parentWidgetTypeID, parentWindow)
{
    _widgetTypeID = WidgetTypeID::MultilineTextBox;

    _text.GetElement().SetAnchorTop(true);
    _placeholder.GetElement().SetAnchorTop(true);
}

void RetroFuturaGUI::MultilineTextBox::moveCaretUp()
{
    Text* text { getActiveText() };

    if(!text)
        return;

    //Already on the first line to the start of the text. in columns, a step up the column
    deselect();
    _caretPosition = text->GetBoundaryBeside(_caretPosition, CaretMove::Up);
    updateCaretPosition();
}

void RetroFuturaGUI::MultilineTextBox::moveCaretDown()
{
    Text* text { getActiveText() };

    if(!text)
        return;

    //Already on the last line, to the end of the text
    deselect();
    _caretPosition = text->GetBoundaryBeside(_caretPosition, CaretMove::Down);
    updateCaretPosition();
}

void RetroFuturaGUI::MultilineTextBox::insertLineBreak()
{
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

void RetroFuturaGUI::MultilineTextBox::filterPastedText(std::u32string& text) const
{
    std::erase_if(text, [](const char32_t codepoint) { return U'\n' != codepoint && isInvalidCodepoint(codepoint); });
}