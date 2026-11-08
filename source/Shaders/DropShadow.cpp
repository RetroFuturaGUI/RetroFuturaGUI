#include "DropShadow.hpp"
#include <algorithm>

RetroFuturaGUI::DropShadow::DropShadow()
    : StateFillStyle(glm::vec4(0.0f, 0.0f, 0.0f, 0.5f))
{
    _degree.Jump(315.0f);
    _distance.Jump(4.0f);
    _blur.Jump(8.0f);
    _scale.Jump(1.0f);
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
    setEffectValue(_degree, degree);
}

f32 RetroFuturaGUI::DropShadow::GetDegree() const
{
    return _degree.GetTarget();
}

void RetroFuturaGUI::DropShadow::SetDistance(const f32 distance)
{
    setEffectValue(_distance, distance);
}

f32 RetroFuturaGUI::DropShadow::GetDistance() const
{
    return _distance.GetTarget();
}

void RetroFuturaGUI::DropShadow::SetBlur(const f32 blur)
{
    setEffectValue(_blur, std::max(blur, 0.0f));
}

f32 RetroFuturaGUI::DropShadow::GetBlur() const
{
    return _blur.GetTarget();
}

void RetroFuturaGUI::DropShadow::SetScale(const f32 scale)
{
    setEffectValue(_scale, std::max(scale, 0.0f));
}

f32 RetroFuturaGUI::DropShadow::GetScale() const
{
    return _scale.GetTarget();
}

glm::vec2 RetroFuturaGUI::DropShadow::GetOffset() const
{
    return getOffset(_degree.GetTarget(), _distance.GetTarget());
}

glm::vec2 RetroFuturaGUI::DropShadow::getShownOffset() const
{
    return getOffset(_degree.Get(), _distance.Get());
}

f32 RetroFuturaGUI::DropShadow::getShownBlur() const
{
    return _blur.Get();
}

f32 RetroFuturaGUI::DropShadow::getShownScale() const
{
    return _scale.Get();
}

f32 RetroFuturaGUI::DropShadow::getGlyphProgress() const
{
    return _blur.GetProgress();
}