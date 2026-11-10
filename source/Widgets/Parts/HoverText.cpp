#include "HoverText.hpp"
#include <array>

RetroFuturaGUI::HoverText::HoverText(Projection* projection)
    : _background(projection), _text(projection)
{
    _background.SetBuiltInColors(StyleKey {}, std::array { glm::vec4(0.0f, 0.0f, 0.0f, 0.75f) }, ColorState::Enabled);
    _background.SetBuiltInCornerRadii(StyleKey {}, glm::vec4(3.0f));
    _text.SetAlignment(TextAlignment::Center);
}

RetroFuturaGUI::Background& RetroFuturaGUI::HoverText::GetBackground()
{
    return _background;
}

const RetroFuturaGUI::Background& RetroFuturaGUI::HoverText::GetBackground() const
{
    return _background;
}

RetroFuturaGUI::WidgetText& RetroFuturaGUI::HoverText::GetText()
{
    return _text;
}

const RetroFuturaGUI::WidgetText& RetroFuturaGUI::HoverText::GetText() const
{
    return _text;
}

void RetroFuturaGUI::HoverText::EnableLineColor(const bool enable)
{
    _isLineColorEnabled = enable;

    if(enable)
        return;

    _text.SetColorState(ColorState::Enabled); //back from the line's color to its own
}

bool RetroFuturaGUI::HoverText::IsLineColorEnabled() const
{
    return _isLineColorEnabled;
}

void RetroFuturaGUI::HoverText::SetTransitionSettings(const TransitionSettings* settings)
{
    _background.SetTransitionSettings(settings);
    _text.SetTransitionSettings(settings);
}