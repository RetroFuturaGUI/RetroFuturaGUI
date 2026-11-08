#include "DropDownList.hpp"
#include <array>

RetroFuturaGUI::DropDownList::DropDownList(Projection* projection)
    : _background(projection), _highlight(projection), _border(projection)
{
    _background.SetColors(std::array { glm::vec4(0.25f, 0.25f, 0.8f, 1.0f) }, ColorState::Enabled);
    _background.SetColors(std::array { glm::vec4(0.1f, 0.1f, 0.45f, 1.0f) }, ColorState::Disabled);
    _background.SetColors(std::array { glm::vec4(0.3f, 0.3f, 0.9f, 1.0f) }, ColorState::Hover);
    _background.SetColors(std::array { glm::vec4(0.4f, 0.4f, 1.0f, 1.0f) }, ColorState::Clicked);
    _border.SetColors(std::array { glm::vec4(0.7f, 0.7f, 0.7f, 1.0f) }, ColorState::Enabled);
    _border.SetColors(std::array { glm::vec4(0.3f, 0.3f, 0.3f, 1.0f) }, ColorState::Disabled);
    _border.SetColors(std::array { glm::vec4(0.85f, 0.85f, 0.85f, 1.0f) }, ColorState::Hover);
    _border.SetColors(std::array { glm::vec4(1.0f, 1.0f, 1.0f, 1.0f) }, ColorState::Clicked);
    _highlight.SetColors(std::array { glm::vec4(0.3f, 0.9f, 2.0f, 1.0f) }, ColorState::Enabled);
}

RetroFuturaGUI::Background& RetroFuturaGUI::DropDownList::GetBackground()
{
    return _background;
}

const RetroFuturaGUI::Background& RetroFuturaGUI::DropDownList::GetBackground() const
{
    return _background;
}

RetroFuturaGUI::Border& RetroFuturaGUI::DropDownList::GetBorder()
{
    return _border;
}

const RetroFuturaGUI::Border& RetroFuturaGUI::DropDownList::GetBorder() const
{
    return _border;
}

RetroFuturaGUI::Background& RetroFuturaGUI::DropDownList::GetHighlight()
{
    return _highlight;
}

const RetroFuturaGUI::Background& RetroFuturaGUI::DropDownList::GetHighlight() const
{
    return _highlight;
}

void RetroFuturaGUI::DropDownList::SetColorState(const ColorState state)
{
    _background.SetColorState(state);
    _border.SetColorState(state);
}

void RetroFuturaGUI::DropDownList::SetTransitionSettings(const TransitionSettings* settings)
{
    _background.SetTransitionSettings(settings);
    _border.SetTransitionSettings(settings);
    _highlight.SetTransitionSettings(settings);
}