#include "FillStyle.hpp"
#include <algorithm>

void RetroFuturaGUI::FillStyle::SetTransitionSettings(const TransitionSettings* settings)
{
    _transitionSettings = settings;
}

void RetroFuturaGUI::FillStyle::SetColors(std::span<const glm::vec4> colors)
{
    _colors = colors;
    _colorCount = std::min<uSize>(_colors.size(), 255);
}

std::span<const glm::vec4> RetroFuturaGUI::FillStyle::getShownColors()
{
    const std::span<const glm::vec4> target { _colors.first(_colorCount) };

    if(!_transitionSettings)
        return target;

    const std::chrono::steady_clock::time_point now { std::chrono::steady_clock::now() };

    //Compared at every draw, since the widget may change the colors in its own vector without handing them over again
    if(!_hasShownColors || !std::ranges::equal(target, _targetColors))
        startColorTransition(target, now);

    if(!_isColorTransitionRunning)
        return _shownColors;

    const f32 progress { Transitions::GetProgress(_colorTransitionStart, now, Transitions::Resolve(*_transitionSettings, TransitionProperty::Colors)) };

    if(1.0f <= progress)
    {
        _isColorTransitionRunning = false;
        _shownColors = _targetColors;
        return _shownColors;
    }

    //Both sides get as many stops as the larger one has, so each stop blends with its partner
    const uSize stopCount { std::max(_fromColors.size(), _targetColors.size()) };
    Transitions::ResampleStops(_fromColors, stopCount, _fromStops);
    Transitions::ResampleStops(_targetColors, stopCount, _targetStops);
    _shownColors.resize(stopCount);

    for(uSize stop = 0; stop < stopCount; ++stop)
        _shownColors[stop] = Transitions::BlendColor(_fromStops[stop], _targetStops[stop], progress);

    return _shownColors;
}

void RetroFuturaGUI::FillStyle::startColorTransition(std::span<const glm::vec4> target, const std::chrono::steady_clock::time_point now)
{
    const Transition transition { Transitions::Resolve(*_transitionSettings, TransitionProperty::Colors) };
    const bool isInstant { !_hasShownColors || _shownColors.empty() || target.empty() || 0 >= transition._Duration.count() };
    _targetColors.assign(target.begin(), target.end());
    _hasShownColors = true;

    if(isInstant)
    {
        _isColorTransitionRunning = false;
        _shownColors = _targetColors;
        return;
    }

    _fromColors = _shownColors; //from where the colors are now, which is midway when the last transition hasn't arrived yet
    _colorTransitionStart = now;
    _isColorTransitionRunning = true;
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

void RetroFuturaGUI::FillStyle::SetBackgroundImageTextureID(const u32 textureID)
{
    _backgroundImageTextureID = textureID;

    if(0 == textureID) //0 is no texture
        _shaderFeatureDIP &= ~ShaderFeatures::BackgroundImage;
    else
        _shaderFeatureDIP |= ShaderFeatures::BackgroundImage;
}

void RetroFuturaGUI::FillStyle::SetBackgroundImagePadding(const f32 padding)
{
    _backgroundImagePadding = padding;
}

void RetroFuturaGUI::FillStyle::SetBackgroundImageFit(const ImageFit fit)
{
    _backgroundImageFit = fit;
}

void RetroFuturaGUI::FillStyle::SetMaskTextureID(const u32 textureID)
{
    _maskTextureID = textureID;

    if(0 == textureID) //0 is no texture
        _shaderFeatureDIP &= ~ShaderFeatures::Mask;
    else
        _shaderFeatureDIP |= ShaderFeatures::Mask;
}

void RetroFuturaGUI::FillStyle::SetMaskMode(const MaskMode mode)
{
    _maskMode = mode;
}

void RetroFuturaGUI::FillStyle::SetMaskFit(const ImageFit fit)
{
    _maskFit = fit;
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
    const std::span<const glm::vec4> colors { getShownColors() };

    if(_fillType == FillType::Solid)
    {
        shader.SetUniformVec4("uColor", colors[0]);
    }
    else
    {
        _gradientOffset += _gradientAnimationSpeed;
        if (1.0f < _gradientOffset)
            _gradientOffset = 0.0f;

        _gradientDegree += _gradientRotationSpeed;
        if(360.0f <= _gradientDegree)
            _gradientDegree = 0.0f;

        shader.SetUniformVec4("uColors", &colors[0][0], static_cast<u32>(colors.size()));
        shader.SetUniformInt("uNumColors", static_cast<i32>(colors.size()));
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

void RetroFuturaGUI::FillStyle::uploadImage(Shader& shader, const glm::vec2& size) const
{
    uploadBackgroundImage(shader, size);
    uploadMask(shader, size);
}

void RetroFuturaGUI::FillStyle::uploadBackgroundImage(Shader& shader, const glm::vec2& size) const
{
    if(0 == (_shaderFeatureDIP & ShaderFeatures::BackgroundImage))
        return;

    //The image is placed in UV space, so the pixel padding goes up as a fraction of each axis
    glm::vec2 paddingUV { 0.0f };

    if(0.0f < size.x && 0.0f < size.y)
        paddingUV = glm::vec2(_backgroundImagePadding) / size;

    //Unit 1, apart from the glass. Unit 0 is made active again for whatever binds a texture next
    shader.SetUniformInt("uBackgroundImage", 1);
    shader.SetUniformVec2("uImagePadding", paddingUV);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, _backgroundImageTextureID);
    shader.SetUniformVec2("uImageCoverage", getCoverage(_backgroundImageFit, size - 2.0f * glm::vec2(_backgroundImagePadding))); //fitted inside the padding
    glActiveTexture(GL_TEXTURE0);
}

void RetroFuturaGUI::FillStyle::uploadMask(Shader& shader, const glm::vec2& size) const
{
    if(0 == (_shaderFeatureDIP & ShaderFeatures::Mask))
        return;

    //Unit 3, after the glyphs, the image and the glass
    shader.SetUniformInt("uMask", 3);
    shader.SetUniformInt("uMaskMode", static_cast<i32>(_maskMode));
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, _maskTextureID);
    shader.SetUniformVec2("uMaskCoverage", getCoverage(_maskFit, size));
    glActiveTexture(GL_TEXTURE0);
}

glm::vec2 RetroFuturaGUI::FillStyle::getCoverage(const ImageFit fit, const glm::vec2& size)
{
    if(ImageFit::Stretch == fit)
        return glm::vec2(1.0f);

    GLint
        width { 0 },
        height { 0 };
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height);

    if(0 >= width || 0 >= height || 0.0f >= size.x || 0.0f >= size.y)
        return glm::vec2(1.0f);

    //Scaled by the same factor both ways: the larger one covers the area, the smaller one fits into it
    const glm::vec2
        textureSize { static_cast<f32>(width), static_cast<f32>(height) },
        factors { size / textureSize };
    const f32 factor { ImageFit::Fill == fit ? std::max(factors.x, factors.y) : std::min(factors.x, factors.y) };
    return textureSize * factor / size;
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