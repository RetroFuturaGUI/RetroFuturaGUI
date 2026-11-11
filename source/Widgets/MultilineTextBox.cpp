#include "MultilineTextBox.hpp"

RetroFuturaGUI::MultilineTextBox::MultilineTextBox(std::string_view name, Projection* projection, IWidget* parentWidget, const WidgetTypeID parentWidgetTypeID, GLFWwindow* parentWindow)
   : ITextBox(name, projection, parentWidget, parentWidgetTypeID, parentWindow, TextLines::Multiple)
{
    _widgetTypeID = WidgetTypeID::MultilineTextBox;

    _text.GetElement().SetAnchor(TextAnchor::Top);
    _placeholder.GetElement().SetAnchor(TextAnchor::Top);
}