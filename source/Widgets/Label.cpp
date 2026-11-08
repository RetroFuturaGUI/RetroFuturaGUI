#include "Label.hpp"

RetroFuturaGUI::Label::Label(std::string_view name, Projection* projection, IWidget* parentWidget, const WidgetTypeID parentWidgetTypeID, GLFWwindow* parentWindow)
    : IWidget(name, projection, parentWidget, parentWidgetTypeID, parentWindow)
{
    _widgetTypeID = WidgetTypeID::Label;
    _text.SetTransitionSettings(&_transitionSettings);
}

void RetroFuturaGUI::Label::Draw()
{
    _text.Draw();
}

void RetroFuturaGUI::Label::setColors(const ColorState state)
{
    _text.SetColorState(ColorState::Selected == state ? ColorState::Enabled : state); //a label can't be selected.
}

void RetroFuturaGUI::Label::applySize(const glm::vec3& size)
{
    IWidget::applySize(size);

    _text.SetSize(glm::vec2(size.x, size.y));
}

void RetroFuturaGUI::Label::applyPosition(const glm::vec3& position)
{
    IWidget::applyPosition(position);

    _text.SetPosition(position);
}

RetroFuturaGUI::WidgetText& RetroFuturaGUI::Label::GetText()
{
    return _text;
}

const RetroFuturaGUI::WidgetText& RetroFuturaGUI::Label::GetText() const
{
    return _text;
}