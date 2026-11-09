#include "Graph.hpp"
#include <utility>

RetroFuturaGUI::Graph::Graph(Projection* projection, GraphHooks hooks)
    : IWidgetDecoration(projection, {}), _wavePlane(projection), _onLayoutChange(std::move(hooks._OnLayoutChange))
{
    _colorsEnabled = { glm::vec4(1.0f, 0.0f, 0.0f, 1.0f) };
    _colorsClicked = { glm::vec4(1.0f, 1.0f, 0.0f, 1.0f) };
    _colorsHover = { glm::vec4(0.0f, 0.0f, 1.0f, 1.0f) };
    _colorsDisabled = { glm::vec4(0.5f, 0.5f, 0.5f, 1.0f) };
    setColors();
}

void RetroFuturaGUI::Graph::Show(const bool show)
{
    _isShown = show;
    notifyLayoutChange();
}

bool RetroFuturaGUI::Graph::IsShown() const
{
    return _isShown;
}

void RetroFuturaGUI::Graph::SetMode(const GraphMode mode)
{
    _mode = mode;
    _wavePlane.ShowWave(GraphMode::Wave == _mode);
}

RetroFuturaGUI::GraphMode RetroFuturaGUI::Graph::GetMode() const
{
    return _mode;
}

void RetroFuturaGUI::Graph::SetWaveThickness(const f32 thickness)
{
    _wavePlane.SetWaveThickness(thickness);
}

void RetroFuturaGUI::Graph::SetWaveHeight(const f32 height)
{
    _wavePlane.SetWaveHeight(height);
}

void RetroFuturaGUI::Graph::SetWaveLength(const f32 length)
{
    _wavePlane.SetWaveLength(length);
}

void RetroFuturaGUI::Graph::SetWidth(const f32 width)
{
    _width = width;
    notifyLayoutChange();
}

f32 RetroFuturaGUI::Graph::GetWidth() const
{
    return _width;
}

const std::vector<glm::vec4>& RetroFuturaGUI::Graph::GetColors(const ColorState state) const
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

void RetroFuturaGUI::Graph::SetFillType(const FillType fillType)
{
    _wavePlane.SetFillType(fillType);
}

void RetroFuturaGUI::Graph::SetGradientOffset(const f32 gradientOffset)
{
    _wavePlane.SetGradientOffset(gradientOffset);
}

void RetroFuturaGUI::Graph::SetGradientAnimationSpeed(const f32 animationSpeed)
{
    _wavePlane.SetGradientAnimationSpeed(animationSpeed);
}

void RetroFuturaGUI::Graph::SetGradientDegree(const f32 degree)
{
    _wavePlane.SetGradientDegree(degree);
}

void RetroFuturaGUI::Graph::SetGradientRotationSpeed(const f32 rotationSpeed)
{
    _wavePlane.SetGradientRotationSpeed(rotationSpeed);
}

void RetroFuturaGUI::Graph::SetWindowImageTextureID(const u32 textureID)
{
    _wavePlane.SetWindowBackgroundImageTextureID(textureID);
}

void RetroFuturaGUI::Graph::SetRasterPattern(const RasterPattern rasterPattern)
{
    _wavePlane.SetRasterPattern(rasterPattern);
}

void RetroFuturaGUI::Graph::SetPrimaryRasterColor(const glm::vec4& color)
{
    _wavePlane.SetPrimaryRasterColor(color);
}

void RetroFuturaGUI::Graph::SetSecondaryRasterColor(const glm::vec4& color)
{
    _wavePlane.SetSecondaryRasterColor(color);
}

void RetroFuturaGUI::Graph::SetRasterSpacing(const f32 spacing)
{
    _wavePlane.SetRasterSpacing(spacing);
}

void RetroFuturaGUI::Graph::SetRasterDegree(const f32 degree)
{
    _wavePlane.SetRasterDegree(degree);
}

void RetroFuturaGUI::Graph::SetPrimaryRasterWidthTransfer(std::span<f32> widthTransfer)
{
    _primaryRasterWidthTransfer.assign(widthTransfer.begin(), widthTransfer.end());
    _wavePlane.SetPrimaryRasterWidthTransfer(_primaryRasterWidthTransfer);
}

void RetroFuturaGUI::Graph::SetDotTransparencyTransfer(const f32 transparencyTransfer)
{
    _wavePlane.SetDotTransparencyTransfer(transparencyTransfer);
}

void RetroFuturaGUI::Graph::SetRasterAnimationSpeed(const f32 animationSpeed)
{
    _wavePlane.SetRasterAnimationSpeed(animationSpeed);
}

void RetroFuturaGUI::Graph::SetFogAlpha(const f32 alpha)
{
    _wavePlane.SetFogAlpha(alpha);
}

void RetroFuturaGUI::Graph::SetFogSpeed(const f32 speed)
{
    _wavePlane.SetFogSpeed(speed);
}

void RetroFuturaGUI::Graph::SetFogDensity(std::span<f32> density)
{
    _fogDensity.assign(density.begin(), density.end());
    _wavePlane.SetFogDensity(_fogDensity);
}

void RetroFuturaGUI::Graph::SetFogClearing(const f32 clearing)
{
    _wavePlane.SetFogClearing(clearing);
}

void RetroFuturaGUI::Graph::Draw()
{
    if(!_isShown)
        return;

    _wavePlane.Draw();
}

void RetroFuturaGUI::Graph::SetSize(const glm::vec2& size)
{
    _wavePlane.SetSize(size);
}

void RetroFuturaGUI::Graph::SetPosition(const glm::vec3& position)
{
    _wavePlane.SetPosition(position);
}

void RetroFuturaGUI::Graph::SetRotation(const glm::vec3& rotation)
{
    _wavePlane.SetRotation(rotation);
}

glm::vec2 RetroFuturaGUI::Graph::GetSize() const
{
    return _wavePlane.GetSize();
}

glm::vec3 RetroFuturaGUI::Graph::GetPosition() const
{
    return _wavePlane.GetPosition();
}

glm::vec3 RetroFuturaGUI::Graph::GetRotation() const
{
    return _wavePlane.GetRotation();
}

void RetroFuturaGUI::Graph::SetTransitionSettings(const TransitionSettings* settings)
{
    _wavePlane.SetTransitionSettings(settings);
}

void RetroFuturaGUI::Graph::setColors()
{
    if(ColorState::Selected == _colorState && _colorsSelected.empty())
    {
        _wavePlane.SetColors(_colorsEnabled);
        return;
    }

    _wavePlane.SetColors(GetColors(_colorState));
}

void RetroFuturaGUI::Graph::setCornerShape()
{
    _wavePlane.SetCornerRadii(_cornerRadii);
    _wavePlane.SetCutCorners(_cutCorners);
}

void RetroFuturaGUI::Graph::notifyLayoutChange()
{
    if(!_onLayoutChange)
        return;

    _onLayoutChange();
}

void RetroFuturaGUI::Graph::SetGlassNoise(const GlassNoise& noise)
{
    _wavePlane.SetGlassNoise(noise);
}

void RetroFuturaGUI::Graph::SetFogNoise(const FogNoise& noise)
{
    _wavePlane.SetFogNoise(noise);
}

void RetroFuturaGUI::Graph::SetPixelateBlockSize(const f32 blockSize)
{
    _wavePlane.SetPixelateBlockSize(blockSize);
}