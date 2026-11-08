#include "Glow.hpp"
#include <algorithm>

RetroFuturaGUI::Glow::Glow()
    : StateFillStyle(glm::vec4(1.0f, 1.0f, 1.0f, 0.75f))
{
    _width.Jump(8.0f);
}

void RetroFuturaGUI::Glow::Show(const bool show)
{
    _isShown = show;
}

bool RetroFuturaGUI::Glow::IsShown() const
{
    return _isShown;
}

void RetroFuturaGUI::Glow::SetWidth(const f32 width)
{
    setEffectValue(_width, std::max(width, 0.0f));
}

f32 RetroFuturaGUI::Glow::GetWidth() const
{
    return _width.GetTarget();
}

f32 RetroFuturaGUI::Glow::getShownWidth() const
{
    return _width.Get();
}
