#include "SelectedArea.hpp"

RetroFuturaGUI::SelectedArea::SelectedArea(Projection* projection)
    : _plane(projection)
{
    _plane.SetFillType(FillType::Solid);
    _plane.SetColors(_colors);
}

void RetroFuturaGUI::SelectedArea::SetColors(std::span<const glm::vec4> colors)
{
    _colors.assign(colors.begin(), colors.end());
    _plane.SetColors(_colors);
}

void RetroFuturaGUI::SelectedArea::SetFillType(const FillType fillType)
{
    _plane.SetFillType(fillType);
}

void RetroFuturaGUI::SelectedArea::SetGradientAnimationSpeed(const f32 speed)
{
    _plane.SetGradientAnimationSpeed(speed);
}

void RetroFuturaGUI::SelectedArea::SetGradientOffset(const f32 gradientOffset)
{
    _plane.SetGradientOffset(gradientOffset);
}

void RetroFuturaGUI::SelectedArea::SetGradientDegree(const f32 degree)
{
    _plane.SetGradientDegree(degree);
}

void RetroFuturaGUI::SelectedArea::SetGradientRotationSpeed(const f32 rotationSpeed)
{
    _plane.SetGradientRotationSpeed(rotationSpeed);
}

void RetroFuturaGUI::SelectedArea::SetCornerRadii(const glm::vec4& radii)
{
    _plane.SetCornerRadii(radii);
}

void RetroFuturaGUI::SelectedArea::Draw(std::span<const LineArea> areas)
{
    //Plane::Draw() advances the gradient animation on every call
    const f32
        gradientOffset { _plane.GetGradientOffset() },
        gradientDegree { _plane.GetGradientDegree() },
        z { _plane.GetPosition().z };

    for(const LineArea& area : areas)
    {
        _plane.SetGradientOffset(gradientOffset);
        _plane.SetGradientDegree(gradientDegree);
        _plane.SetSize(area._Size);
        _plane.SetPosition(glm::vec3(area._Center, z));
        _plane.Draw();
    }
}

void RetroFuturaGUI::SelectedArea::SetPosition(const glm::vec3& position)
{
    _plane.SetPosition(position);
}

void RetroFuturaGUI::SelectedArea::SetTransitionSettings(const TransitionSettings* settings)
{
    _plane.SetTransitionSettings(settings);
}