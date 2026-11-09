#include "Border.hpp"
#include "Background.hpp"
#include <utility>

RetroFuturaGUI::Border::Border(Projection* projection, BorderHooks hooks)
    : IWidgetDecoration(projection, std::move(hooks._OnCornerRadiiChange)), _onWidthChange(std::move(hooks._OnWidthChange))
{
    _borderFrame = std::make_unique<Frame>(_projection);
    _shadowRectangle = std::make_unique<ShadowRectangle>(_projection, &_dropShadow);
    _glowRectangle = std::make_unique<GlowRectangle>(_projection, &_glow);
    _innerGlowRectangle = std::make_unique<InnerGlowRectangle>(_projection, &_innerGlow);
    _builtInColors[static_cast<uSize>(ColorState::Enabled)] = { glm::vec4(0.25f, 0.25f, 0.25f, 1.0f) };
    _builtInColors[static_cast<uSize>(ColorState::Disabled)] = { glm::vec4(0.2f, 0.2f, 0.2f, 1.0f) };
    _builtInColors[static_cast<uSize>(ColorState::Clicked)] = { glm::vec4(0.3f, 0.3f, 1.3f, 1.0f) };
    _builtInColors[static_cast<uSize>(ColorState::Hover)] = { glm::vec4(0.275f, 0.275f, 0.275f, 1.0f) };
    _builtInWidth = _borderFrame->GetBorderWidth();
    setColors();
}

void RetroFuturaGUI::Border::Draw()
{
    if(!_borderFrame)
        return;

    followStyles();
    drawShadow();
    drawGlow();
    _borderFrame->Draw();
    drawInnerGlow();
    drawBackgroundGlow();
}

void RetroFuturaGUI::Border::setCornerShape()
{
    if(!_borderFrame)
        return;

    _borderFrame->SetCornerRadii(_cornerRadii);
    _borderFrame->SetCutCorners(_cutCorners);
}

void RetroFuturaGUI::Border::SetTransitionSettings(const TransitionSettings* settings)
{
    if(!_borderFrame)
        return;

    _borderFrame->SetTransitionSettings(settings);
    _dropShadow.SetTransitionSettings(settings);
    _glow.SetTransitionSettings(settings);
    _innerGlow.SetTransitionSettings(settings);
}

void RetroFuturaGUI::Border::SetSize(const glm::vec2& size)
{
    if(!_borderFrame)
        return;

    _borderFrame->SetSize(size);
}

void RetroFuturaGUI::Border::SetPosition(const glm::vec3& position)
{
    if(!_borderFrame)
        return;

    _borderFrame->SetPosition(position);
}

void RetroFuturaGUI::Border::SetRotation(const glm::vec3& rotation)
{
    if(!_borderFrame)
        return;

    _borderFrame->SetRotation(rotation);
}

glm::vec2 RetroFuturaGUI::Border::GetSize() const
{
    if(!_borderFrame)
        return glm::vec2(0.0f);

    return _borderFrame->GetSize();
}

glm::vec3 RetroFuturaGUI::Border::GetPosition() const
{
    if(!_borderFrame)
        return glm::vec3(0.0f);

    return _borderFrame->GetPosition();
}

glm::vec3 RetroFuturaGUI::Border::GetRotation() const
{
    if(!_borderFrame)
        return glm::vec3(0.0f);

    return _borderFrame->GetRotation();
}

const std::vector<glm::vec4>& RetroFuturaGUI::Border::GetColors(const ColorState state) const
{
    return getOwnColors(state);
}

void RetroFuturaGUI::Border::SetFillType(const FillType fillType)
{
    _fill.SetFillType(fillType);
    setFill();
}

void RetroFuturaGUI::Border::SetGradientOffset(const f32 gradientOffset)
{
    _fill.SetGradientOffset(gradientOffset);
    setFill();
}

void RetroFuturaGUI::Border::SetGradientAnimationSpeed(const f32 animationSpeed)
{
    _fill.SetGradientAnimationSpeed(animationSpeed);
    setFill();
}

void RetroFuturaGUI::Border::SetGradientDegree(const f32 degree)
{
    _fill.SetGradientDegree(degree);
    setFill();
}

void RetroFuturaGUI::Border::SetGradientRotationSpeed(const f32 rotationSpeed)
{
    _fill.SetGradientRotationSpeed(rotationSpeed);
    setFill();
}

void RetroFuturaGUI::Border::SetWindowImageTextureID(const u32 textureID)
{
    if(!_borderFrame)
        return;

    _borderFrame->SetWindowBackgroundImageTextureID(textureID);
}

void RetroFuturaGUI::Border::SetRasterPattern(const RasterPattern rasterPattern)
{
    if(!_borderFrame)
        return;

    _borderFrame->SetRasterPattern(rasterPattern);
}

void RetroFuturaGUI::Border::SetPrimaryRasterColor(const glm::vec4& color)
{
    if(!_borderFrame)
        return;

    _borderFrame->SetPrimaryRasterColor(color);
}

void RetroFuturaGUI::Border::SetSecondaryRasterColor(const glm::vec4& color)
{
    if(!_borderFrame)
        return;

    _borderFrame->SetSecondaryRasterColor(color);
}

void RetroFuturaGUI::Border::SetRasterSpacing(const f32 spacing)
{
    if(!_borderFrame)
        return;

    _borderFrame->SetRasterSpacing(spacing);
}

void RetroFuturaGUI::Border::SetRasterDegree(const f32 degree)
{
    if(!_borderFrame)
        return;

    _borderFrame->SetRasterDegree(degree);
}

void RetroFuturaGUI::Border::SetPrimaryRasterWidthTransfer(std::span<f32> widthTransfer)
{
    if(!_borderFrame)
        return;

    _primaryRasterWidthTransfer.assign(widthTransfer.begin(), widthTransfer.end());
    _borderFrame->SetPrimaryRasterWidthTransfer(_primaryRasterWidthTransfer);
}

void RetroFuturaGUI::Border::SetDotTransparencyTransfer(const f32 transparencyTransfer)
{
    if(!_borderFrame)
        return;

    _borderFrame->SetDotTransparencyTransfer(transparencyTransfer);
}

void RetroFuturaGUI::Border::SetRasterAnimationSpeed(const f32 animationSpeed)
{
    if(!_borderFrame)
        return;

    _borderFrame->SetRasterAnimationSpeed(animationSpeed);
}

void RetroFuturaGUI::Border::setColors()
{
    if(!_borderFrame)
        return;

    _borderFrame->SetColors(GetShownColors(_colorState));

    _dropShadow.SetColorState(_colorState);
    _glow.SetColorState(_colorState);
    _innerGlow.SetColorState(_colorState);
}

void RetroFuturaGUI::Border::SetWidth(const f32 borderWidth)
{
    _ownWidth = borderWidth;
    resolveWidth();
}

void RetroFuturaGUI::Border::ResetWidth()
{
    _ownWidth.reset();
    resolveWidth();
}

void RetroFuturaGUI::Border::SetBuiltInWidth(const StyleKey&, const f32 borderWidth)
{
    _builtInWidth = borderWidth;
    resolveWidth();
}

void RetroFuturaGUI::Border::restyle()
{
    IWidgetDecoration::restyle();
    resolveWidth();
}

void RetroFuturaGUI::Border::resolveWidth()
{
    if(!_borderFrame)
        return;

    //A border without a widget's flavors (a sub-part, a window's) goes from its own width straight to its built-in one.
    const f32 width { _ownWidth.value_or(_flavors ? StylingManager::FindWidth(getFlavors(), _stylePart).value_or(_builtInWidth) : _builtInWidth) };

    if(width == _borderFrame->GetBorderWidth())
        return;

    _borderFrame->SetBorderWidth(width);

    if(!_onWidthChange)
        return;

    _onWidthChange();
}

void RetroFuturaGUI::Border::SetGaps(std::span<BorderGap> gaps)
{
    if(!_borderFrame)
        return;

    _borderFrame->SetBorderGaps(gaps);
}

f32 RetroFuturaGUI::Border::GetWidth() const
{
    if(!_borderFrame)
        return 0.0f;

    return _borderFrame->GetBorderWidth();
}

RetroFuturaGUI::DropShadow& RetroFuturaGUI::Border::GetDropShadow()
{
    return _dropShadow;
}

const RetroFuturaGUI::DropShadow& RetroFuturaGUI::Border::GetDropShadow() const
{
    return _dropShadow;
}

void RetroFuturaGUI::Border::drawShadow()
{
    if(!_borderFrame)
        return;

    if(!_shadowRectangle)
        return;

    if(!_dropShadow.IsShown())
        return;

    _shadowRectangle->SetCaster(ShadowCaster
    {
        ._Size = _borderFrame->GetSize(),
        ._Position = _borderFrame->GetPosition(),
        ._CornerRadii = _borderFrame->GetShownCornerRadii(),
        ._CutCorners = _borderFrame->GetCutCorners(),
        ._RingWidth = _borderFrame->GetBorderWidth()
    });
    _shadowRectangle->Draw();
}

RetroFuturaGUI::Glow& RetroFuturaGUI::Border::GetGlow()
{
    return _glow;
}

const RetroFuturaGUI::Glow& RetroFuturaGUI::Border::GetGlow() const
{
    return _glow;
}

void RetroFuturaGUI::Border::EnableGlowOverBackground(const bool enable)
{
    _isGlowOverBackground = enable;
}

bool RetroFuturaGUI::Border::IsGlowOverBackgroundEnabled() const
{
    return _isGlowOverBackground;
}

void RetroFuturaGUI::Border::drawGlow()
{
    if(!_borderFrame)
        return;

    if(!_glowRectangle)
        return;

    if(!_glow.IsShown())
        return;

    const glm::vec3 position { glm::vec2(_borderFrame->GetPosition()), _borderFrame->GetPosition().z - _glowDepthOffset };
    _glowRectangle->SetCaster(ShadowCaster
    {
        ._Size = _borderFrame->GetSize(),
        ._Position = position,
        ._CornerRadii = _borderFrame->GetShownCornerRadii(),
        ._CutCorners = _borderFrame->GetCutCorners(),
        ._RingWidth = _borderFrame->GetBorderWidth()
    }, GlowEdge::Outer);
    _glowRectangle->Draw();
}

void RetroFuturaGUI::Border::drawInwardGlow(const f32 depth)
{
    if(!_borderFrame)
        return;

    if(!_glowRectangle)
        return;

    if(!_glow.IsShown())
        return;

    if(!_hasBackgroundGap)
        return;

    const glm::vec3 position { glm::vec2(_borderFrame->GetPosition()), depth };
    _glowRectangle->SetCaster(ShadowCaster
    {
        ._Size = _borderFrame->GetSize(),
        ._Position = position,
        ._CornerRadii = _borderFrame->GetShownCornerRadii(),
        ._CutCorners = _borderFrame->GetCutCorners(),
        ._RingWidth = _borderFrame->GetBorderWidth()
    }, GlowEdge::Inner);
    _glowRectangle->Draw();
}

void RetroFuturaGUI::Border::drawBackgroundGlow()
{
    if(!_borderFrame)
        return;

    if(!_linkedBackground)
        return;

    if(!_linkedBackground->_isGlowOverBorder)
        return;

    _linkedBackground->drawGlow(_borderFrame->GetPosition().z + _coverDepthOffset);
}

RetroFuturaGUI::InnerGlow& RetroFuturaGUI::Border::GetInnerGlow()
{
    return _innerGlow;
}

const RetroFuturaGUI::InnerGlow& RetroFuturaGUI::Border::GetInnerGlow() const
{
    return _innerGlow;
}

void RetroFuturaGUI::Border::drawInnerGlow()
{
    if(!_borderFrame)
        return;

    if(!_innerGlowRectangle)
        return;

    if(!_innerGlow.IsShown())
        return;

    _innerGlowRectangle->SetCaster(ShadowCaster
    {
        ._Size = _borderFrame->GetSize(),
        ._Position = _borderFrame->GetPosition() + glm::vec3(0.0f, 0.0f, _innerGlowDepthOffset),
        ._CornerRadii = _borderFrame->GetShownCornerRadii(),
        ._CutCorners = _borderFrame->GetCutCorners(),
        ._RingWidth = _borderFrame->GetBorderWidth()
    });
    _innerGlowRectangle->Draw();
}

void RetroFuturaGUI::Border::SetGlassNoise(const GlassNoise& noise)
{
    if(!_borderFrame)
        return;

    _borderFrame->SetGlassNoise(noise);
}

void RetroFuturaGUI::Border::setFill()
{
    if(!_borderFrame)
        return;

    _fill.Apply(*_borderFrame, getFlavors(), nullptr != _flavors, _stylePart);
}