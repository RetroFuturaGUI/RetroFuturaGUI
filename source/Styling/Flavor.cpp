#ifdef RETROFUTURAGUI_MODULES
module;
#include "ModuleExternals.hpp"
module RetroFuturaGUI;
#else
#include "Flavor.hpp"
#include "StylingManager.hpp"
#endif

void RetroFuturaGUI::FlavorPart::SetColors(std::span<const glm::vec4> colors, const ColorState state)
{
    _colors[static_cast<uSize>(state)].assign(colors.begin(), colors.end());
    StylingManager::markChanged();
}

const std::vector<glm::vec4>& RetroFuturaGUI::FlavorPart::GetColors(const ColorState state) const
{
    return _colors[static_cast<uSize>(state)];
}

void RetroFuturaGUI::FlavorPart::Reset(const StyleSetting setting)
{
    switch(setting)
    {
        case StyleSetting::CornerRadii:
            _cornerRadii.reset();
        break;
        case StyleSetting::CutCorners:
            _cutCorners.reset();
        break;
        case StyleSetting::Width:
            _width.reset();
        break;
        case StyleSetting::Image:
            _image.reset();
        break;
        case StyleSetting::FillType:
            _fillType.reset();
        break;
        case StyleSetting::GradientDegree:
            _gradientDegree.reset();
        break;
        case StyleSetting::GradientOffset:
            _gradientOffset.reset();
        break;
        case StyleSetting::GradientAnimationSpeed:
            _gradientAnimationSpeed.reset();
        break;
        case StyleSetting::GradientRotationSpeed:
            _gradientRotationSpeed.reset();
        break;
        case StyleSetting::RasterPattern:
            _rasterPattern.reset();
        break;
        case StyleSetting::PrimaryRasterColor:
            _primaryRasterColor.reset();
        break;
        case StyleSetting::SecondaryRasterColor:
            _secondaryRasterColor.reset();
        break;
        case StyleSetting::RasterSpacing:
            _rasterSpacing.reset();
        break;
        case StyleSetting::RasterDegree:
            _rasterDegree.reset();
        break;
        case StyleSetting::PrimaryRasterWidthTransfer:
            _primaryRasterWidthTransfer.reset();
        break;
        case StyleSetting::DotTransparencyTransfer:
            _dotTransparencyTransfer.reset();
        break;
        case StyleSetting::RasterAnimationSpeed:
            _rasterAnimationSpeed.reset();
        break;
        case StyleSetting::WindowImageTextureID:
            _windowImageTextureID.reset();
        break;
        case StyleSetting::GlassNoise:
            _glassNoise.reset();
        break;
        case StyleSetting::FogAlpha:
            _fogAlpha.reset();
        break;
        case StyleSetting::FogSpeed:
            _fogSpeed.reset();
        break;
        case StyleSetting::FogDensity:
            _fogDensity.reset();
        break;
        case StyleSetting::FogClearing:
            _fogClearing.reset();
        break;
        case StyleSetting::FogNoise:
            _fogNoise.reset();
        break;
        case StyleSetting::ImagePadding:
            _imagePadding.reset();
        break;
        case StyleSetting::ImageFit:
            _imageFit.reset();
        break;
        case StyleSetting::MaskTextureID:
            _maskTextureID.reset();
        break;
        case StyleSetting::MaskMode:
            _maskMode.reset();
        break;
        case StyleSetting::MaskFit:
            _maskFit.reset();
        break;
        case StyleSetting::Gaps:
            _backgroundGaps.reset();
            _borderGaps.reset();
        break;
        case StyleSetting::PixelateBlockSize:
            _pixelateBlockSize.reset();
        break;
        case StyleSetting::CrtEffect:
            _crtEffect.reset();
        break;
    }

    StylingManager::markChanged();
}

void RetroFuturaGUI::FlavorPart::SetCornerRadii(const glm::vec4& radii)
{
    _cornerRadii = radii;
    StylingManager::markChanged();
}

const std::optional<glm::vec4>& RetroFuturaGUI::FlavorPart::GetCornerRadii() const
{
    return _cornerRadii;
}

void RetroFuturaGUI::FlavorPart::SetCutCorners(const CutCorners corners)
{
    _cutCorners = corners;
    StylingManager::markChanged();
}

const std::optional<RetroFuturaGUI::CutCorners>& RetroFuturaGUI::FlavorPart::GetCutCorners() const
{
    return _cutCorners;
}

void RetroFuturaGUI::FlavorPart::SetWidth(const f32 width)
{
    _width = width;
    StylingManager::markChanged();
}

const std::optional<f32>& RetroFuturaGUI::FlavorPart::GetWidth() const
{
    return _width;
}

void RetroFuturaGUI::FlavorPart::SetImage(std::string_view path)
{
    _image = std::string(path);
    StylingManager::markChanged();
}

const std::optional<std::string>& RetroFuturaGUI::FlavorPart::GetImage() const
{
    return _image;
}

void RetroFuturaGUI::FlavorPart::SetFillType(const FillType value)
{
    _fillType = value;
    StylingManager::markChanged();
}

const std::optional<RetroFuturaGUI::FillType>& RetroFuturaGUI::FlavorPart::GetFillType() const
{
    return _fillType;
}

void RetroFuturaGUI::FlavorPart::SetGradientDegree(const f32 value)
{
    _gradientDegree = value;
    StylingManager::markChanged();
}

const std::optional<f32>& RetroFuturaGUI::FlavorPart::GetGradientDegree() const
{
    return _gradientDegree;
}

void RetroFuturaGUI::FlavorPart::SetGradientOffset(const f32 value)
{
    _gradientOffset = value;
    StylingManager::markChanged();
}

const std::optional<f32>& RetroFuturaGUI::FlavorPart::GetGradientOffset() const
{
    return _gradientOffset;
}

void RetroFuturaGUI::FlavorPart::SetGradientAnimationSpeed(const f32 value)
{
    _gradientAnimationSpeed = value;
    StylingManager::markChanged();
}

const std::optional<f32>& RetroFuturaGUI::FlavorPart::GetGradientAnimationSpeed() const
{
    return _gradientAnimationSpeed;
}

void RetroFuturaGUI::FlavorPart::SetGradientRotationSpeed(const f32 value)
{
    _gradientRotationSpeed = value;
    StylingManager::markChanged();
}

const std::optional<f32>& RetroFuturaGUI::FlavorPart::GetGradientRotationSpeed() const
{
    return _gradientRotationSpeed;
}

void RetroFuturaGUI::FlavorPart::SetRasterPattern(const RasterPattern value)
{
    _rasterPattern = value;
    StylingManager::markChanged();
}

const std::optional<RetroFuturaGUI::RasterPattern>& RetroFuturaGUI::FlavorPart::GetRasterPattern() const
{
    return _rasterPattern;
}

void RetroFuturaGUI::FlavorPart::SetPrimaryRasterColor(const glm::vec4& value)
{
    _primaryRasterColor = value;
    StylingManager::markChanged();
}

const std::optional<glm::vec4>& RetroFuturaGUI::FlavorPart::GetPrimaryRasterColor() const
{
    return _primaryRasterColor;
}

void RetroFuturaGUI::FlavorPart::SetSecondaryRasterColor(const glm::vec4& value)
{
    _secondaryRasterColor = value;
    StylingManager::markChanged();
}

const std::optional<glm::vec4>& RetroFuturaGUI::FlavorPart::GetSecondaryRasterColor() const
{
    return _secondaryRasterColor;
}

void RetroFuturaGUI::FlavorPart::SetRasterSpacing(const f32 value)
{
    _rasterSpacing = value;
    StylingManager::markChanged();
}

const std::optional<f32>& RetroFuturaGUI::FlavorPart::GetRasterSpacing() const
{
    return _rasterSpacing;
}

void RetroFuturaGUI::FlavorPart::SetRasterDegree(const f32 value)
{
    _rasterDegree = value;
    StylingManager::markChanged();
}

const std::optional<f32>& RetroFuturaGUI::FlavorPart::GetRasterDegree() const
{
    return _rasterDegree;
}

void RetroFuturaGUI::FlavorPart::SetPrimaryRasterWidthTransfer(std::span<const f32> value)
{
    _primaryRasterWidthTransfer = std::vector<f32>(value.begin(), value.end());
    StylingManager::markChanged();
}

const std::optional<std::vector<f32>>& RetroFuturaGUI::FlavorPart::GetPrimaryRasterWidthTransfer() const
{
    return _primaryRasterWidthTransfer;
}

void RetroFuturaGUI::FlavorPart::SetDotTransparencyTransfer(const f32 value)
{
    _dotTransparencyTransfer = value;
    StylingManager::markChanged();
}

const std::optional<f32>& RetroFuturaGUI::FlavorPart::GetDotTransparencyTransfer() const
{
    return _dotTransparencyTransfer;
}

void RetroFuturaGUI::FlavorPart::SetRasterAnimationSpeed(const f32 value)
{
    _rasterAnimationSpeed = value;
    StylingManager::markChanged();
}

const std::optional<f32>& RetroFuturaGUI::FlavorPart::GetRasterAnimationSpeed() const
{
    return _rasterAnimationSpeed;
}

void RetroFuturaGUI::FlavorPart::SetWindowImageTextureID(const u32 value)
{
    _windowImageTextureID = value;
    StylingManager::markChanged();
}

const std::optional<u32>& RetroFuturaGUI::FlavorPart::GetWindowImageTextureID() const
{
    return _windowImageTextureID;
}

void RetroFuturaGUI::FlavorPart::SetGlassNoise(const GlassNoise& value)
{
    _glassNoise = value;
    StylingManager::markChanged();
}

const std::optional<RetroFuturaGUI::GlassNoise>& RetroFuturaGUI::FlavorPart::GetGlassNoise() const
{
    return _glassNoise;
}

void RetroFuturaGUI::FlavorPart::SetFogAlpha(const f32 value)
{
    _fogAlpha = value;
    StylingManager::markChanged();
}

const std::optional<f32>& RetroFuturaGUI::FlavorPart::GetFogAlpha() const
{
    return _fogAlpha;
}

void RetroFuturaGUI::FlavorPart::SetFogSpeed(const f32 value)
{
    _fogSpeed = value;
    StylingManager::markChanged();
}

const std::optional<f32>& RetroFuturaGUI::FlavorPart::GetFogSpeed() const
{
    return _fogSpeed;
}

void RetroFuturaGUI::FlavorPart::SetFogDensity(std::span<const f32> value)
{
    _fogDensity = std::vector<f32>(value.begin(), value.end());
    StylingManager::markChanged();
}

const std::optional<std::vector<f32>>& RetroFuturaGUI::FlavorPart::GetFogDensity() const
{
    return _fogDensity;
}

void RetroFuturaGUI::FlavorPart::SetFogClearing(const f32 value)
{
    _fogClearing = value;
    StylingManager::markChanged();
}

const std::optional<f32>& RetroFuturaGUI::FlavorPart::GetFogClearing() const
{
    return _fogClearing;
}

void RetroFuturaGUI::FlavorPart::SetFogNoise(const FogNoise& value)
{
    _fogNoise = value;
    StylingManager::markChanged();
}

const std::optional<RetroFuturaGUI::FogNoise>& RetroFuturaGUI::FlavorPart::GetFogNoise() const
{
    return _fogNoise;
}

void RetroFuturaGUI::FlavorPart::SetImagePadding(const f32 value)
{
    _imagePadding = value;
    StylingManager::markChanged();
}

const std::optional<f32>& RetroFuturaGUI::FlavorPart::GetImagePadding() const
{
    return _imagePadding;
}

void RetroFuturaGUI::FlavorPart::SetImageFit(const ImageFit value)
{
    _imageFit = value;
    StylingManager::markChanged();
}

const std::optional<RetroFuturaGUI::ImageFit>& RetroFuturaGUI::FlavorPart::GetImageFit() const
{
    return _imageFit;
}

void RetroFuturaGUI::FlavorPart::SetMaskTextureID(const u32 value)
{
    _maskTextureID = value;
    StylingManager::markChanged();
}

const std::optional<u32>& RetroFuturaGUI::FlavorPart::GetMaskTextureID() const
{
    return _maskTextureID;
}

void RetroFuturaGUI::FlavorPart::SetMaskMode(const MaskMode value)
{
    _maskMode = value;
    StylingManager::markChanged();
}

const std::optional<RetroFuturaGUI::MaskMode>& RetroFuturaGUI::FlavorPart::GetMaskMode() const
{
    return _maskMode;
}

void RetroFuturaGUI::FlavorPart::SetMaskFit(const ImageFit value)
{
    _maskFit = value;
    StylingManager::markChanged();
}

const std::optional<RetroFuturaGUI::ImageFit>& RetroFuturaGUI::FlavorPart::GetMaskFit() const
{
    return _maskFit;
}

void RetroFuturaGUI::FlavorPart::SetGaps(const BackgroundGap& value)
{
    _backgroundGaps = value;
    StylingManager::markChanged();
}

const std::optional<RetroFuturaGUI::BackgroundGap>& RetroFuturaGUI::FlavorPart::GetBackgroundGaps() const
{
    return _backgroundGaps;
}

void RetroFuturaGUI::FlavorPart::SetGaps(std::span<const BorderGap> value)
{
    _borderGaps = std::vector<BorderGap>(value.begin(), value.end());
    StylingManager::markChanged();
}

const std::optional<std::vector<RetroFuturaGUI::BorderGap>>& RetroFuturaGUI::FlavorPart::GetBorderGaps() const
{
    return _borderGaps;
}

void RetroFuturaGUI::FlavorPart::SetPixelateBlockSize(const f32 value)
{
    _pixelateBlockSize = value;
    StylingManager::markChanged();
}

const std::optional<f32>& RetroFuturaGUI::FlavorPart::GetPixelateBlockSize() const
{
    return _pixelateBlockSize;
}

void RetroFuturaGUI::FlavorPart::SetCrtEffect(const CrtEffect& value)
{
    _crtEffect = value;
    StylingManager::markChanged();
}

const std::optional<RetroFuturaGUI::CrtEffect>& RetroFuturaGUI::FlavorPart::GetCrtEffect() const
{
    return _crtEffect;
}

void RetroFuturaGUI::FlavorEffect::Show(const bool show)
{
    _shown = show;
    StylingManager::markChanged();
}

const std::optional<bool>& RetroFuturaGUI::FlavorEffect::GetShown() const
{
    return _shown;
}

void RetroFuturaGUI::FlavorEffect::SetColors(std::span<const glm::vec4> colors, const ColorState state)
{
    _colors[static_cast<uSize>(state)].assign(colors.begin(), colors.end());
    StylingManager::markChanged();
}

const std::vector<glm::vec4>& RetroFuturaGUI::FlavorEffect::GetColors(const ColorState state) const
{
    return _colors[static_cast<uSize>(state)];
}

void RetroFuturaGUI::FlavorEffect::Reset(const EffectSetting setting)
{
    switch(setting)
    {
        case EffectSetting::Shown:
            _shown.reset();
        break;
        case EffectSetting::Degree:
            _degree.reset();
        break;
        case EffectSetting::Distance:
            _distance.reset();
        break;
        case EffectSetting::Blur:
            _blur.reset();
        break;
        case EffectSetting::Scale:
            _scale.reset();
        break;
        case EffectSetting::Width:
            _width.reset();
        break;
        case EffectSetting::FillType:
            _fillType.reset();
        break;
        case EffectSetting::GradientDegree:
            _gradientDegree.reset();
        break;
        case EffectSetting::GradientOffset:
            _gradientOffset.reset();
        break;
        case EffectSetting::GradientAnimationSpeed:
            _gradientAnimationSpeed.reset();
        break;
        case EffectSetting::GradientRotationSpeed:
            _gradientRotationSpeed.reset();
        break;
    }

    StylingManager::markChanged();
}

void RetroFuturaGUI::FlavorEffect::SetDegree(const f32 value)
{
    _degree = value;
    StylingManager::markChanged();
}

const std::optional<f32>& RetroFuturaGUI::FlavorEffect::GetDegree() const
{
    return _degree;
}

void RetroFuturaGUI::FlavorEffect::SetDistance(const f32 value)
{
    _distance = value;
    StylingManager::markChanged();
}

const std::optional<f32>& RetroFuturaGUI::FlavorEffect::GetDistance() const
{
    return _distance;
}

void RetroFuturaGUI::FlavorEffect::SetBlur(const f32 value)
{
    _blur = value;
    StylingManager::markChanged();
}

const std::optional<f32>& RetroFuturaGUI::FlavorEffect::GetBlur() const
{
    return _blur;
}

void RetroFuturaGUI::FlavorEffect::SetScale(const f32 value)
{
    _scale = value;
    StylingManager::markChanged();
}

const std::optional<f32>& RetroFuturaGUI::FlavorEffect::GetScale() const
{
    return _scale;
}

void RetroFuturaGUI::FlavorEffect::SetWidth(const f32 value)
{
    _width = value;
    StylingManager::markChanged();
}

const std::optional<f32>& RetroFuturaGUI::FlavorEffect::GetWidth() const
{
    return _width;
}

void RetroFuturaGUI::FlavorEffect::SetFillType(const FillType value)
{
    _fillType = value;
    StylingManager::markChanged();
}

const std::optional<RetroFuturaGUI::FillType>& RetroFuturaGUI::FlavorEffect::GetFillType() const
{
    return _fillType;
}

void RetroFuturaGUI::FlavorEffect::SetGradientDegree(const f32 value)
{
    _gradientDegree = value;
    StylingManager::markChanged();
}

const std::optional<f32>& RetroFuturaGUI::FlavorEffect::GetGradientDegree() const
{
    return _gradientDegree;
}

void RetroFuturaGUI::FlavorEffect::SetGradientOffset(const f32 value)
{
    _gradientOffset = value;
    StylingManager::markChanged();
}

const std::optional<f32>& RetroFuturaGUI::FlavorEffect::GetGradientOffset() const
{
    return _gradientOffset;
}

void RetroFuturaGUI::FlavorEffect::SetGradientAnimationSpeed(const f32 value)
{
    _gradientAnimationSpeed = value;
    StylingManager::markChanged();
}

const std::optional<f32>& RetroFuturaGUI::FlavorEffect::GetGradientAnimationSpeed() const
{
    return _gradientAnimationSpeed;
}

void RetroFuturaGUI::FlavorEffect::SetGradientRotationSpeed(const f32 value)
{
    _gradientRotationSpeed = value;
    StylingManager::markChanged();
}

const std::optional<f32>& RetroFuturaGUI::FlavorEffect::GetGradientRotationSpeed() const
{
    return _gradientRotationSpeed;
}

RetroFuturaGUI::FlavorEffect& RetroFuturaGUI::FlavorPart::GetDropShadow()
{
    return _dropShadow;
}

const RetroFuturaGUI::FlavorEffect& RetroFuturaGUI::FlavorPart::GetDropShadow() const
{
    return _dropShadow;
}

RetroFuturaGUI::FlavorEffect& RetroFuturaGUI::FlavorPart::GetGlow()
{
    return _glow;
}

const RetroFuturaGUI::FlavorEffect& RetroFuturaGUI::FlavorPart::GetGlow() const
{
    return _glow;
}

RetroFuturaGUI::FlavorEffect& RetroFuturaGUI::FlavorPart::GetInnerGlow()
{
    return _innerGlow;
}

const RetroFuturaGUI::FlavorEffect& RetroFuturaGUI::FlavorPart::GetInnerGlow() const
{
    return _innerGlow;
}

RetroFuturaGUI::FlavorPart& RetroFuturaGUI::Flavor::GetBackground()
{
    return _background;
}

const RetroFuturaGUI::FlavorPart& RetroFuturaGUI::Flavor::GetBackground() const
{
    return _background;
}

RetroFuturaGUI::FlavorPart& RetroFuturaGUI::Flavor::GetBorder()
{
    return _border;
}

const RetroFuturaGUI::FlavorPart& RetroFuturaGUI::Flavor::GetBorder() const
{
    return _border;
}

RetroFuturaGUI::FlavorPart& RetroFuturaGUI::Flavor::GetText()
{
    return _text;
}

const RetroFuturaGUI::FlavorPart& RetroFuturaGUI::Flavor::GetText() const
{
    return _text;
}

const RetroFuturaGUI::FlavorPart& RetroFuturaGUI::Flavor::GetPart(const StylePart part) const
{
    switch(part)
    {
        case StylePart::Border:
            return _border;
        case StylePart::Text:
            return _text;
        default: //Background
            return _background;
    }
}
