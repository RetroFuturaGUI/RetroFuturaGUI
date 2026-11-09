#include "InnerGlow.hpp"
#include <algorithm>

RetroFuturaGUI::InnerGlow::InnerGlow()
    : StateFillStyle(glm::vec4(1.0f, 1.0f, 1.0f, 0.6f))
{
    _width.Jump(6.0f);
    _degree.Jump(315.0f);
    _distance.Jump(0.0f);
}

void RetroFuturaGUI::InnerGlow::SetWidth(const f32 width)
{
    _widthLayer.Set(std::max(width, 0.0f));
    resolveNumbers();
}

f32 RetroFuturaGUI::InnerGlow::GetWidth() const
{
    return _width.GetTarget();
}

void RetroFuturaGUI::InnerGlow::SetDegree(const f32 degree)
{
    _degreeLayer.Set(degree);
    resolveNumbers();
}

f32 RetroFuturaGUI::InnerGlow::GetDegree() const
{
    return _degree.GetTarget();
}

void RetroFuturaGUI::InnerGlow::SetDistance(const f32 distance)
{
    _distanceLayer.Set(distance);
    resolveNumbers();
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

const RetroFuturaGUI::FlavorEffect& RetroFuturaGUI::InnerGlow::pickFlavor(const FlavorPart& part) const
{
    return part.GetInnerGlow();
}

void RetroFuturaGUI::InnerGlow::resolveNumbers()
{
    resolveNumber(_widthLayer, _width, 6.0f, &FlavorEffect::GetWidth, true);
    resolveNumber(_degreeLayer, _degree, 315.0f, &FlavorEffect::GetDegree, false);
    resolveNumber(_distanceLayer, _distance, 0.0f, &FlavorEffect::GetDistance, false);
}

bool RetroFuturaGUI::InnerGlow::resetNumber(const EffectSetting setting)
{
    switch(setting)
    {
        case EffectSetting::Width:
            _widthLayer.Reset();
            return true;
        case EffectSetting::Degree:
            _degreeLayer.Reset();
            return true;
        case EffectSetting::Distance:
            _distanceLayer.Reset();
            return true;
        default:
            return false;
    }
}