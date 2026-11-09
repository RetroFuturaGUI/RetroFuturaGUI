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

void RetroFuturaGUI::DropShadow::SetDegree(const f32 degree)
{
    _degreeLayer.Set(degree);
    resolveNumbers();
}

f32 RetroFuturaGUI::DropShadow::GetDegree() const
{
    return _degree.GetTarget();
}

void RetroFuturaGUI::DropShadow::SetDistance(const f32 distance)
{
    _distanceLayer.Set(distance);
    resolveNumbers();
}

f32 RetroFuturaGUI::DropShadow::GetDistance() const
{
    return _distance.GetTarget();
}

void RetroFuturaGUI::DropShadow::SetBlur(const f32 blur)
{
    _blurLayer.Set(std::max(blur, 0.0f));
    resolveNumbers();
}

f32 RetroFuturaGUI::DropShadow::GetBlur() const
{
    return _blur.GetTarget();
}

void RetroFuturaGUI::DropShadow::SetScale(const f32 scale)
{
    _scaleLayer.Set(std::max(scale, 0.0f));
    resolveNumbers();
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

const RetroFuturaGUI::FlavorEffect& RetroFuturaGUI::DropShadow::pickFlavor(const FlavorPart& part) const
{
    return part.GetDropShadow();
}

void RetroFuturaGUI::DropShadow::resolveNumbers()
{
    resolveNumber(_degreeLayer, _degree, 315.0f, &FlavorEffect::GetDegree, false);
    resolveNumber(_distanceLayer, _distance, 4.0f, &FlavorEffect::GetDistance, false);
    resolveNumber(_blurLayer, _blur, 8.0f, &FlavorEffect::GetBlur, true);
    resolveNumber(_scaleLayer, _scale, 1.0f, &FlavorEffect::GetScale, true);
}

bool RetroFuturaGUI::DropShadow::resetNumber(const EffectSetting setting)
{
    switch(setting)
    {
        case EffectSetting::Degree:
            _degreeLayer.Reset();
            return true;
        case EffectSetting::Distance:
            _distanceLayer.Reset();
            return true;
        case EffectSetting::Blur:
            _blurLayer.Reset();
            return true;
        case EffectSetting::Scale:
            _scaleLayer.Reset();
            return true;
        default:
            return false;
    }
}