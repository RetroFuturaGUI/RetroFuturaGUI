#include "DropShadow.hpp"
#include <algorithm>
#include <cmath>
#include <glm/trigonometric.hpp>

RetroFuturaGUI::DropShadow::DropShadow()
{
    FillStyle::SetColors(_shadowColors);
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

void RetroFuturaGUI::DropShadow::SetColors(std::span<const glm::vec4> colors)
{
    if(colors.empty())
        return; //the fill keeps a span over them, so there is always one.

    _shadowColors.assign(colors.begin(), colors.end());
    FillStyle::SetColors(_shadowColors);
}

const std::vector<glm::vec4>& RetroFuturaGUI::DropShadow::GetColors() const
{
    return _shadowColors;
}

glm::vec2 RetroFuturaGUI::DropShadow::GetOffset() const
{
    const f32 radians { glm::radians(_degree) };
    return glm::vec2(std::cos(radians), std::sin(radians)) * _distance;
}