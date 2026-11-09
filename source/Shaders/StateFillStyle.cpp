#include "StateFillStyle.hpp"
#include <algorithm>
#include <cmath>
#include <utility>
#include <glm/trigonometric.hpp>

RetroFuturaGUI::StateFillStyle::StateFillStyle(const glm::vec4& enabledColor)
{
    _builtInColors[static_cast<uSize>(ColorState::Enabled)] = { enabledColor };
    applyColors();
}

void RetroFuturaGUI::StateFillStyle::SetColors(std::span<const glm::vec4> colors, const ColorState state)
{
    _ownColors[static_cast<uSize>(state)].assign(colors.begin(), colors.end());
    applyColors();
}

const std::vector<glm::vec4>& RetroFuturaGUI::StateFillStyle::GetColors(const ColorState state) const
{
    return _ownColors[static_cast<uSize>(state)];
}

void RetroFuturaGUI::StateFillStyle::Show(const bool show)
{
    _shownLayer.Set(show);
    resolveShown();
}

bool RetroFuturaGUI::StateFillStyle::IsShown() const
{
    return _isShown;
}

void RetroFuturaGUI::StateFillStyle::SetFillType(const FillType fillType)
{
    _fillTypeLayer.Set(fillType);
    resolveFill();
}

void RetroFuturaGUI::StateFillStyle::SetGradientOffset(const f32 gradientOffset)
{
    _gradientOffsetLayer.Set(gradientOffset);
    resolveFill();
}

void RetroFuturaGUI::StateFillStyle::SetGradientAnimationSpeed(const f32 animationSpeed)
{
    _gradientAnimationSpeedLayer.Set(animationSpeed);
    resolveFill();
}

void RetroFuturaGUI::StateFillStyle::SetGradientDegree(const f32 degree)
{
    _gradientDegreeLayer.Set(degree);
    resolveFill();
}

void RetroFuturaGUI::StateFillStyle::SetGradientRotationSpeed(const f32 rotationSpeed)
{
    _gradientRotationSpeedLayer.Set(rotationSpeed);
    resolveFill();
}

void RetroFuturaGUI::StateFillStyle::ResetSetting(const EffectSetting setting)
{
    if(resetNumber(setting))
    {
        resolveNumbers();
        return;
    }

    switch(setting)
    {
        case EffectSetting::Shown:
            _shownLayer.Reset();
            resolveShown();
            return;
        case EffectSetting::FillType:
            _fillTypeLayer.Reset();
        break;
        case EffectSetting::GradientDegree:
            _gradientDegreeLayer.Reset();
        break;
        case EffectSetting::GradientOffset:
            _gradientOffsetLayer.Reset();
        break;
        case EffectSetting::GradientAnimationSpeed:
            _gradientAnimationSpeedLayer.Reset();
        break;
        case EffectSetting::GradientRotationSpeed:
            _gradientRotationSpeedLayer.Reset();
        break;
        default: //a number the effect doesn't have.
            return;
    }

    resolveFill();
}

void RetroFuturaGUI::StateFillStyle::SetColorState(const ColorState state)
{
    if(state == _colorState)
        return;

    _colorState = state;
    applyColors();
}

void RetroFuturaGUI::StateFillStyle::setEffectValue(TransitionValue<f32>& value, const f32 target)
{
    //Before the first draw the colors haven't shown either: setup values show at once, as theirs do.
    if(!_transitionSettings || !_hasShownColors)
    {
        value.Jump(target);
        return;
    }

    value.Set(target, Transitions::Resolve(*_transitionSettings, TransitionProperty::Effects));
}

glm::vec2 RetroFuturaGUI::StateFillStyle::getOffset(const f32 degree, const f32 distance)
{
    const f32 radians { glm::radians(degree) };
    return glm::vec2(std::cos(radians), std::sin(radians)) * distance;
}

void RetroFuturaGUI::StateFillStyle::resolveNumber(LayeredSetting<f32>& layer, TransitionValue<f32>& value, const f32 builtIn, const std::optional<f32>& (FlavorEffect::*get)() const, const bool isNonNegative)
{
    const f32* number { layer.Resolve(getFlavors(), nullptr != _flavors, _stylePart, builtIn, [this, get](const FlavorPart& part) -> const std::optional<f32>& { return (pickFlavor(part).*get)(); }) };

    if(!number)
        return;

    setEffectValue(value, isNonNegative ? std::max(*number, 0.0f) : *number);
}

void RetroFuturaGUI::StateFillStyle::followFlavors(const std::vector<u32>* flavors, const StylePart part)
{
    _flavors = flavors;
    _stylePart = part;
    restyle();
}

void RetroFuturaGUI::StateFillStyle::restyle()
{
    applyColors(); //handed on by reference, so they go again even when unchanged: a flavor's list may have moved.
    resolveFill();
    resolveShown();
    resolveNumbers();
}

const std::vector<glm::vec4>& RetroFuturaGUI::StateFillStyle::getShownColors(const ColorState state) const
{
    const std::vector<glm::vec4>& own { _ownColors[static_cast<uSize>(state)] };

    if(!own.empty())
        return own;

    //An effect without flavors (a sub-part's) goes from its own colors straight to its default.
    const std::vector<glm::vec4>* styled { _flavors ? StylingManager::FindFirst<const std::vector<glm::vec4>*>(getFlavors(), _stylePart, [this, state](const FlavorPart& part) -> const std::vector<glm::vec4>*
    {
        const std::vector<glm::vec4>& colors { pickFlavor(part).GetColors(state) };
        return colors.empty() ? nullptr : &colors;
    }) : nullptr };

    if(styled)
        return *styled;

    const std::vector<glm::vec4>& builtIn { _builtInColors[static_cast<uSize>(state)] };

    if(!builtIn.empty() || ColorState::Enabled == state)
        return builtIn;

    return getShownColors(ColorState::Enabled); //a state nothing sets looks like Enabled.
}

void RetroFuturaGUI::StateFillStyle::applyColors()
{
    FillStyle::SetColors(getShownColors(_colorState));
}

void RetroFuturaGUI::StateFillStyle::resolveFill()
{
    const std::span<const u32> flavors { getFlavors() };
    const bool isStyled { nullptr != _flavors };

    if(const FillType* fillType { _fillTypeLayer.Resolve(flavors, isStyled, _stylePart, FillType::Solid, [this](const FlavorPart& part) -> const std::optional<FillType>& { return pickFlavor(part).GetFillType(); }) })
        FillStyle::SetFillType(*fillType);

    if(const f32* degree { _gradientDegreeLayer.Resolve(flavors, isStyled, _stylePart, 45.0f, [this](const FlavorPart& part) -> const std::optional<f32>& { return pickFlavor(part).GetGradientDegree(); }) })
        FillStyle::SetGradientDegree(*degree);

    if(const f32* offset { _gradientOffsetLayer.Resolve(flavors, isStyled, _stylePart, 0.0f, [this](const FlavorPart& part) -> const std::optional<f32>& { return pickFlavor(part).GetGradientOffset(); }) })
        FillStyle::SetGradientOffset(*offset);

    if(const f32* speed { _gradientAnimationSpeedLayer.Resolve(flavors, isStyled, _stylePart, 0.018f, [this](const FlavorPart& part) -> const std::optional<f32>& { return pickFlavor(part).GetGradientAnimationSpeed(); }) })
        FillStyle::SetGradientAnimationSpeed(*speed);

    if(const f32* speed { _gradientRotationSpeedLayer.Resolve(flavors, isStyled, _stylePart, 1.2f, [this](const FlavorPart& part) -> const std::optional<f32>& { return pickFlavor(part).GetGradientRotationSpeed(); }) })
        FillStyle::SetGradientRotationSpeed(*speed);
}

void RetroFuturaGUI::StateFillStyle::resolveShown()
{
    if(const bool* shown { _shownLayer.Resolve(getFlavors(), nullptr != _flavors, _stylePart, false, [this](const FlavorPart& part) -> const std::optional<bool>& { return pickFlavor(part).GetShown(); }) })
        _isShown = *shown;
}

std::span<const u32> RetroFuturaGUI::StateFillStyle::getFlavors() const
{
    if(!_flavors)
        return {};

    return *_flavors;
}
