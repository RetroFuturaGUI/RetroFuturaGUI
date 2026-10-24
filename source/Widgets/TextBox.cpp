#include "TextBox.hpp"

RetroFuturaGUI::TextBox::TextBox(const std::string& name, Projection* projection, IWidget* parentWidget, const WidgetTypeID parentWidgetTypeID, GLFWwindow* parentWindow)
   : ITextBox(name, projection, parentWidget, parentWidgetTypeID, parentWindow)
{
    _widgetTypeID = WidgetTypeID::TextBox;
}

void RetroFuturaGUI::TextBox::emitChange()
{
    syncValueFromText(GetText());
    ITextEditable::emitChange();
}

void RetroFuturaGUI::TextBox::renderValueText(std::string_view text, const bool emitSignal)
{
    ITextProperties::SetText(text, emitSignal);
    placeCaretAfterText(); //typing never comes here: it stores what was typed without showing it again
}
