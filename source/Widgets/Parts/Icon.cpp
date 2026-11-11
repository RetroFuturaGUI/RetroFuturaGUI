#include "Icon.hpp"
#include <utility>

RetroFuturaGUI::Icon::Icon(std::unique_ptr<SvgTexture> svg, std::string_view pathName, const SvgPathFill& style, IconColors colors)
    : _svg(std::move(svg)), _pathName(pathName), _fill(style), _builtInFill(style)
{
    _colors.SetBuiltIn(colors._Enabled, ColorState::Enabled);
    _colors.SetBuiltIn(colors._Disabled, ColorState::Disabled);
    _colors.SetBuiltIn(colors._Clicked, ColorState::Clicked);
    _colors.SetBuiltIn(colors._Hover, ColorState::Hover);
    _colors.SetBuiltIn(colors._Selected, ColorState::Selected);
}

void RetroFuturaGUI::Icon::SetColors(std::span<const glm::vec4> colors, const ColorState state)
{
    _colors.SetOwn(colors, state);
    applyFill();
}

void RetroFuturaGUI::Icon::SetBuiltInColors(const StyleKey&, std::span<const glm::vec4> colors, const ColorState state)
{
    _colors.SetBuiltIn(colors, state);
    applyFill();
}

const std::vector<glm::vec4>& RetroFuturaGUI::Icon::GetColors(const ColorState state) const
{
    return _colors.GetOwn(state);
}

void RetroFuturaGUI::Icon::SetFillType(const FillType fillType)
{
    _fillTypeLayer.Set(fillType);
    applyFill();
}

void RetroFuturaGUI::Icon::SetGradientDegree(const f32 degree)
{
    _gradientDegreeLayer.Set(degree);
    applyFill();
}

void RetroFuturaGUI::Icon::SetGradientOffset(const f32 gradientOffset)
{
    _gradientOffsetLayer.Set(gradientOffset);
    applyFill();
}

void RetroFuturaGUI::Icon::SetGradientAnimationSpeed(const f32 animationSpeed)
{
    _gradientAnimationSpeedLayer.Set(animationSpeed);
    applyFill();
}

void RetroFuturaGUI::Icon::SetGradientRotationSpeed(const f32 rotationSpeed)
{
    _gradientRotationSpeedLayer.Set(rotationSpeed);
    applyFill();
}

void RetroFuturaGUI::Icon::ResetSetting(const StyleSetting setting)
{
    switch(setting)
    {
        case StyleSetting::FillType:
            _fillTypeLayer.Reset();
        break;
        case StyleSetting::GradientDegree:
            _gradientDegreeLayer.Reset();
        break;
        case StyleSetting::GradientOffset:
            _gradientOffsetLayer.Reset();
        break;
        case StyleSetting::GradientAnimationSpeed:
            _gradientAnimationSpeedLayer.Reset();
        break;
        case StyleSetting::GradientRotationSpeed:
            _gradientRotationSpeedLayer.Reset();
        break;
        default: //a setting the icon doesn't have.
            return;
    }

    applyFill();
}

void RetroFuturaGUI::Icon::Draw()
{
    if(!_svg)
        return;

    followFlavors();
    _svg->Draw();
}

void RetroFuturaGUI::Icon::SetSize(const glm::vec2& size)
{
    if(!_svg)
        return;

    _svg->SetSize(size);
}

void RetroFuturaGUI::Icon::SetPosition(const glm::vec3& position)
{
    if(!_svg)
        return;

    _svg->SetPosition(position);
}

void RetroFuturaGUI::Icon::SetRotation(const glm::vec3& rotation)
{
    if(!_svg)
        return;

    _svg->SetRotation(rotation);
}

f32 RetroFuturaGUI::Icon::GetAspectRatio() const
{
    if(!_svg)
        return 0.0f;

    return _svg->GetAspectRatio();
}

void RetroFuturaGUI::Icon::Rasterize(const glm::i32vec2& resolution)
{
    if(!_svg)
        return;

    _svg->Rasterize(resolution);
}

void RetroFuturaGUI::Icon::SetColorState(const ColorState state)
{
    if(_isFilled && state == _colorState)
        return; //a widget drawing one icon in many places asks for the same state again and again

    _colorState = state;
    applyFill();
}

void RetroFuturaGUI::Icon::restyle()
{
    if(!_isFilled)
        return; //until the widget shows a state, the SVG keeps its own colors.

    applyFill();
}

void RetroFuturaGUI::Icon::applyFill()
{
    if(!_svg)
        return;

    const std::span<const u32> flavors { getFlavors() };
    const StylePart part { getStylePart() };

    if(const FillType* fillType { _fillTypeLayer.Resolve(flavors, isStyled(), part, _builtInFill._FillType, [](const FlavorPart& style) -> const std::optional<FillType>& { return style.GetFillType(); }) })
        _fill._FillType = *fillType;

    if(const f32* degree { _gradientDegreeLayer.Resolve(flavors, isStyled(), part, _builtInFill._GradientDegree, [](const FlavorPart& style) -> const std::optional<f32>& { return style.GetGradientDegree(); }) })
        _fill._GradientDegree = *degree;

    if(const f32* offset { _gradientOffsetLayer.Resolve(flavors, isStyled(), part, _builtInFill._GradientOffset, [](const FlavorPart& style) -> const std::optional<f32>& { return style.GetGradientOffset(); }) })
        _fill._GradientOffset = *offset;

    if(const f32* speed { _gradientAnimationSpeedLayer.Resolve(flavors, isStyled(), part, _builtInFill._GradientAnimationSpeed, [](const FlavorPart& style) -> const std::optional<f32>& { return style.GetGradientAnimationSpeed(); }) })
        _fill._GradientAnimationSpeed = *speed;

    if(const f32* speed { _gradientRotationSpeedLayer.Resolve(flavors, isStyled(), part, _builtInFill._GradientRotationSpeed, [](const FlavorPart& style) -> const std::optional<f32>& { return style.GetGradientRotationSpeed(); }) })
        _fill._GradientRotationSpeed = *speed;

    _fill._Colors = _colors.GetShown(_colorState, flavors, isStyled(), part);
    _svg->SetPathFill(_pathName, _fill);
    _isFilled = true;
}
