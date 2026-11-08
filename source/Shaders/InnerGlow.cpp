#include "InnerGlow.hpp"
#include <algorithm>
#include <cmath>
#include <glm/trigonometric.hpp>

RetroFuturaGUI::InnerGlow::InnerGlow()
    : StateFillStyle(glm::vec4(1.0f, 1.0f, 1.0f, 0.6f))
{
}

void RetroFuturaGUI::InnerGlow::Show(const bool show)
{
    _isShown = show;
}

bool RetroFuturaGUI::InnerGlow::IsShown() const
{
    return _isShown;
}

void RetroFuturaGUI::InnerGlow::SetWidth(const f32 width)
{
    _width = std::max(width, 0.0f);
}

f32 RetroFuturaGUI::InnerGlow::GetWidth() const
{
    return _width;
}

void RetroFuturaGUI::InnerGlow::SetDegree(const f32 degree)
{
    _degree = degree;
}

f32 RetroFuturaGUI::InnerGlow::GetDegree() const
{
    return _degree;
}

void RetroFuturaGUI::InnerGlow::SetDistance(const f32 distance)
{
    _distance = distance;
}

f32 RetroFuturaGUI::InnerGlow::GetDistance() const
{
    return _distance;
}

glm::vec2 RetroFuturaGUI::InnerGlow::GetOffset() const
{
    const f32 radians { glm::radians(_degree) };
    return glm::vec2(std::cos(radians), std::sin(radians)) * _distance;
}