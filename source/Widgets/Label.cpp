#include "Label.hpp"

RetroFuturaGUI::Label::Label(const std::string& name, Projection* projection, IWidget* parentWidget, const WidgetTypeID parentWidgetTypeID, GLFWwindow* parentWindow)
    : IWidget(name, projection, parentWidget, parentWidgetTypeID, parentWindow)
{
    _widgetTypeID = WidgetTypeID::Label;
    _text = std::make_unique<Text>(projection);
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

void RetroFuturaGUI::Label::SetSize(const glm::vec3& size)
{
    if(!_text)
        return;

    IWidget::SetSize(size);

    _text->SetParentSize(glm::vec2(size.x, size.y));
}

void RetroFuturaGUI::Label::SetPosition(const glm::vec3& position)
{
    if(!_text)
        return;

    IWidget::SetPosition(position);

    _text->SetPosition(position);
}

void RetroFuturaGUI::Label::SetRotation(const glm::vec3& rotation)
{
    if(!_text)
        return;

    _rotation = rotation;

    _text->SetRotation(rotation);
}