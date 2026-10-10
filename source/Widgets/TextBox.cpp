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
