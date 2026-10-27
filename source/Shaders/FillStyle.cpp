#include "FillStyle.hpp"
#include <algorithm>

void RetroFuturaGUI::FillStyle::SetColors(std::span<const glm::vec4> colors)
{
    _colors = colors;
    _colorCount = std::min<uSize>(_colors.size(), 255);
}

void RetroFuturaGUI::FillStyle::SetFillType(const FillType fillType)
{
    _fillType = fillType;
}

void RetroFuturaGUI::FillStyle::SetGradientOffset(const f32 gradientOffset)
{
    _gradientOffset = gradientOffset;
}

void RetroFuturaGUI::FillStyle::SetGradientAnimationSpeed(const f32 animationSpeed)
{
    _gradientAnimationSpeed = animationSpeed;
}

void RetroFuturaGUI::FillStyle::SetGradientDegree(const f32 degree)
{
    _gradientDegree = degree;
}

void RetroFuturaGUI::FillStyle::SetGradientRotationSpeed(const f32 rotationSpeed)
{
    _gradientRotationSpeed = rotationSpeed;
}

f32 RetroFuturaGUI::FillStyle::GetGradientOffset() const
{
    return _gradientOffset;
}

f32 RetroFuturaGUI::FillStyle::GetGradientDegree() const
{
    return _gradientDegree;
}

void RetroFuturaGUI::FillStyle::SetWindowBackgroundImageTextureID(const u32 textureID)
{
    _windowBackgroundTextureID = textureID;

    if(0 == textureID) //0 is no texture
        _shaderFeatureDIP &= ~ShaderFeatures::GlassEffectWithImage;
    else
        _shaderFeatureDIP |= ShaderFeatures::GlassEffectWithImage;
}

void RetroFuturaGUI::FillStyle::SetRasterPattern(const RasterPattern rasterPattern)
{
    _rasterPattern = rasterPattern;
}

void RetroFuturaGUI::FillStyle::SetPrimaryRasterColor(const glm::vec4& color)
{
    _primaryRasterColor = color;
}

void RetroFuturaGUI::FillStyle::SetSecondaryRasterColor(const glm::vec4& color)
{
    _secondaryRasterColor = color;
}

void RetroFuturaGUI::FillStyle::SetRasterSpacing(const f32 spacing)
{
    _rasterSpacing = std::max(spacing, 0.0f);
}

void RetroFuturaGUI::FillStyle::SetRasterDegree(const f32 degree)
{
    _rasterDegree = degree;
}

void RetroFuturaGUI::FillStyle::SetPrimaryRasterWidthTransfer(std::span<f32> widthTransfer)
{
    _primaryRasterWidthTransfer = widthTransfer;
    _primaryRasterWidthTransferCount = std::min<uSize>(_primaryRasterWidthTransfer.size(), 255);

    if(0 < _primaryRasterWidthTransferCount)
        _shaderFeatureDIP |= ShaderFeatures::Raster;
    else
        _shaderFeatureDIP &= ~ShaderFeatures::Raster;
}

void RetroFuturaGUI::FillStyle::SetDotTransparencyTransfer(const f32 transparencyTransfer)
{
    _dotTransparencyTransfer = transparencyTransfer;
}

void RetroFuturaGUI::FillStyle::SetRasterAnimationSpeed(const f32 animationSpeed)
{
    _rasterAnimationSpeed = animationSpeed;
}

void RetroFuturaGUI::FillStyle::uploadFill(Shader& shader, const u32 extraFeatures)
{
    shader.SetUniformInt("uDIP", _shaderFeatureDIP | extraFeatures);

    if(_fillType == FillType::Solid)
    {
        shader.SetUniformVec4("uColor", _colors[0]);
    }
    else
    {
        _gradientOffset += _gradientAnimationSpeed;
        if (1.0f < _gradientOffset)
            _gradientOffset = 0.0f;

        _gradientDegree += _gradientRotationSpeed;
        if(360.0f <= _gradientDegree)
            _gradientDegree = 0.0f;

        shader.SetUniformVec4("uColors", &_colors[0][0], static_cast<u32>(_colorCount));
        shader.SetUniformInt("uNumColors", static_cast<i32>(_colorCount));
        shader.SetUniformFloat("uDegree", _gradientDegree);
        shader.SetUniformFloat("uGradientOffset", _gradientOffset);
    }

    if(_shaderFeatureDIP & ShaderFeatures::Raster)
        uploadRaster(shader);

    if(_shaderFeatureDIP & ShaderFeatures::GlassEffect)
    {
        //Unit 2, since Text keeps its glyph pages on unit 0 and a Plane its image on unit 1
        shader.SetUniformInt("uBackgroundTexture", 2);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, _windowBackgroundTextureID);
        glActiveTexture(GL_TEXTURE0);
    }
}

void RetroFuturaGUI::FillStyle::uploadRaster(Shader& shader)
{
    _rasterAnimationOffset += _rasterAnimationSpeed;

    // The width transfer, the cell size, the primary color, the angle and the animation drive every pattern, so they go up whichever one is selected. Each pattern then adds only the uniforms it owns.
    shader.SetUniformInt("uRasterPattern", static_cast<i32>(_rasterPattern));
    shader.SetUniformVec4("uPrimaryRasterColor", _primaryRasterColor);
    shader.SetUniformFloat("uRasterDegree", _rasterDegree);
    shader.SetUniformFloat("uRasterAnimationOffset", _rasterAnimationOffset);
    shader.SetUniformInt("uPrimaryRasterWidthTransferCount", static_cast<i32>(_primaryRasterWidthTransferCount));

    if(0 < _primaryRasterWidthTransferCount)
        shader.SetUniformFloat("uPrimaryRasterWidthTransfer", _primaryRasterWidthTransfer.data(), static_cast<u32>(_primaryRasterWidthTransferCount));

    //The largest element plus the spacing makes one grid cell. Measured here, since the widths may change after they are set
    f32 largestWidth { 0.0f };

    if(0 < _primaryRasterWidthTransferCount)
        largestWidth = std::ranges::max(_primaryRasterWidthTransfer.first(_primaryRasterWidthTransferCount));

    shader.SetUniformFloat("uRasterCellSize", std::max(largestWidth, 0.0f) + _rasterSpacing);

    switch(_rasterPattern)
    {
        case RasterPattern::Checkered:
        {
            uploadCheckeredUniforms(shader);
        } break;
        default: // RasterPattern::Dotted
        {
            uploadDotUniforms(shader);
        } break;
    }
}

void RetroFuturaGUI::FillStyle::uploadDotUniforms(Shader& shader) const
{
    shader.SetUniformFloat("uDotTransparencyTransfer", _dotTransparencyTransfer);
}

void RetroFuturaGUI::FillStyle::uploadCheckeredUniforms(Shader& shader) const
{
    shader.SetUniformVec4("uSecondaryRasterColor", _secondaryRasterColor);
}