#include "Border.hpp"
#include <utility>

RetroFuturaGUI::Border::Border(Projection* projection, BorderHooks hooks)
    : IWidgetDecoration(projection, std::move(hooks._OnCornerRadiiChange)), _onWidthChange(std::move(hooks._OnWidthChange))
{
    _borderFrame = std::make_unique<Frame>(_projection);
    _colorsEnabled = { glm::vec4(0.25f, 0.25f, 0.25f, 1.0f) };
    _colorsDisabled = { glm::vec4(0.2f, 0.2f, 0.2f, 1.0f) };
    _colorsClicked = { glm::vec4(0.3f, 0.3f, 1.3f, 1.0f) };
    _colorsHover = { glm::vec4(0.275f, 0.275f, 0.275f, 1.0f) };
    setColors();
}

void RetroFuturaGUI::Border::Draw()
{
    if(!_borderFrame)
        return;

    _borderFrame->Draw();
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
    switch(state)
    {
        case ColorState::Clicked:
            return _colorsClicked;
        case ColorState::Disabled:
            return _colorsDisabled;
        case ColorState::Hover:
            return _colorsHover;
        case ColorState::Selected:
            return _colorsSelected;
        default: // Enabled
            return _colorsEnabled;
    }
}

void RetroFuturaGUI::Border::SetFillType(const FillType fillType)
{
    if(!_borderFrame)
        return;

    _borderFrame->SetFillType(fillType);
}

void RetroFuturaGUI::Border::SetGradientOffset(const f32 gradientOffset)
{
    if(!_borderFrame)
        return;

    _borderFrame->SetGradientOffset(gradientOffset);
}

void RetroFuturaGUI::Border::SetGradientAnimationSpeed(const f32 animationSpeed)
{
    if(!_borderFrame)
        return;

    _borderFrame->SetGradientAnimationSpeed(animationSpeed);
}

void RetroFuturaGUI::Border::SetGradientDegree(const f32 degree)
{
    if(!_borderFrame)
        return;

    _borderFrame->SetGradientDegree(degree);
}

void RetroFuturaGUI::Border::SetGradientRotationSpeed(const f32 rotationSpeed)
{
    if(!_borderFrame)
        return;

    _borderFrame->SetGradientRotationSpeed(rotationSpeed);
}

void RetroFuturaGUI::Border::SetWindowImageTextureID(const u32 textureID)
{
    if(!_borderFrame)
        return;

    _borderFrame->SetWindowBackgroundImageTextureID(textureID);
}

void RetroFuturaGUI::Border::SetPrimaryRasterColor(const glm::vec4& color)
{
    if(!_borderFrame)
        return;

    _borderFrame->SetPrimaryRasterColor(color);
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

    switch(_colorState)
    {
        case ColorState::Disabled:
            _borderFrame->SetColors(_colorsDisabled);
        break;
        case ColorState::Clicked:
            _borderFrame->SetColors(_colorsClicked);
        break;
        case ColorState::Hover:
            _borderFrame->SetColors(_colorsHover);
        break;
        case ColorState::Selected:
            _borderFrame->SetColors(_colorsSelected);
        break;
        default: //Enabled
            _borderFrame->SetColors(_colorsEnabled);
    }
}

void RetroFuturaGUI::Border::SetWidth(const f32 borderWidth)
{
    if(!_borderFrame)
        return;

    _borderFrame->SetBorderWidth(borderWidth);

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