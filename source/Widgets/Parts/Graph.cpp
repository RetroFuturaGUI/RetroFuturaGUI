#include "Graph.hpp"
#include <utility>

RetroFuturaGUI::Graph::Graph(Projection* projection, GraphHooks hooks)
    : IWidgetDecoration(projection, {}), _wavePlane(projection), _onLayoutChange(std::move(hooks._OnLayoutChange))
{
    _builtInColors[static_cast<uSize>(ColorState::Enabled)] = { glm::vec4(1.0f, 0.0f, 0.0f, 1.0f) };
    _builtInColors[static_cast<uSize>(ColorState::Clicked)] = { glm::vec4(1.0f, 1.0f, 0.0f, 1.0f) };
    _builtInColors[static_cast<uSize>(ColorState::Hover)] = { glm::vec4(0.0f, 0.0f, 1.0f, 1.0f) };
    _builtInColors[static_cast<uSize>(ColorState::Disabled)] = { glm::vec4(0.5f, 0.5f, 0.5f, 1.0f) };
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
    return getOwnColors(state);
}

void RetroFuturaGUI::Graph::SetFillType(const FillType fillType)
{
    _shader._FillType.Set(fillType);
    setShader();
}

void RetroFuturaGUI::Graph::SetGradientOffset(const f32 gradientOffset)
{
    _shader._GradientOffset.Set(gradientOffset);
    setShader();
}

void RetroFuturaGUI::Graph::SetGradientAnimationSpeed(const f32 animationSpeed)
{
    _shader._GradientAnimationSpeed.Set(animationSpeed);
    setShader();
}

void RetroFuturaGUI::Graph::SetGradientDegree(const f32 degree)
{
    _shader._GradientDegree.Set(degree);
    setShader();
}

void RetroFuturaGUI::Graph::SetGradientRotationSpeed(const f32 rotationSpeed)
{
    _shader._GradientRotationSpeed.Set(rotationSpeed);
    setShader();
}

void RetroFuturaGUI::Graph::SetWindowImageTextureID(const u32 textureID)
{
    _shader._WindowImageTextureID.Set(textureID);
    setShader();
}

void RetroFuturaGUI::Graph::SetRasterPattern(const RasterPattern rasterPattern)
{
    _shader._RasterPattern.Set(rasterPattern);
    setShader();
}

void RetroFuturaGUI::Graph::SetPrimaryRasterColor(const glm::vec4& color)
{
    _shader._PrimaryRasterColor.Set(color);
    setShader();
}

void RetroFuturaGUI::Graph::SetSecondaryRasterColor(const glm::vec4& color)
{
    _shader._SecondaryRasterColor.Set(color);
    setShader();
}

void RetroFuturaGUI::Graph::SetRasterSpacing(const f32 spacing)
{
    _shader._RasterSpacing.Set(spacing);
    setShader();
}

void RetroFuturaGUI::Graph::SetRasterDegree(const f32 degree)
{
    _shader._RasterDegree.Set(degree);
    setShader();
}

void RetroFuturaGUI::Graph::SetPrimaryRasterWidthTransfer(std::span<f32> widthTransfer)
{
    _shader._PrimaryRasterWidthTransfer.Set(std::vector(widthTransfer.begin(), widthTransfer.end()));
    setShader();
}

void RetroFuturaGUI::Graph::SetDotTransparencyTransfer(const f32 transparencyTransfer)
{
    _shader._DotTransparencyTransfer.Set(transparencyTransfer);
    setShader();
}

void RetroFuturaGUI::Graph::SetRasterAnimationSpeed(const f32 animationSpeed)
{
    _shader._RasterAnimationSpeed.Set(animationSpeed);
    setShader();
}

void RetroFuturaGUI::Graph::SetFogAlpha(const f32 alpha)
{
    _shader._FogAlpha.Set(alpha);
    setShader();
}

void RetroFuturaGUI::Graph::SetFogSpeed(const f32 speed)
{
    _shader._FogSpeed.Set(speed);
    setShader();
}

void RetroFuturaGUI::Graph::SetFogDensity(std::span<f32> density)
{
    _shader._FogDensity.Set(std::vector(density.begin(), density.end()));
    setShader();
}

void RetroFuturaGUI::Graph::SetFogClearing(const f32 clearing)
{
    _shader._FogClearing.Set(clearing);
    setShader();
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
    _wavePlane.SetColors(GetShownColors(_colorState));
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
    _shader._GlassNoise.Set(noise);
    setShader();
}

void RetroFuturaGUI::Graph::SetFogNoise(const FogNoise& noise)
{
    _shader._FogNoise.Set(noise);
    setShader();
}

void RetroFuturaGUI::Graph::SetPixelateBlockSize(const f32 blockSize)
{
    _shader._PixelateBlockSize.Set(blockSize);
    setShader();
}

void RetroFuturaGUI::Graph::SetCrtEffect(const CrtEffect& crtEffect)
{
    _shader._CrtEffect.Set(crtEffect);
    setShader();
}

void RetroFuturaGUI::Graph::setShader()
{
    _shader.Apply(_wavePlane, getFlavors(), nullptr != _flavors, _stylePart);
}