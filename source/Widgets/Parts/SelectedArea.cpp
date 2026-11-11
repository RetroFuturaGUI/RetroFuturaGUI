#include "SelectedArea.hpp"
#include <array>

RetroFuturaGUI::SelectedArea::SelectedArea(Projection* projection)
    : _plane(projection)
{
    _plane.SetFillType(FillType::Solid);
    _colors.SetBuiltIn(std::array { glm::vec4(0.24f, 0.47f, 0.85f, 0.4f) }, ColorState::Enabled);
    applyStyle();
}

void RetroFuturaGUI::SelectedArea::SetColors(std::span<const glm::vec4> colors)
{
    _colors.SetOwn(colors, ColorState::Enabled);
    applyStyle();
}

void RetroFuturaGUI::SelectedArea::SetBuiltInColors(const StyleKey&, std::span<const glm::vec4> colors)
{
    _colors.SetBuiltIn(colors, ColorState::Enabled);
    applyStyle();
}

void RetroFuturaGUI::SelectedArea::SetFillType(const FillType fillType)
{
    _fillTypeLayer.Set(fillType);
    applyStyle();
}

void RetroFuturaGUI::SelectedArea::SetGradientAnimationSpeed(const f32 speed)
{
    _gradientAnimationSpeedLayer.Set(speed);
    applyStyle();
}

void RetroFuturaGUI::SelectedArea::SetGradientOffset(const f32 gradientOffset)
{
    _gradientOffsetLayer.Set(gradientOffset);
    applyStyle();
}

void RetroFuturaGUI::SelectedArea::SetGradientDegree(const f32 degree)
{
    _gradientDegreeLayer.Set(degree);
    applyStyle();
}

void RetroFuturaGUI::SelectedArea::SetGradientRotationSpeed(const f32 rotationSpeed)
{
    _gradientRotationSpeedLayer.Set(rotationSpeed);
    applyStyle();
}

void RetroFuturaGUI::SelectedArea::SetCornerRadii(const glm::vec4& radii)
{
    _cornerRadiiLayer.Set(radii);
    applyStyle();
}

void RetroFuturaGUI::SelectedArea::Draw(std::span<const LineArea> areas)
{
    followFlavors();

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

void RetroFuturaGUI::SelectedArea::ResetSetting(const StyleSetting setting)
{
    switch(setting)
    {
        case StyleSetting::FillType:
            _fillTypeLayer.Reset();
        break;
        case StyleSetting::GradientAnimationSpeed:
            _gradientAnimationSpeedLayer.Reset();
        break;
        case StyleSetting::GradientOffset:
            _gradientOffsetLayer.Reset();
        break;
        case StyleSetting::GradientDegree:
            _gradientDegreeLayer.Reset();
        break;
        case StyleSetting::GradientRotationSpeed:
            _gradientRotationSpeedLayer.Reset();
        break;
        case StyleSetting::CornerRadii:
            _cornerRadiiLayer.Reset();
        break;
        default: //a setting the highlight doesn't have.
            return;
    }

    applyStyle();
}

void RetroFuturaGUI::SelectedArea::restyle()
{
    applyStyle();
}

void RetroFuturaGUI::SelectedArea::applyStyle()
{
    const std::span<const u32> flavors { getFlavors() };
    const StylePart part { getStylePart() };

    _plane.SetColors(_colors.GetShown(ColorState::Enabled, flavors, isStyled(), part));

    if(const FillType* value { _fillTypeLayer.Resolve(flavors, isStyled(), part, FillType::Solid, [](const FlavorPart& style) -> const std::optional<FillType>& { return style.GetFillType(); }) })
        _plane.SetFillType(*value);

    if(const f32* value { _gradientAnimationSpeedLayer.Resolve(flavors, isStyled(), part, 0.018f, [](const FlavorPart& style) -> const std::optional<f32>& { return style.GetGradientAnimationSpeed(); }) })
        _plane.SetGradientAnimationSpeed(*value);

    if(const f32* value { _gradientOffsetLayer.Resolve(flavors, isStyled(), part, 0.0f, [](const FlavorPart& style) -> const std::optional<f32>& { return style.GetGradientOffset(); }) })
        _plane.SetGradientOffset(*value);

    if(const f32* value { _gradientDegreeLayer.Resolve(flavors, isStyled(), part, 45.0f, [](const FlavorPart& style) -> const std::optional<f32>& { return style.GetGradientDegree(); }) })
        _plane.SetGradientDegree(*value);

    if(const f32* value { _gradientRotationSpeedLayer.Resolve(flavors, isStyled(), part, 1.2f, [](const FlavorPart& style) -> const std::optional<f32>& { return style.GetGradientRotationSpeed(); }) })
        _plane.SetGradientRotationSpeed(*value);

    if(const glm::vec4* radii { _cornerRadiiLayer.Resolve(flavors, isStyled(), part, glm::vec4(0.0f), [](const FlavorPart& style) -> const std::optional<glm::vec4>& { return style.GetCornerRadii(); }) })
        _plane.SetCornerRadii(*radii);
}
