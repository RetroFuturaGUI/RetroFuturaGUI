#include "TextBox.hpp"

RetroFuturaGUI::TextBox::TextBox(const std::string& name, Projection* projection, IWidget* parentWidget, const WidgetTypeID parentWidgetTypeID, GLFWwindow* parentWindow)
   : ITextBox(name, projection, parentWidget, parentWidgetTypeID, parentWindow)
{
    _widgetTypeID = WidgetTypeID::TextBox;
}

void RetroFuturaGUI::TextBox::textChanged(const WidgetTextChange change)
{
    if(WidgetTextChange::String != change)
        return;

    //Text set by code reaches the value store too, and the caret stays inside it. SetValue renders without coming through here
    updateValueStore(_text.GetString());
    placeCaretAfterText();
}

void RetroFuturaGUI::TextBox::emitChange()
{
    syncValueFromText(_text.GetString());
    ITextEditable::emitChange();
}

void RetroFuturaGUI::TextBox::renderValueText(std::string_view text, const bool emitSignal)
{
    _text.GetElement().SetTextUTF8(text);

    if(emitSignal)
        _text.EmitChange();
    placeCaretAfterText(); //typing never comes here: it stores what was typed without showing it again
}