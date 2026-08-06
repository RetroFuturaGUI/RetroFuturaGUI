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

void RetroFuturaGUI::Label::SetEnabled(const bool enable)
{
    _isEnabledFlag = enable;

    if(_isEnabledFlag)
    {
        _onEnableAsync.EmitAsync();
        _onEnable.Emit();
        setColors(ColorState::Enabled);
        return;
    }

    _onDisableAsync.EmitAsync();
    _onDisable.Emit();
    setColors(ColorState::Disabled);
}

void RetroFuturaGUI::Label::setColors(const ColorState state)
{
    _textColorState = state;
    setTextColors();
}

void RetroFuturaGUI::Label::SetSize(const glm::vec3& size)
{
    IWidget::SetSize(size);

    if(_text)
        _text->SetParentSize(glm::vec2(size.x, size.y));
}

void RetroFuturaGUI::Label::SetPosition(const glm::vec3& position)
{
    IWidget::SetPosition(position);

    if(_text)
        _text->SetPosition(position);
}

void RetroFuturaGUI::Label::SetRotation(const glm::vec3& rotation)
{
    _rotation = rotation;

    if(_text)
        _text->SetRotation(rotation);
}