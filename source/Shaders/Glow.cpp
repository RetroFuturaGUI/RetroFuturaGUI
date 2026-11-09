#include "Glow.hpp"
#include <algorithm>

RetroFuturaGUI::Glow::Glow()
    : StateFillStyle(glm::vec4(1.0f, 1.0f, 1.0f, 0.75f))
{
    _width.Jump(8.0f);
}

void RetroFuturaGUI::Glow::SetWidth(const f32 width)
{
    _widthLayer.Set(std::max(width, 0.0f));
    resolveNumbers();
}

f32 RetroFuturaGUI::Glow::GetWidth() const
{
    return _width.GetTarget();
}

f32 RetroFuturaGUI::Glow::getShownWidth() const
{
    return _width.Get();
}

f32 RetroFuturaGUI::Glow::getGlyphProgress() const
{
    return _width.GetProgress();
}

const RetroFuturaGUI::FlavorEffect& RetroFuturaGUI::Glow::pickFlavor(const FlavorPart& part) const
{
    return part.GetGlow();
}

void RetroFuturaGUI::Glow::resolveNumbers()
{
    resolveNumber(_widthLayer, _width, 8.0f, &FlavorEffect::GetWidth, true);
}

bool RetroFuturaGUI::Glow::resetNumber(const EffectSetting setting)
{
    switch(setting)
    {
        case EffectSetting::Width:
            _widthLayer.Reset();
            return true;
        default:
            return false;
    }
}