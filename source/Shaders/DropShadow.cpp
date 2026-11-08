#include "DropShadow.hpp"
#include <algorithm>
#include <cmath>
#include <utility>
#include <glm/trigonometric.hpp>

RetroFuturaGUI::DropShadow::DropShadow()
{
    applyColors();
}

void RetroFuturaGUI::DropShadow::Show(const bool show)
{
    _isShown = show;
}

bool RetroFuturaGUI::DropShadow::IsShown() const
{
    return _isShown;
}

void RetroFuturaGUI::DropShadow::SetDegree(const f32 degree)
{
    _degree = degree;
}

f32 RetroFuturaGUI::DropShadow::GetDegree() const
{
    return _degree;
}

void RetroFuturaGUI::DropShadow::SetDistance(const f32 distance)
{
    _distance = distance;
}

f32 RetroFuturaGUI::DropShadow::GetDistance() const
{
    return _distance;
}

void RetroFuturaGUI::DropShadow::SetBlur(const f32 blur)
{
    _blur = std::max(blur, 0.0f);
}

f32 RetroFuturaGUI::DropShadow::GetBlur() const
{
    return _blur;
}

void RetroFuturaGUI::DropShadow::SetScale(const f32 scale)
{
    _scale = std::max(scale, 0.0f);
}

f32 RetroFuturaGUI::DropShadow::GetScale() const
{
    return _scale;
}

void RetroFuturaGUI::DropShadow::SetColors(std::span<const glm::vec4> colors, const ColorState state)
{
    if(ColorState::Enabled == state && colors.empty())
        return; //every empty state shows Enabled, so it always holds a color

    colorsOf(state).assign(colors.begin(), colors.end());
    applyColors();
}

const std::vector<glm::vec4>& RetroFuturaGUI::DropShadow::GetColors(const ColorState state) const
{
    return colorsOf(state);
}

glm::vec2 RetroFuturaGUI::DropShadow::GetOffset() const
{
    const f32 radians { glm::radians(_degree) };
    return glm::vec2(std::cos(radians), std::sin(radians)) * _distance;
}

void RetroFuturaGUI::DropShadow::SetColorState(const ColorState state)
{
    if(state == _colorState)
        return;

    _colorState = state;
    applyColors();
}

std::vector<glm::vec4>& RetroFuturaGUI::DropShadow::colorsOf(const ColorState state)
{
    return const_cast<std::vector<glm::vec4>&>(std::as_const(*this).colorsOf(state));
}

const std::vector<glm::vec4>& RetroFuturaGUI::DropShadow::colorsOf(const ColorState state) const
{
    switch(state)
    {
        case ColorState::Disabled:
            return _colorsDisabled;
        case ColorState::Hover:
            return _colorsHover;
        case ColorState::Clicked:
            return _colorsClicked;
        case ColorState::Selected:
            return _colorsSelected;
        default: //Enabled
            return _colorsEnabled;
    }
}

void RetroFuturaGUI::DropShadow::applyColors()
{
    const std::vector<glm::vec4>& colors { colorsOf(_colorState) };
    FillStyle::SetColors(colors.empty() ? _colorsEnabled : colors);
}