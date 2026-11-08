#include "Box.hpp"

RetroFuturaGUI::Box::Box(Projection* projection)
    : _background(projection), _border(projection)
{
}

RetroFuturaGUI::Background& RetroFuturaGUI::Box::GetBackground()
{
    return _background;
}

const RetroFuturaGUI::Background& RetroFuturaGUI::Box::GetBackground() const
{
    return _background;
}

RetroFuturaGUI::Border& RetroFuturaGUI::Box::GetBorder()
{
    return _border;
}

const RetroFuturaGUI::Border& RetroFuturaGUI::Box::GetBorder() const
{
    return _border;
}

void RetroFuturaGUI::Box::Draw()
{
    _background.Draw();
    _border.Draw();
}

void RetroFuturaGUI::Box::Place(const glm::vec2& size, const glm::vec3& position)
{
    _background.SetSize(size);
    _background.SetPosition(position);
    _border.SetSize(size);
    _border.SetPosition(position + glm::vec3(0.0f, 0.0f, 0.01f));
}

void RetroFuturaGUI::Box::SetColorState(const ColorState state)
{
    _background.SetColorState(state);
    _border.SetColorState(state);
}