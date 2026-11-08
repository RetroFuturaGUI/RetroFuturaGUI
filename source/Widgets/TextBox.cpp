#include "TextBox.hpp"

RetroFuturaGUI::TextBox::TextBox(std::string_view name, Projection* projection, IWidget* parentWidget, const WidgetTypeID parentWidgetTypeID, GLFWwindow* parentWindow)
   : ITextBox(name, projection, parentWidget, parentWidgetTypeID, parentWindow, TextLines::Single)
{
    _widgetTypeID = WidgetTypeID::TextBox;
}

void RetroFuturaGUI::TextBox::textChanged(const WidgetTextChange change)
{
    if(WidgetTextChange::String != change)
        return;

    //Text set by code reaches the value store too, and the caret stays inside it. SetValue renders without coming through here
    updateValueStore(_text.GetString());
    _editor.PlaceCaretAfterText();
}

void RetroFuturaGUI::TextBox::textEdited()
{
    syncValueFromText(_text.GetString());
    ITextBox::textEdited();
}

void RetroFuturaGUI::TextBox::renderValueText(std::string_view text, const bool emitSignal)
{
    _text.GetElement().SetTextUTF8(text);

    if(emitSignal)
        _text.EmitChange();
    _editor.PlaceCaretAfterText(); //typing never comes here. It stores what was typed without showing it again
}