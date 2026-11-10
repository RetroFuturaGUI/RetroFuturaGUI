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
        case StyleSetting::FontFamily:
            _fontFamily.reset();
        break;
        case StyleSetting::FontSize:
            _fontSize.reset();
        break;
        case StyleSetting::FontSlant:
            _fontSlant.reset();
        break;
        case StyleSetting::FontWeight:
            _fontWeight.reset();
        break;
        case StyleSetting::Alignment:
            _alignment.reset();
        break;
        case StyleSetting::Padding:
            _padding.reset();
        break;
        case StyleSetting::Flow:
            _flow.reset();
        break;
        case StyleSetting::ColorGlyphTint:
            _colorGlyphTint.reset();
        break;
        case StyleSetting::MaskSpan:
            _maskSpan.reset();
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

void RetroFuturaGUI::FlavorPart::SetFontSize(const f32 size, const FontSizeUnit sizeUnit)
{
    _fontSize = FontManager::ToPixels(size, sizeUnit);
    StylingManager::markChanged();
}

const std::optional<f32>& RetroFuturaGUI::FlavorPart::GetFontSize() const
{
    return _fontSize;
}

void RetroFuturaGUI::FlavorPart::SetFontFamily(std::string_view value)
{
    _fontFamily = std::string(value);
    StylingManager::markChanged();
}

const std::optional<std::string>& RetroFuturaGUI::FlavorPart::GetFontFamily() const
{
    return _fontFamily;
}

void RetroFuturaGUI::FlavorPart::SetFontSlant(const PlatformBridge::Fonts::Slant value)
{
    _fontSlant = value;
    StylingManager::markChanged();
}

const std::optional<PlatformBridge::Fonts::Slant>& RetroFuturaGUI::FlavorPart::GetFontSlant() const
{
    return _fontSlant;
}

void RetroFuturaGUI::FlavorPart::SetFontWeight(const PlatformBridge::Fonts::Weight value)
{
    _fontWeight = value;
    StylingManager::markChanged();
}

const std::optional<PlatformBridge::Fonts::Weight>& RetroFuturaGUI::FlavorPart::GetFontWeight() const
{
    return _fontWeight;
}

void RetroFuturaGUI::FlavorPart::SetAlignment(const TextAlignment value)
{
    _alignment = value;
    StylingManager::markChanged();
}

const std::optional<RetroFuturaGUI::TextAlignment>& RetroFuturaGUI::FlavorPart::GetAlignment() const
{
    return _alignment;
}

void RetroFuturaGUI::FlavorPart::SetPadding(const f32 value)
{
    _padding = value;
    StylingManager::markChanged();
}

const std::optional<f32>& RetroFuturaGUI::FlavorPart::GetPadding() const
{
    return _padding;
}

void RetroFuturaGUI::FlavorPart::SetFlow(const TextFlow value)
{
    _flow = value;
    StylingManager::markChanged();
}

const std::optional<RetroFuturaGUI::TextFlow>& RetroFuturaGUI::FlavorPart::GetFlow() const
{
    return _flow;
}

void RetroFuturaGUI::FlavorPart::SetColorGlyphTint(const f32 value)
{
    _colorGlyphTint = value;
    StylingManager::markChanged();
}

const std::optional<f32>& RetroFuturaGUI::FlavorPart::GetColorGlyphTint() const
{
    return _colorGlyphTint;
}

void RetroFuturaGUI::FlavorPart::SetMaskSpan(const MaskSpan value)
{
    _maskSpan = value;
    StylingManager::markChanged();
}

const std::optional<RetroFuturaGUI::MaskSpan>& RetroFuturaGUI::FlavorPart::GetMaskSpan() const
{
    return _maskSpan;
}

RetroFuturaGUI::FlavorEffect& RetroFuturaGUI::FlavorPart::GetOutline()
{
    return _outline;
}

const RetroFuturaGUI::FlavorEffect& RetroFuturaGUI::FlavorPart::GetOutline() const
{
    return _outline;
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

const RetroFuturaGUI::FlavorPart* RetroFuturaGUI::Flavor::FindPart(const StylePart part) const
{
    switch(part)
    {
        case StylePart::Background:
            return &_background;
        case StylePart::Border:
            return &_border;
        case StylePart::Text:
            return &_text;
        case StylePart::IndicatorBackground:
            return _indicator ? &_indicator->GetBackground() : nullptr;
        case StylePart::IndicatorBorder:
            return _indicator ? &_indicator->GetBorder() : nullptr;
        case StylePart::Graph:
            return _graph.get();
        case StylePart::ListBackground:
            return _list ? &_list->GetBackground() : nullptr;
        case StylePart::ListBorder:
            return _list ? &_list->GetBorder() : nullptr;
        case StylePart::ListHighlight:
            return _list ? &_list->GetHighlight() : nullptr;
        case StylePart::ItemStyleBackground:
            return _itemStyle ? &_itemStyle->GetBackground() : nullptr;
        case StylePart::ItemStyleBorder:
            return _itemStyle ? &_itemStyle->GetBorder() : nullptr;
        case StylePart::SeparatorLine:
            return _separatorLine.get();
        case StylePart::FieldStyleBackground:
            return _fieldStyle ? &_fieldStyle->GetBackground() : nullptr;
        case StylePart::FieldStyleBorder:
            return _fieldStyle ? &_fieldStyle->GetBorder() : nullptr;
        case StylePart::CloseButtonBackground:
            return _closeButton ? &_closeButton->GetBackground() : nullptr;
        case StylePart::SelectionBandBackground:
            return _selectionBand ? &_selectionBand->GetBackground() : nullptr;
        case StylePart::SelectionBandBorder:
            return _selectionBand ? &_selectionBand->GetBorder() : nullptr;
        case StylePart::HoverTextBackground:
            return _hoverText ? &_hoverText->GetBackground() : nullptr;
        case StylePart::HoverTextText:
            return _hoverText ? &_hoverText->GetText() : nullptr;
        case StylePart::Placeholder:
            return _placeholder.get();
        case StylePart::Preview:
            return _preview.get();
        case StylePart::Checkmark:
            return _checkmark.get();
        case StylePart::Arrow:
            return _arrow.get();
        case StylePart::Caret:
            return _caret.get();
        case StylePart::SelectedArea:
            return _selectedArea.get();
        case StylePart::ItemText:
            return _itemText.get();
        case StylePart::CloseButtonIcon:
            return _closeButton ? &_closeButton->GetIcon() : nullptr;
        case StylePart::CellStyleBackground:
            return _cellStyle ? &_cellStyle->_background : nullptr;
        case StylePart::CellStyleInnerBorder:
            return _cellStyle ? &_cellStyle->_innerBorder : nullptr;
        case StylePart::CellStyleText:
            return _cellStyle ? &_cellStyle->_text : nullptr;
        case StylePart::CellStyleIcon:
            return _cellStyle ? &_cellStyle->_icon : nullptr;
        case StylePart::CellStyleDropTarget:
            return _cellStyle ? &_cellStyle->_dropTarget : nullptr;
        case StylePart::HeaderStyleBackground:
            return _headerStyle ? &_headerStyle->_background : nullptr;
        case StylePart::HeaderStyleInnerBorder:
            return _headerStyle ? &_headerStyle->_innerBorder : nullptr;
        case StylePart::HeaderStyleOuterBorder:
            return _headerStyle ? &_headerStyle->_outerBorder : nullptr;
        case StylePart::HeaderStyleText:
            return _headerStyle ? &_headerStyle->_text : nullptr;
        case StylePart::CellCheckBoxBackground:
            return _cellCheckBox ? &_cellCheckBox->GetBackground() : nullptr;
        case StylePart::CellCheckBoxBorder:
            return _cellCheckBox ? &_cellCheckBox->GetBorder() : nullptr;
        case StylePart::CellCheckBoxCheckmark:
            return _cellCheckBox ? &_cellCheckBox->GetCheckmark() : nullptr;
        default:
            return nullptr;
    }
}

RetroFuturaGUI::FlavorCellStyle& RetroFuturaGUI::Flavor::GetCellStyle()
{
    if(!_cellStyle)
        _cellStyle = std::make_unique<FlavorCellStyle>();

    return *_cellStyle;
}

const RetroFuturaGUI::FlavorCellStyle* RetroFuturaGUI::Flavor::FindCellStyle() const
{
    return _cellStyle.get();
}

RetroFuturaGUI::FlavorHeaderStyle& RetroFuturaGUI::Flavor::GetHeaderStyle()
{
    if(!_headerStyle)
        _headerStyle = std::make_unique<FlavorHeaderStyle>();

    return *_headerStyle;
}

const RetroFuturaGUI::FlavorHeaderStyle* RetroFuturaGUI::Flavor::FindHeaderStyle() const
{
    return _headerStyle.get();
}

RetroFuturaGUI::FlavorCellCheckBox& RetroFuturaGUI::Flavor::GetCellCheckBox()
{
    if(!_cellCheckBox)
        _cellCheckBox = std::make_unique<FlavorCellCheckBox>();

    return *_cellCheckBox;
}

const RetroFuturaGUI::FlavorCellCheckBox* RetroFuturaGUI::Flavor::FindCellCheckBox() const
{
    return _cellCheckBox.get();
}

RetroFuturaGUI::FlavorPart& RetroFuturaGUI::Flavor::GetCheckmark()
{
    if(!_checkmark)
        _checkmark = std::make_unique<FlavorPart>();

    return *_checkmark;
}

const RetroFuturaGUI::FlavorPart* RetroFuturaGUI::Flavor::FindCheckmark() const
{
    return _checkmark.get();
}

RetroFuturaGUI::FlavorPart& RetroFuturaGUI::Flavor::GetArrow()
{
    if(!_arrow)
        _arrow = std::make_unique<FlavorPart>();

    return *_arrow;
}

const RetroFuturaGUI::FlavorPart* RetroFuturaGUI::Flavor::FindArrow() const
{
    return _arrow.get();
}

RetroFuturaGUI::FlavorPart& RetroFuturaGUI::Flavor::GetCaret()
{
    if(!_caret)
        _caret = std::make_unique<FlavorPart>();

    return *_caret;
}

const RetroFuturaGUI::FlavorPart* RetroFuturaGUI::Flavor::FindCaret() const
{
    return _caret.get();
}

RetroFuturaGUI::FlavorPart& RetroFuturaGUI::Flavor::GetSelectedArea()
{
    if(!_selectedArea)
        _selectedArea = std::make_unique<FlavorPart>();

    return *_selectedArea;
}

const RetroFuturaGUI::FlavorPart* RetroFuturaGUI::Flavor::FindSelectedArea() const
{
    return _selectedArea.get();
}

RetroFuturaGUI::FlavorPart& RetroFuturaGUI::Flavor::GetItemText()
{
    if(!_itemText)
        _itemText = std::make_unique<FlavorPart>();

    return *_itemText;
}

const RetroFuturaGUI::FlavorPart* RetroFuturaGUI::Flavor::FindItemText() const
{
    return _itemText.get();
}

RetroFuturaGUI::FlavorBox& RetroFuturaGUI::Flavor::GetIndicator()
{
    if(!_indicator)
        _indicator = std::make_unique<FlavorBox>();

    return *_indicator;
}

const RetroFuturaGUI::FlavorBox* RetroFuturaGUI::Flavor::FindIndicator() const
{
    return _indicator.get();
}

RetroFuturaGUI::FlavorPart& RetroFuturaGUI::Flavor::GetGraph()
{
    if(!_graph)
        _graph = std::make_unique<FlavorPart>();

    return *_graph;
}

const RetroFuturaGUI::FlavorPart* RetroFuturaGUI::Flavor::FindGraph() const
{
    return _graph.get();
}

RetroFuturaGUI::FlavorList& RetroFuturaGUI::Flavor::GetList()
{
    if(!_list)
        _list = std::make_unique<FlavorList>();

    return *_list;
}

const RetroFuturaGUI::FlavorList* RetroFuturaGUI::Flavor::FindList() const
{
    return _list.get();
}

RetroFuturaGUI::FlavorBox& RetroFuturaGUI::Flavor::GetItemStyle()
{
    if(!_itemStyle)
        _itemStyle = std::make_unique<FlavorBox>();

    return *_itemStyle;
}

const RetroFuturaGUI::FlavorBox* RetroFuturaGUI::Flavor::FindItemStyle() const
{
    return _itemStyle.get();
}

RetroFuturaGUI::FlavorPart& RetroFuturaGUI::Flavor::GetSeparatorLine()
{
    if(!_separatorLine)
        _separatorLine = std::make_unique<FlavorPart>();

    return *_separatorLine;
}

const RetroFuturaGUI::FlavorPart* RetroFuturaGUI::Flavor::FindSeparatorLine() const
{
    return _separatorLine.get();
}

RetroFuturaGUI::FlavorBox& RetroFuturaGUI::Flavor::GetFieldStyle()
{
    if(!_fieldStyle)
        _fieldStyle = std::make_unique<FlavorBox>();

    return *_fieldStyle;
}

const RetroFuturaGUI::FlavorBox* RetroFuturaGUI::Flavor::FindFieldStyle() const
{
    return _fieldStyle.get();
}

RetroFuturaGUI::FlavorCloseButton& RetroFuturaGUI::Flavor::GetCloseButton()
{
    if(!_closeButton)
        _closeButton = std::make_unique<FlavorCloseButton>();

    return *_closeButton;
}

const RetroFuturaGUI::FlavorCloseButton* RetroFuturaGUI::Flavor::FindCloseButton() const
{
    return _closeButton.get();
}

RetroFuturaGUI::FlavorBox& RetroFuturaGUI::Flavor::GetSelectionBand()
{
    if(!_selectionBand)
        _selectionBand = std::make_unique<FlavorBox>();

    return *_selectionBand;
}

const RetroFuturaGUI::FlavorBox* RetroFuturaGUI::Flavor::FindSelectionBand() const
{
    return _selectionBand.get();
}

RetroFuturaGUI::FlavorHoverText& RetroFuturaGUI::Flavor::GetHoverText()
{
    if(!_hoverText)
        _hoverText = std::make_unique<FlavorHoverText>();

    return *_hoverText;
}

const RetroFuturaGUI::FlavorHoverText* RetroFuturaGUI::Flavor::FindHoverText() const
{
    return _hoverText.get();
}

RetroFuturaGUI::FlavorPart& RetroFuturaGUI::Flavor::GetPlaceholder()
{
    if(!_placeholder)
        _placeholder = std::make_unique<FlavorPart>();

    return *_placeholder;
}

const RetroFuturaGUI::FlavorPart* RetroFuturaGUI::Flavor::FindPlaceholder() const
{
    return _placeholder.get();
}

RetroFuturaGUI::FlavorPart& RetroFuturaGUI::Flavor::GetPreview()
{
    if(!_preview)
        _preview = std::make_unique<FlavorPart>();

    return *_preview;
}

const RetroFuturaGUI::FlavorPart* RetroFuturaGUI::Flavor::FindPreview() const
{
    return _preview.get();
}

void RetroFuturaGUI::Flavor::SetTransition(const TransitionProperty properties, const Transition& transition)
{
    Transitions::Override(_transitions, properties, transition);
    StylingManager::markChanged();
}

void RetroFuturaGUI::Flavor::ResetTransition(const TransitionProperty properties)
{
    Transitions::Reset(_transitions, properties);
    StylingManager::markChanged();
}

const RetroFuturaGUI::TransitionSettings& RetroFuturaGUI::Flavor::GetTransitions() const
{
    return _transitions;
}

void RetroFuturaGUI::Flavor::SetBorderGap(const glm::vec4& gap)
{
    _borderGap = gap;
    StylingManager::markChanged();
}

void RetroFuturaGUI::Flavor::ResetBorderGap()
{
    _borderGap.reset();
    StylingManager::markChanged();
}

const std::optional<glm::vec4>& RetroFuturaGUI::Flavor::GetBorderGap() const
{
    return _borderGap;
}

RetroFuturaGUI::FlavorPart& RetroFuturaGUI::FlavorBox::GetBackground()
{
    return _background;
}

const RetroFuturaGUI::FlavorPart& RetroFuturaGUI::FlavorBox::GetBackground() const
{
    return _background;
}

RetroFuturaGUI::FlavorPart& RetroFuturaGUI::FlavorBox::GetBorder()
{
    return _border;
}

const RetroFuturaGUI::FlavorPart& RetroFuturaGUI::FlavorBox::GetBorder() const
{
    return _border;
}

RetroFuturaGUI::FlavorPart& RetroFuturaGUI::FlavorList::GetBackground()
{
    return _background;
}

const RetroFuturaGUI::FlavorPart& RetroFuturaGUI::FlavorList::GetBackground() const
{
    return _background;
}

RetroFuturaGUI::FlavorPart& RetroFuturaGUI::FlavorList::GetBorder()
{
    return _border;
}

const RetroFuturaGUI::FlavorPart& RetroFuturaGUI::FlavorList::GetBorder() const
{
    return _border;
}

RetroFuturaGUI::FlavorPart& RetroFuturaGUI::FlavorList::GetHighlight()
{
    return _highlight;
}

const RetroFuturaGUI::FlavorPart& RetroFuturaGUI::FlavorList::GetHighlight() const
{
    return _highlight;
}

RetroFuturaGUI::FlavorPart& RetroFuturaGUI::FlavorCloseButton::GetBackground()
{
    return _background;
}

const RetroFuturaGUI::FlavorPart& RetroFuturaGUI::FlavorCloseButton::GetBackground() const
{
    return _background;
}

RetroFuturaGUI::FlavorPart& RetroFuturaGUI::FlavorHoverText::GetBackground()
{
    return _background;
}

const RetroFuturaGUI::FlavorPart& RetroFuturaGUI::FlavorHoverText::GetBackground() const
{
    return _background;
}

RetroFuturaGUI::FlavorPart& RetroFuturaGUI::FlavorHoverText::GetText()
{
    return _text;
}

const RetroFuturaGUI::FlavorPart& RetroFuturaGUI::FlavorHoverText::GetText() const
{
    return _text;
}

RetroFuturaGUI::FlavorPart& RetroFuturaGUI::FlavorCloseButton::GetIcon()
{
    return _icon;
}

const RetroFuturaGUI::FlavorPart& RetroFuturaGUI::FlavorCloseButton::GetIcon() const
{
    return _icon;
}

void RetroFuturaGUI::FlavorTrackStyle::SetBackgroundColors(std::span<const glm::vec4> colors)
{
    _background.assign(colors.begin(), colors.end());
    StylingManager::markChanged();
}

const std::vector<glm::vec4>& RetroFuturaGUI::FlavorTrackStyle::GetBackgroundColors() const
{
    return _background;
}

void RetroFuturaGUI::FlavorTrackStyle::SetInnerBorderColors(std::span<const glm::vec4> colors)
{
    _innerBorder.assign(colors.begin(), colors.end());
    StylingManager::markChanged();
}

const std::vector<glm::vec4>& RetroFuturaGUI::FlavorTrackStyle::GetInnerBorderColors() const
{
    return _innerBorder;
}

void RetroFuturaGUI::FlavorTrackStyle::SetTextColors(std::span<const glm::vec4> colors)
{
    _text.assign(colors.begin(), colors.end());
    StylingManager::markChanged();
}

const std::vector<glm::vec4>& RetroFuturaGUI::FlavorTrackStyle::GetTextColors() const
{
    return _text;
}

void RetroFuturaGUI::FlavorTrackStyle::SetIconColors(std::span<const glm::vec4> colors)
{
    _icon.assign(colors.begin(), colors.end());
    StylingManager::markChanged();
}

const std::vector<glm::vec4>& RetroFuturaGUI::FlavorTrackStyle::GetIconColors() const
{
    return _icon;
}

void RetroFuturaGUI::FlavorCellStyle::SetBackgroundColors(std::span<const glm::vec4> colors, const ColorState state)
{
    if(!isShared(state))
        return;

    _background.SetColors(colors, state);
}

const std::vector<glm::vec4>& RetroFuturaGUI::FlavorCellStyle::GetBackgroundColors(const ColorState state) const
{
    return _background.GetColors(state);
}

void RetroFuturaGUI::FlavorCellStyle::SetInnerBorderColors(std::span<const glm::vec4> colors, const ColorState state)
{
    if(!isShared(state))
        return;

    _innerBorder.SetColors(colors, state);
}

const std::vector<glm::vec4>& RetroFuturaGUI::FlavorCellStyle::GetInnerBorderColors(const ColorState state) const
{
    return _innerBorder.GetColors(state);
}

void RetroFuturaGUI::FlavorCellStyle::SetTextColors(std::span<const glm::vec4> colors, const ColorState state)
{
    if(!isShared(state))
        return;

    _text.SetColors(colors, state);
}

const std::vector<glm::vec4>& RetroFuturaGUI::FlavorCellStyle::GetTextColors(const ColorState state) const
{
    return _text.GetColors(state);
}

void RetroFuturaGUI::FlavorCellStyle::SetIconColors(std::span<const glm::vec4> colors, const ColorState state)
{
    if(!isShared(state))
        return;

    _icon.SetColors(colors, state);
}

const std::vector<glm::vec4>& RetroFuturaGUI::FlavorCellStyle::GetIconColors(const ColorState state) const
{
    return _icon.GetColors(state);
}

void RetroFuturaGUI::FlavorCellStyle::SetDropTargetColors(std::span<const glm::vec4> colors)
{
    _dropTarget.SetColors(colors, ColorState::Enabled);
}

const std::vector<glm::vec4>& RetroFuturaGUI::FlavorCellStyle::GetDropTargetColors() const
{
    return _dropTarget.GetColors(ColorState::Enabled);
}

void RetroFuturaGUI::FlavorCellStyle::SetInnerBorderWidth(const f32 width)
{
    _innerBorder.SetWidth(width);
}

const std::optional<f32>& RetroFuturaGUI::FlavorCellStyle::GetInnerBorderWidth() const
{
    return _innerBorder.GetWidth();
}

void RetroFuturaGUI::FlavorCellStyle::SetFontFamily(std::string_view fontFamily)
{
    _textSettings._FontFamily = std::string(fontFamily);
    StylingManager::markChanged();
}

void RetroFuturaGUI::FlavorCellStyle::SetFontSize(const f32 fontSize, const FontSizeUnit sizeUnit)
{
    _textSettings._FontSize = FontManager::ToPixels(fontSize, sizeUnit);
    StylingManager::markChanged();
}

void RetroFuturaGUI::FlavorCellStyle::SetFontSlant(const PlatformBridge::Fonts::Slant slant)
{
    _textSettings._Slant = slant;
    StylingManager::markChanged();
}

void RetroFuturaGUI::FlavorCellStyle::SetFontWeight(const PlatformBridge::Fonts::Weight fontWeight)
{
    _textSettings._Weight = fontWeight;
    StylingManager::markChanged();
}

void RetroFuturaGUI::FlavorCellStyle::SetTextAlignment(const TextAlignment alignment)
{
    _textSettings._Alignment = alignment;
    StylingManager::markChanged();
}

void RetroFuturaGUI::FlavorCellStyle::SetTextPadding(const f32 padding)
{
    _textSettings._Padding = padding;
    StylingManager::markChanged();
}

const RetroFuturaGUI::TableTextLayer& RetroFuturaGUI::FlavorCellStyle::GetTextSettings() const
{
    return _textSettings;
}

bool RetroFuturaGUI::FlavorCellStyle::isShared(const ColorState state)
{
    return ColorState::Enabled != state;
}

void RetroFuturaGUI::FlavorHeaderStyle::SetBackgroundColors(std::span<const glm::vec4> colors, const ColorState state)
{
    _background.SetColors(colors, headerState(state));
}

const std::vector<glm::vec4>& RetroFuturaGUI::FlavorHeaderStyle::GetBackgroundColors(const ColorState state) const
{
    return _background.GetColors(headerState(state));
}

void RetroFuturaGUI::FlavorHeaderStyle::SetInnerBorderColors(std::span<const glm::vec4> colors, const ColorState state)
{
    _innerBorder.SetColors(colors, headerState(state));
}

const std::vector<glm::vec4>& RetroFuturaGUI::FlavorHeaderStyle::GetInnerBorderColors(const ColorState state) const
{
    return _innerBorder.GetColors(headerState(state));
}

void RetroFuturaGUI::FlavorHeaderStyle::SetOuterBorderColors(std::span<const glm::vec4> colors, const ColorState state)
{
    _outerBorder.SetColors(colors, headerState(state));
}

const std::vector<glm::vec4>& RetroFuturaGUI::FlavorHeaderStyle::GetOuterBorderColors(const ColorState state) const
{
    return _outerBorder.GetColors(headerState(state));
}

void RetroFuturaGUI::FlavorHeaderStyle::SetTextColors(std::span<const glm::vec4> colors, const ColorState state)
{
    _text.SetColors(colors, headerState(state));
}

const std::vector<glm::vec4>& RetroFuturaGUI::FlavorHeaderStyle::GetTextColors(const ColorState state) const
{
    return _text.GetColors(headerState(state));
}

void RetroFuturaGUI::FlavorHeaderStyle::SetInnerBorderWidth(const f32 width)
{
    _innerBorder.SetWidth(width);
}

const std::optional<f32>& RetroFuturaGUI::FlavorHeaderStyle::GetInnerBorderWidth() const
{
    return _innerBorder.GetWidth();
}

void RetroFuturaGUI::FlavorHeaderStyle::SetOuterBorderWidth(const f32 width)
{
    _outerBorder.SetWidth(width);
}

const std::optional<f32>& RetroFuturaGUI::FlavorHeaderStyle::GetOuterBorderWidth() const
{
    return _outerBorder.GetWidth();
}

void RetroFuturaGUI::FlavorHeaderStyle::SetFontFamily(std::string_view fontFamily)
{
    _textSettings._FontFamily = std::string(fontFamily);
    StylingManager::markChanged();
}

void RetroFuturaGUI::FlavorHeaderStyle::SetFontSize(const f32 fontSize, const FontSizeUnit sizeUnit)
{
    _textSettings._FontSize = FontManager::ToPixels(fontSize, sizeUnit);
    StylingManager::markChanged();
}

void RetroFuturaGUI::FlavorHeaderStyle::SetFontSlant(const PlatformBridge::Fonts::Slant slant)
{
    _textSettings._Slant = slant;
    StylingManager::markChanged();
}

void RetroFuturaGUI::FlavorHeaderStyle::SetFontWeight(const PlatformBridge::Fonts::Weight fontWeight)
{
    _textSettings._Weight = fontWeight;
    StylingManager::markChanged();
}

void RetroFuturaGUI::FlavorHeaderStyle::SetTextAlignment(const TextAlignment alignment)
{
    _textSettings._Alignment = alignment;
    StylingManager::markChanged();
}

void RetroFuturaGUI::FlavorHeaderStyle::SetTextPadding(const f32 padding)
{
    _textSettings._Padding = padding;
    StylingManager::markChanged();
}

const RetroFuturaGUI::TableTextLayer& RetroFuturaGUI::FlavorHeaderStyle::GetTextSettings() const
{
    return _textSettings;
}

RetroFuturaGUI::ColorState RetroFuturaGUI::FlavorHeaderStyle::headerState(const ColorState state)
{
    return ColorState::Disabled == state ? ColorState::Disabled : ColorState::Enabled;
}

RetroFuturaGUI::FlavorPart& RetroFuturaGUI::FlavorCellCheckBox::GetBackground()
{
    return _background;
}

const RetroFuturaGUI::FlavorPart& RetroFuturaGUI::FlavorCellCheckBox::GetBackground() const
{
    return _background;
}

RetroFuturaGUI::FlavorPart& RetroFuturaGUI::FlavorCellCheckBox::GetBorder()
{
    return _border;
}

const RetroFuturaGUI::FlavorPart& RetroFuturaGUI::FlavorCellCheckBox::GetBorder() const
{
    return _border;
}

RetroFuturaGUI::FlavorPart& RetroFuturaGUI::FlavorCellCheckBox::GetCheckmark()
{
    return _checkmark;
}

const RetroFuturaGUI::FlavorPart& RetroFuturaGUI::FlavorCellCheckBox::GetCheckmark() const
{
    return _checkmark;
}

RetroFuturaGUI::FlavorTrackStyle& RetroFuturaGUI::Flavor::GetTrackStyle(const uSize nthIndex)
{
    if(_trackStyles.size() <= nthIndex)
        _trackStyles.resize(nthIndex + 1);

    std::unique_ptr<FlavorTrackStyle>& trackStyle { _trackStyles[nthIndex] };

    if(!trackStyle)
        trackStyle = std::make_unique<FlavorTrackStyle>();

    return *trackStyle;
}

const RetroFuturaGUI::FlavorTrackStyle* RetroFuturaGUI::Flavor::FindTrackStyle(const uSize nthIndex) const
{
    if(_trackStyles.size() <= nthIndex)
        return nullptr;

    return _trackStyles[nthIndex].get();
}
