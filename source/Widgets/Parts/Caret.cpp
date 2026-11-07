#include "Caret.hpp"

RetroFuturaGUI::Caret::Caret(Projection* projection)
    : _plane(projection)
{
    _plane.SetFillType(FillType::Solid);
    _plane.SetColors(_colors);
}

void RetroFuturaGUI::Caret::SetColors(std::span<const glm::vec4> colors)
{
    _colors.assign(colors.begin(), colors.end());
    _plane.SetColors(_colors);
}

void RetroFuturaGUI::Caret::SetFillType(const FillType fillType)
{
    _plane.SetFillType(fillType);
}

void RetroFuturaGUI::Caret::SetGradientAnimationSpeed(const f32 speed)
{
    _plane.SetGradientAnimationSpeed(speed);
}

void RetroFuturaGUI::Caret::SetBlinkTime(const f64 milliseconds)
{
    if(16.0 >= milliseconds) //at or below one frame a blink can't be seen
    {
        _neverBlinks = true;
        return;
    }

    _blinkTime = milliseconds;
    _neverBlinks = false;
}

f64 RetroFuturaGUI::Caret::GetBlinkTime() const
{
    return _blinkTime;
}

bool RetroFuturaGUI::Caret::NeverBlinks() const
{
    return _neverBlinks;
}

void RetroFuturaGUI::Caret::Draw()
{
    _plane.Draw();
}

void RetroFuturaGUI::Caret::SetSize(const glm::vec2& size)
{
    _plane.SetSize(size);
}

void RetroFuturaGUI::Caret::SetPosition(const glm::vec3& position)
{
    _plane.SetPosition(position);
}

void RetroFuturaGUI::Caret::SetTransitionSettings(const TransitionSettings* settings)
{
    _plane.SetTransitionSettings(settings);
}