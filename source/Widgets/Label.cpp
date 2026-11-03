#include "Label.hpp"

RetroFuturaGUI::Label::Label(const std::string& name, Projection* projection, IWidget* parentWidget, const WidgetTypeID parentWidgetTypeID, GLFWwindow* parentWindow)
    : IWidget(name, projection, parentWidget, parentWidgetTypeID, parentWindow)
{
    _widgetTypeID = WidgetTypeID::Label;
    _text = std::make_unique<Text>(projection);
    useTransitions({ _text.get() });
}

void RetroFuturaGUI::Label::Draw()
{
    drawText();
}

void RetroFuturaGUI::Label::setColors(const ColorState state)
{
    _textColorState = state;
    setTextColors();
}

void RetroFuturaGUI::Label::applySize(const glm::vec3& size)
{
    if(!_text)
        return;

    IWidget::applySize(size);

    _text->SetParentSize(glm::vec2(size.x, size.y));
}

void RetroFuturaGUI::Label::applyPosition(const glm::vec3& position)
{
    if(!_text)
        return;

    IWidget::applyPosition(position);

    _text->SetPosition(position);
}
