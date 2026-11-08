#include "ColorPickerIndicator.hpp"

RetroFuturaGUI::ColorPickerIndicator::ColorPickerIndicator(Projection* projection)
    : _dot(projection), _border(projection)
{
    _dot.SetColors(_dotColors);
    _border.SetWidth(2.0f);
}

RetroFuturaGUI::Border& RetroFuturaGUI::ColorPickerIndicator::GetBorder()
{
    return _border;
}

const RetroFuturaGUI::Border& RetroFuturaGUI::ColorPickerIndicator::GetBorder() const
{
    return _border;
}

void RetroFuturaGUI::ColorPickerIndicator::SetColor(const glm::vec4& color)
{
    _dotColors[0] = glm::vec4(glm::vec3(color), 1.0f);
}

void RetroFuturaGUI::ColorPickerIndicator::SetDiameter(const f32 diameter)
{
    _dot.SetSize(glm::vec2(diameter));
    _dot.SetCornerRadii(glm::vec4(0.5f * diameter));
    _border.SetSize(glm::vec2(diameter));
    _border.SetCornerRadii(glm::vec4(0.5f * diameter));
}

void RetroFuturaGUI::ColorPickerIndicator::SetPosition(const glm::vec3& position)
{
    _dot.SetPosition(position);
    _border.SetPosition(position + glm::vec3(0.0f, 0.0f, 0.01f));
}

glm::vec2 RetroFuturaGUI::ColorPickerIndicator::GetSize() const
{
    return _dot.GetSize();
}

glm::vec3 RetroFuturaGUI::ColorPickerIndicator::GetPosition() const
{
    return _dot.GetPosition();
}

void RetroFuturaGUI::ColorPickerIndicator::Draw()
{
    _dot.Draw();
    _border.Draw();
}

void RetroFuturaGUI::ColorPickerIndicator::SetColorState(const ColorState state)
{
    _border.SetColorState(state);
}

void RetroFuturaGUI::ColorPickerIndicator::SetTransitionSettings(const TransitionSettings* settings)
{
    _dot.SetTransitionSettings(settings);
    _border.SetTransitionSettings(settings);
}
