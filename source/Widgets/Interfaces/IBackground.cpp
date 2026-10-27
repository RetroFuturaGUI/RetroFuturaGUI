#include "IBackground.hpp"
#include "ResourceManager.hpp"
#include <algorithm>
#include <cctype>

void RetroFuturaGUI::IBackground::SetBackgroundColors(std::span<const glm::vec4> colors, const ColorState state)
{
    switch(state)
    {
        case ColorState::Clicked:
            _backgroundColorClicked.assign(colors.begin(), colors.end());
        break;
        case ColorState::Disabled:
            _backgroundColorDisabled.assign(colors.begin(), colors.end());
        break;
        case ColorState::Hover:
            _backgroundColorHover.assign(colors.begin(), colors.end());
        break;
        default: // Enabled
            _backgroundColorEnabled.assign(colors.begin(), colors.end());
    }

    setBackgroundColors();
}

void RetroFuturaGUI::IBackground::SetBackgroundGradientOffset(const f32 gradientOffset)
{
    if(!_background)
        return;

    _background->SetGradientOffset(gradientOffset);
}

void RetroFuturaGUI::IBackground::SetBackgroundGradientAnimationSpeed(const f32 animationSpeed)
{
    if(!_background)
        return;

    _background->SetGradientAnimationSpeed(animationSpeed);
}

void RetroFuturaGUI::IBackground::SetBackgroundGradientDegree(const f32 degree)
{
    if(!_background)
        return;

    _background->SetGradientDegree(degree);
}

void RetroFuturaGUI::IBackground::SetBackgroundGradientRotationSpeed(const f32 rotationSpeed)
{    
    if(!_background)
        return;

    _background->SetGradientRotationSpeed(rotationSpeed);
}

const std::vector<glm::vec4>& RetroFuturaGUI::IBackground::GetBackgroundColors(const ColorState state) const
{
    switch(state)
    {
        case ColorState::Clicked:
            return _backgroundColorClicked;
        case ColorState::Disabled:
            return _backgroundColorDisabled;
        case ColorState::Hover:
            return _backgroundColorHover;
        default: // Enabled
            return _backgroundColorEnabled;
    }
}

void RetroFuturaGUI::IBackground::SetBackgroundFillType(const FillType fillType)
{
    _backgroundFillType = fillType;

    if(!_background)
        return;

    _background->SetFillType(fillType);
}

void RetroFuturaGUI::IBackground::SetBackgroundGaps(const BackgroundGap& gap)
{
    if(!_background)
        return;

    _background->SetBackgroundGaps(gap);
}

void RetroFuturaGUI::IBackground::SetWindowBackgroundImageTextureID(const u32 textureID)
{
    if(!_background)
        return;

    _background->SetWindowBackgroundImageTextureID(textureID);
}

void RetroFuturaGUI::IBackground::SetBackgroundPrimaryRasterColor(const glm::vec4& color)
{
    if(!_background)
        return;

    _background->SetPrimaryRasterColor(color);
}

void RetroFuturaGUI::IBackground::SetBackgroundRasterSpacing(const f32 spacing)
{
    if(!_background)
        return;

    _background->SetRasterSpacing(spacing);
}

void RetroFuturaGUI::IBackground::SetBackgroundRasterDegree(const f32 degree)
{
    if(!_background)
        return;

    _background->SetRasterDegree(degree);
}

void RetroFuturaGUI::IBackground::SetBackgroundPrimaryRasterWidthTransfer(std::span<f32> widthTransfer)
{
    _backgroundPrimaryRasterWidthTransfer.assign(widthTransfer.begin(), widthTransfer.end());

    if(!_background)
        return;

    _background->SetPrimaryRasterWidthTransfer(_backgroundPrimaryRasterWidthTransfer);
}

void RetroFuturaGUI::IBackground::SetBackgroundDotTransparencyTransfer(const f32 transparencyTransfer)
{
    if(!_background)
        return;

    _background->SetDotTransparencyTransfer(transparencyTransfer);
}

void RetroFuturaGUI::IBackground::SetBackgroundRasterAnimationSpeed(const f32 animationSpeed)
{
    if(!_background)
        return;

    _background->SetRasterAnimationSpeed(animationSpeed);
}

void RetroFuturaGUI::IBackground::SetBackgroundFogAlpha(const f32 alpha)
{
    if(!_background)
        return;

    _background->SetFogAlpha(alpha);
}

void RetroFuturaGUI::IBackground::SetBackgroundFogSpeed(const f32 speed)
{
    if(!_background)
        return;

    _background->SetFogSpeed(speed);
}

void RetroFuturaGUI::IBackground::SetBackgroundFogDensity(std::span<f32> density)
{
    _backgroundFogDensity.assign(density.begin(), density.end());

    if(!_background)
        return;

    _background->SetFogDensity(_backgroundFogDensity);
}

void RetroFuturaGUI::IBackground::SetBackgroundFogClearing(const f32 clearing)
{
    if(!_background)
        return;

    _background->SetFogClearing(clearing);
}

void RetroFuturaGUI::IBackground::SetBackgroundImage(std::string_view imagePath)
{
    if(!_background)
        return;

    if(isSvgPath(imagePath))
    {
        _backgroundImage = nullptr;
        _background->SetBackgroundImageTextureID(0);
        _backgroundSvg = ResourceManager::CreateSvgTexture(imagePath, &_background->GetProjection());
        return;
    }

    _backgroundSvg.reset();
    _backgroundImage = ResourceManager::GetTexture(imagePath);

    if(!_backgroundImage)
        return;

    _background->SetBackgroundImageTextureID(_backgroundImage->GetID());
}

void RetroFuturaGUI::IBackground::SetBackgroundImagePadding(const f32 padding)
{
    _backgroundImagePadding = padding;

    if(!_background)
        return;

    _background->SetBackgroundImagePadding(padding);
}

void RetroFuturaGUI::IBackground::SetBackgroundImagePathFill(std::string_view name, const SvgPathFill& fill)
{
    if(!_backgroundSvg)
        return;

    _backgroundSvg->SetPathFill(name, fill);
}

void RetroFuturaGUI::IBackground::ClearBackgroundImagePathFill(std::string_view name)
{
    if(!_backgroundSvg)
        return;

    _backgroundSvg->ClearPathFill(name);
}

void RetroFuturaGUI::IBackground::setBackgroundCornerRadii(const glm::vec4& radii)
{
    if(!_background)
        return;

    _background->SetCornerRadii(radii);
}

void RetroFuturaGUI::IBackground::setBackgroundColorElement(std::vector<glm::vec4>& color)
{
    if(!_background)
        return;

    _background->SetColors(color);
}

void RetroFuturaGUI::IBackground::setBackgroundColors()
{
    switch(_backgroundColorState)
    {
        case ColorState::Enabled:
            setBackgroundColorElement(_backgroundColorEnabled);
        break;
        case ColorState::Clicked:
            setBackgroundColorElement(_backgroundColorClicked);
        break;
        case ColorState::Hover:
            setBackgroundColorElement(_backgroundColorHover);
        break;
        default: //Disabled
            setBackgroundColorElement(_backgroundColorDisabled);
    }
}

void RetroFuturaGUI::IBackground::drawBackground()
{
    if(!_background)
        return;

    _background->Draw();
    drawBackgroundSvg();
}

void RetroFuturaGUI::IBackground::drawBackgroundSvg()
{
    if(!_background)
        return;

    if(!_backgroundSvg)
        return;

    //Widgets place _background themselves, so the SVG follows it here. SvgTexture rebuilds a matrix only for a value that changed
    const glm::vec2 size { glm::max(_background->GetSize() - glm::vec2(_backgroundImagePadding * 2.0f), glm::vec2(0.0f)) };
    _backgroundSvg->SetSize(size);
    _backgroundSvg->SetPosition(_background->GetPosition() + glm::vec3(0.0f, 0.0f, 0.005f));
    _backgroundSvg->SetRotation(_background->GetRotation());
    _backgroundSvg->Draw();
}

bool RetroFuturaGUI::IBackground::isSvgPath(std::string_view path)
{
    constexpr std::string_view extension { ".svg" };

    if(path.size() < extension.size())
        return false;

    const std::string_view end { path.substr(path.size() - extension.size()) };

    return std::equal(end.begin(), end.end(), extension.begin(), [](const char left, const char right)
        {
            return std::tolower(static_cast<unsigned char>(left)) == right;
        });
}