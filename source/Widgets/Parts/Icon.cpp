#include "Icon.hpp"
#include <utility>

RetroFuturaGUI::Icon::Icon(std::unique_ptr<SvgTexture> svg, std::string_view pathName, const SvgPathFill& style, IconColors colors)
    : _svg(std::move(svg)), _pathName(pathName), _fill(style), _colors(std::move(colors))
{
}

void RetroFuturaGUI::Icon::SetColors(std::span<const glm::vec4> colors, const ColorState state)
{
    colorsOf(state).assign(colors.begin(), colors.end());
    applyFill();
}

const std::vector<glm::vec4>& RetroFuturaGUI::Icon::GetColors(const ColorState state) const
{
    switch(state)
    {
        case ColorState::Clicked:
            return _colors._Clicked;
        case ColorState::Disabled:
            return _colors._Disabled;
        case ColorState::Hover:
            return _colors._Hover;
        case ColorState::Selected:
            return _colors._Selected;
        default: // Enabled
            return _colors._Enabled;
    }
}

void RetroFuturaGUI::Icon::SetFillType(const FillType fillType)
{
    _fill._FillType = fillType;
    applyFill();
}

void RetroFuturaGUI::Icon::SetGradientDegree(const f32 degree)
{
    _fill._GradientDegree = degree;
    applyFill();
}

void RetroFuturaGUI::Icon::SetGradientOffset(const f32 gradientOffset)
{
    _fill._GradientOffset = gradientOffset;
    applyFill();
}

void RetroFuturaGUI::Icon::SetGradientAnimationSpeed(const f32 animationSpeed)
{
    _fill._GradientAnimationSpeed = animationSpeed;
    applyFill();
}

void RetroFuturaGUI::Icon::SetGradientRotationSpeed(const f32 rotationSpeed)
{
    _fill._GradientRotationSpeed = rotationSpeed;
    applyFill();
}

void RetroFuturaGUI::Icon::Draw()
{
    if(!_svg)
        return;

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
    _colorState = state;
    applyFill();
}

std::vector<glm::vec4>& RetroFuturaGUI::Icon::colorsOf(const ColorState state)
{
    switch(state)
    {
        case ColorState::Clicked:
            return _colors._Clicked;
        case ColorState::Disabled:
            return _colors._Disabled;
        case ColorState::Hover:
            return _colors._Hover;
        case ColorState::Selected:
            return _colors._Selected;
        default: // Enabled
            return _colors._Enabled;
    }
}

void RetroFuturaGUI::Icon::applyFill()
{
    if(!_svg)
        return;

    const bool showsEnabled { ColorState::Selected == _colorState && _colors._Selected.empty() };
    _fill._Colors = showsEnabled ? _colors._Enabled : GetColors(_colorState);
    _svg->SetPathFill(_pathName, _fill);
}