#include "DropShadow.hpp"
#include <algorithm>
#include <cmath>
#include <glm/trigonometric.hpp>

RetroFuturaGUI::DropShadow::DropShadow()
    : StateFillStyle(glm::vec4(0.0f, 0.0f, 0.0f, 0.5f))
{
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

glm::vec2 RetroFuturaGUI::DropShadow::GetOffset() const
{
    const f32 radians { glm::radians(_degree) };
    return glm::vec2(std::cos(radians), std::sin(radians)) * _distance;
}
