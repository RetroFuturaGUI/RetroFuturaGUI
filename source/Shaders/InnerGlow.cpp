#include "InnerGlow.hpp"
#include <algorithm>

RetroFuturaGUI::InnerGlow::InnerGlow()
    : StateFillStyle(glm::vec4(1.0f, 1.0f, 1.0f, 0.6f))
{
    _width.Jump(6.0f);
    _degree.Jump(315.0f);
    _distance.Jump(0.0f);
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
    setEffectValue(_width, std::max(width, 0.0f));
}

f32 RetroFuturaGUI::InnerGlow::GetWidth() const
{
    return _width.GetTarget();
}

void RetroFuturaGUI::InnerGlow::SetDegree(const f32 degree)
{
    setEffectValue(_degree, degree);
}

f32 RetroFuturaGUI::InnerGlow::GetDegree() const
{
    return _degree.GetTarget();
}

void RetroFuturaGUI::InnerGlow::SetDistance(const f32 distance)
{
    setEffectValue(_distance, distance);
}

f32 RetroFuturaGUI::InnerGlow::GetDistance() const
{
    return _distance.GetTarget();
}

glm::vec2 RetroFuturaGUI::InnerGlow::GetOffset() const
{
    return getOffset(_degree.GetTarget(), _distance.GetTarget());
}

f32 RetroFuturaGUI::InnerGlow::getShownWidth() const
{
    return _width.Get();
}

glm::vec2 RetroFuturaGUI::InnerGlow::getShownOffset() const
{
    return getOffset(_degree.Get(), _distance.Get());
}

f32 RetroFuturaGUI::InnerGlow::getGlyphProgress() const
{
    return std::min({ _width.GetProgress(), _degree.GetProgress(), _distance.GetProgress() });
}