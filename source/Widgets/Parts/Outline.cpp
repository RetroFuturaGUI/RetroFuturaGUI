#include "Outline.hpp"

RetroFuturaGUI::Outline::Outline(Text& text)
    : _text(text)
{
    for(const ColorState state : { ColorState::Enabled, ColorState::Disabled, ColorState::Clicked, ColorState::Hover })
        _builtInColors[static_cast<uSize>(state)] = { glm::vec4(0.0f, 0.0f, 0.0f, 1.0f) };
}

void RetroFuturaGUI::Outline::SetWidth(const f32 width)
{
    _widthLayer.Set(width);
    resolveSettings();
}

f32 RetroFuturaGUI::Outline::GetWidth() const
{
    return _text.GetOutlineWidth();
}

void RetroFuturaGUI::Outline::SetColors(std::span<const glm::vec4> colors, const ColorState state)
{
    _ownColors[static_cast<uSize>(state)].assign(colors.begin(), colors.end());
    SetColorState(_colorState);
}

const std::vector<glm::vec4>& RetroFuturaGUI::Outline::GetColors(const ColorState state) const
{
    return _ownColors[static_cast<uSize>(state)];
}

void RetroFuturaGUI::Outline::SetFillType(const FillType fillType)
{
    _fillTypeLayer.Set(fillType);
    resolveSettings();
}

void RetroFuturaGUI::Outline::SetGradientOffset(const f32 gradientOffset)
{
    _gradientOffsetLayer.Set(gradientOffset);
    resolveSettings();
}

void RetroFuturaGUI::Outline::SetGradientAnimationSpeed(const f32 animationSpeed)
{
    _gradientAnimationSpeedLayer.Set(animationSpeed);
    resolveSettings();
}

void RetroFuturaGUI::Outline::SetGradientDegree(const f32 degree)
{
    _gradientDegreeLayer.Set(degree);
    resolveSettings();
}

void RetroFuturaGUI::Outline::SetGradientRotationSpeed(const f32 rotationSpeed)
{
    _gradientRotationSpeedLayer.Set(rotationSpeed);
    resolveSettings();
}

void RetroFuturaGUI::Outline::ResetSetting(const EffectSetting setting)
{
    switch(setting)
    {
        case EffectSetting::Width:
            _widthLayer.Reset();
        break;
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
        default: //a setting the outline doesn't have
            return;
    }

    resolveSettings();
}

void RetroFuturaGUI::Outline::SetImageTextureID(const u32 textureID)
{
    _text.GetOutline().SetBackgroundImageTextureID(textureID);
}

void RetroFuturaGUI::Outline::SetImagePadding(const f32 padding)
{
    _text.GetOutline().SetBackgroundImagePadding(padding);
}

void RetroFuturaGUI::Outline::SetImageFit(const ImageFit fit)
{
    _text.GetOutline().SetBackgroundImageFit(fit);
}

void RetroFuturaGUI::Outline::SetColorState(const ColorState state)
{
    _colorState = state;
    _text.GetOutline().SetColors(getShownColors(state));
}

void RetroFuturaGUI::Outline::followFlavors(const std::vector<u32>* flavors, const StylePart part)
{
    _flavors = flavors;
    _stylePart = part;
    restyle();
}

void RetroFuturaGUI::Outline::restyle()
{
    SetColorState(_colorState); //handed on by reference, so they go again even when unchanged. A flavor's list may have moved
    resolveSettings();
}

const std::vector<glm::vec4>& RetroFuturaGUI::Outline::getShownColors(const ColorState state) const
{
    const std::vector<glm::vec4>& own { _ownColors[static_cast<uSize>(state)] };

    if(!own.empty())
        return own;

    const std::vector<glm::vec4>* styled { _flavors ? StylingManager::FindFirst<const std::vector<glm::vec4>*>(getFlavors(), _stylePart, [state](const FlavorPart& part) -> const std::vector<glm::vec4>*
    {
        const std::vector<glm::vec4>& colors { part.GetOutline().GetColors(state) };
        return colors.empty() ? nullptr : &colors;
    }) : nullptr };

    if(styled)
        return *styled;

    const std::vector<glm::vec4>& builtIn { _builtInColors[static_cast<uSize>(state)] };

    if(!builtIn.empty() || ColorState::Enabled == state)
        return builtIn;

    return getShownColors(ColorState::Enabled); //a state nothing sets looks like Enabled
}

void RetroFuturaGUI::Outline::resolveSettings()
{
    const std::span<const u32> flavors { getFlavors() };
    const bool isStyled { nullptr != _flavors };

    if(const f32* width { _widthLayer.Resolve(flavors, isStyled, _stylePart, 0.0f, [](const FlavorPart& part) -> const std::optional<f32>& { return part.GetOutline().GetWidth(); }) })
        _text.SetOutlineWidth(*width);

    if(const FillType* fillType { _fillTypeLayer.Resolve(flavors, isStyled, _stylePart, FillType::Solid, [](const FlavorPart& part) -> const std::optional<FillType>& { return part.GetOutline().GetFillType(); }) })
        _text.GetOutline().SetFillType(*fillType);

    if(const f32* degree { _gradientDegreeLayer.Resolve(flavors, isStyled, _stylePart, 45.0f, [](const FlavorPart& part) -> const std::optional<f32>& { return part.GetOutline().GetGradientDegree(); }) })
        _text.GetOutline().SetGradientDegree(*degree);

    if(const f32* offset { _gradientOffsetLayer.Resolve(flavors, isStyled, _stylePart, 0.0f, [](const FlavorPart& part) -> const std::optional<f32>& { return part.GetOutline().GetGradientOffset(); }) })
        _text.GetOutline().SetGradientOffset(*offset);

    if(const f32* speed { _gradientAnimationSpeedLayer.Resolve(flavors, isStyled, _stylePart, 0.018f, [](const FlavorPart& part) -> const std::optional<f32>& { return part.GetOutline().GetGradientAnimationSpeed(); }) })
        _text.GetOutline().SetGradientAnimationSpeed(*speed);

    if(const f32* speed { _gradientRotationSpeedLayer.Resolve(flavors, isStyled, _stylePart, 1.2f, [](const FlavorPart& part) -> const std::optional<f32>& { return part.GetOutline().GetGradientRotationSpeed(); }) })
        _text.GetOutline().SetGradientRotationSpeed(*speed);
}

std::span<const u32> RetroFuturaGUI::Outline::getFlavors() const
{
    if(!_flavors)
        return {};

    return *_flavors;
}