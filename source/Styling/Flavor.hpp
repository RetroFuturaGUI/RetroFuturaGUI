#pragma once
#include "IncludeHelper.hpp"
#include "IRectangle.hpp"
#include "Plane.hpp"
#include "Frame.hpp"
#include "FillStyle.hpp"
#include "Noise.hpp"
#include <array>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace RetroFuturaGUI
{
    /// @brief The part of a widget a flavor's part style applies to.
    RFG_EXPORT enum class StylePart : u32
    {
        Background,
        Border,
        Text
    };

    /// @brief One setting a part or a flavor can leave unset (ResetSetting, FlavorPart::Reset), so it comes from further
    ///        down. The widget's flavors, .default, then the built-in look. Colors are left unset by setting none
    RFG_EXPORT enum class StyleSetting : u32
    {
        CornerRadii,
        CutCorners,
        Width,
        Image,
        FillType,
        GradientDegree,
        GradientOffset,
        GradientAnimationSpeed,
        GradientRotationSpeed,
        RasterPattern,
        PrimaryRasterColor,
        SecondaryRasterColor,
        RasterSpacing,
        RasterDegree,
        PrimaryRasterWidthTransfer,
        DotTransparencyTransfer,
        RasterAnimationSpeed,
        WindowImageTextureID,
        GlassNoise,
        FogAlpha,
        FogSpeed,
        FogDensity,
        FogClearing,
        FogNoise,
        ImagePadding,
        ImageFit,
        MaskTextureID,
        MaskMode,
        MaskFit,
        Gaps,
        PixelateBlockSize,
        CrtEffect
    };

    /// @brief A setting of an effect (drop shadow, glow, inner glow) that a part's effect or a flavor can leave unset.
    ///        Colors are left unset by setting none
    RFG_EXPORT enum class EffectSetting : u32
    {
        Shown,
        Degree,
        Distance,
        Blur,
        Scale,
        Width,
        FillType,
        GradientDegree,
        GradientOffset,
        GradientAnimationSpeed,
        GradientRotationSpeed
    };

    /// @brief What a flavor sets for one effect of its widgets' parts: whether it shows, its colors per state, its numbers
    ///        and its fill. Unset values come from the next flavor, .default, then the effect's own default; what the widget
    ///        sets on its effect wins. A number the effect doesn't have (a glow's degree) is ignored there.
    ///        Reached by StylingManager::GetFlavor("primary").GetBackground().GetDropShadow()
    RFG_EXPORT class FlavorEffect final
    {
    public:
        FlavorEffect() = default;
        FlavorEffect(const FlavorEffect&) = delete;
        FlavorEffect(FlavorEffect&&) = delete;
        auto operator =(const FlavorEffect&) = delete;
        auto operator =(FlavorEffect&&) = delete;

        /// @brief Sets whether the effect shows
        void Show(const bool show);
        const std::optional<bool>& GetShown() const;

        /// @brief Sets the colors for a color state. No colors leave the state unset
        void SetColors(std::span<const glm::vec4> colors, const ColorState state);
        const std::vector<glm::vec4>& GetColors(const ColorState state) const;

        /// @brief Leaves a setting unset, so it comes from further down
        void Reset(const EffectSetting setting);

        /// @brief Sets the angle the shadow falls or the inner glow is set off at, in degrees
        void SetDegree(const f32 value);
        const std::optional<f32>& GetDegree() const;

        /// @brief Sets how far the shadow falls or the inner glow is set off, in pixels
        void SetDistance(const f32 value);
        const std::optional<f32>& GetDistance() const;

        /// @brief Sets how wide the shadow's edges fade, in pixels
        void SetBlur(const f32 value);
        const std::optional<f32>& GetBlur() const;

        /// @brief Sets how much bigger than what casts it the shadow is
        void SetScale(const f32 value);
        const std::optional<f32>& GetScale() const;

        /// @brief Sets how far a glow or an inner glow reaches, in pixels
        void SetWidth(const f32 value);
        const std::optional<f32>& GetWidth() const;

        /// @brief Sets the fill type
        void SetFillType(const FillType value);
        const std::optional<FillType>& GetFillType() const;

        /// @brief Sets the gradient's angle in degrees
        void SetGradientDegree(const f32 value);
        const std::optional<f32>& GetGradientDegree() const;

        /// @brief Sets where the gradient's colors start, 0 to 1
        void SetGradientOffset(const f32 value);
        const std::optional<f32>& GetGradientOffset() const;

        /// @brief Sets how fast the gradient's colors move on, in runs per second
        void SetGradientAnimationSpeed(const f32 value);
        const std::optional<f32>& GetGradientAnimationSpeed() const;

        /// @brief Sets how fast the gradient turns, in degrees per second
        void SetGradientRotationSpeed(const f32 value);
        const std::optional<f32>& GetGradientRotationSpeed() const;

    private:
        std::array<std::vector<glm::vec4>, 5> _colors {}; //per ColorState, in its order
        std::optional<bool> _shown {};
        std::optional<f32> _degree {};
        std::optional<f32> _distance {};
        std::optional<f32> _blur {};
        std::optional<f32> _scale {};
        std::optional<f32> _width {};
        std::optional<FillType> _fillType {};
        std::optional<f32> _gradientDegree {};
        std::optional<f32> _gradientOffset {};
        std::optional<f32> _gradientAnimationSpeed {};
        std::optional<f32> _gradientRotationSpeed {};
    };

    /// @brief What a flavor sets for one part of the widgets that have it: colors per state, the corner shape, a border's
    ///        width, an image, and every shader setting (fill, raster, glass, fog, masks, gaps, pixelation, CRT). Whatever it
    ///        leaves unset comes from the widget's next flavor, then .default, then the widget's built-in look; whatever the
    ///        widget sets itself wins over all of them. A change shows on every widget with the flavor from its next draw.
    ///        A setting a part doesn't have, such as fog on a border, is ignored there.
    ///        Reached by StylingManager::GetFlavor("primary").GetBackground()
    RFG_EXPORT class FlavorPart final
    {
    public:
        FlavorPart() = default;
        FlavorPart(const FlavorPart&) = delete;
        FlavorPart(FlavorPart&&) = delete;
        auto operator =(const FlavorPart&) = delete;
        auto operator =(FlavorPart&&) = delete;

        /// @brief Sets the colors for a color state. One for a solid fill, the stops for a gradient. No colors leave the
        ///        state unset, so it comes from further down
        void SetColors(std::span<const glm::vec4> colors, const ColorState state);

        /// @brief Returns the colors set for a color state, empty when unset
        const std::vector<glm::vec4>& GetColors(const ColorState state) const;

        /// @brief Leaves a setting unset, so it comes from further down
        void Reset(const StyleSetting setting);

        /// @brief Sets the corner radii, clockwise from the top left
        void SetCornerRadii(const glm::vec4& radii);
        const std::optional<glm::vec4>& GetCornerRadii() const;

        /// @brief Sets which corners are cut off straight instead of rounded
        void SetCutCorners(const CutCorners corners);
        const std::optional<CutCorners>& GetCutCorners() const;

        /// @brief Sets a border's width in pixels
        void SetWidth(const f32 width);
        const std::optional<f32>& GetWidth() const;

        /// @brief Returns what the flavor sets for the part's drop shadow, glow and inner glow
        FlavorEffect& GetDropShadow();
        const FlavorEffect& GetDropShadow() const;
        FlavorEffect& GetGlow();
        const FlavorEffect& GetGlow() const;
        FlavorEffect& GetInnerGlow();
        const FlavorEffect& GetInnerGlow() const;

        /// @brief Sets a background's image by its file path, a picture or an SVG; an empty path shows none
        void SetImage(std::string_view path);
        const std::optional<std::string>& GetImage() const;

        /// @brief Sets the fill type (solid, linear, radial or conic shift)
        void SetFillType(const FillType value);
        const std::optional<FillType>& GetFillType() const;

        /// @brief Sets the gradient's angle in degrees
        void SetGradientDegree(const f32 value);
        const std::optional<f32>& GetGradientDegree() const;

        /// @brief Sets where the gradient's colors start, 0 to 1
        void SetGradientOffset(const f32 value);
        const std::optional<f32>& GetGradientOffset() const;

        /// @brief Sets how fast the gradient's colors move on, in runs through all colors per second
        void SetGradientAnimationSpeed(const f32 value);
        const std::optional<f32>& GetGradientAnimationSpeed() const;

        /// @brief Sets how fast the gradient turns, in degrees per second
        void SetGradientRotationSpeed(const f32 value);
        const std::optional<f32>& GetGradientRotationSpeed() const;

        /// @brief Sets which pattern the raster draws
        void SetRasterPattern(const RasterPattern value);
        const std::optional<RasterPattern>& GetRasterPattern() const;

        /// @brief Sets the raster's primary color
        void SetPrimaryRasterColor(const glm::vec4& value);
        const std::optional<glm::vec4>& GetPrimaryRasterColor() const;

        /// @brief Sets the raster's secondary color
        void SetSecondaryRasterColor(const glm::vec4& value);
        const std::optional<glm::vec4>& GetSecondaryRasterColor() const;

        /// @brief Sets the gap between the widest raster elements, in pixels
        void SetRasterSpacing(const f32 value);
        const std::optional<f32>& GetRasterSpacing() const;

        /// @brief Sets the raster's angle in degrees
        void SetRasterDegree(const f32 value);
        const std::optional<f32>& GetRasterDegree() const;

        /// @brief Sets the raster's widths across the part, in pixels. None turn the raster off
        void SetPrimaryRasterWidthTransfer(std::span<const f32> value);
        const std::optional<std::vector<f32>>& GetPrimaryRasterWidthTransfer() const;

        /// @brief Sets how far a dot's opacity reaches before it fades, 0 to 1
        void SetDotTransparencyTransfer(const f32 value);
        const std::optional<f32>& GetDotTransparencyTransfer() const;

        /// @brief Sets how fast the raster moves, in pixels per second
        void SetRasterAnimationSpeed(const f32 value);
        const std::optional<f32>& GetRasterAnimationSpeed() const;

        /// @brief Sets the backdrop the glass shows; 0 turns the glass off
        void SetWindowImageTextureID(const u32 value);
        const std::optional<u32>& GetWindowImageTextureID() const;

        /// @brief Sets the noise the glass is made of
        void SetGlassNoise(const GlassNoise& value);
        const std::optional<GlassNoise>& GetGlassNoise() const;

        /// @brief Sets the fog's opacity
        void SetFogAlpha(const f32 value);
        const std::optional<f32>& GetFogAlpha() const;

        /// @brief Sets how fast the fog drifts, in pixels per second
        void SetFogSpeed(const f32 value);
        const std::optional<f32>& GetFogSpeed() const;

        /// @brief Sets the fog's layers of noise and their weights; none turn the fog off
        void SetFogDensity(std::span<const f32> value);
        const std::optional<std::vector<f32>>& GetFogDensity() const;

        /// @brief Sets how much of the fog clears, 0 to 1
        void SetFogClearing(const f32 value);
        const std::optional<f32>& GetFogClearing() const;

        /// @brief Sets the noise the fog is made of
        void SetFogNoise(const FogNoise& value);
        const std::optional<FogNoise>& GetFogNoise() const;

        /// @brief Sets how far the image stays clear of the edges, in pixels
        void SetImagePadding(const f32 value);
        const std::optional<f32>& GetImagePadding() const;

        /// @brief Sets how the image is laid over the part
        void SetImageFit(const ImageFit value);
        const std::optional<ImageFit>& GetImageFit() const;

        /// @brief Sets the grayscale mask; 0 shows none
        void SetMaskTextureID(const u32 value);
        const std::optional<u32>& GetMaskTextureID() const;

        /// @brief Sets how the mask works
        void SetMaskMode(const MaskMode value);
        const std::optional<MaskMode>& GetMaskMode() const;

        /// @brief Sets how the mask is laid over the part
        void SetMaskFit(const ImageFit value);
        const std::optional<ImageFit>& GetMaskFit() const;

        /// @brief Sets the gaps cut out of a background
        void SetGaps(const BackgroundGap& value);
        const std::optional<BackgroundGap>& GetBackgroundGaps() const;

        /// @brief Sets the gaps cut out of a border
        void SetGaps(std::span<const BorderGap> value);
        const std::optional<std::vector<BorderGap>>& GetBorderGaps() const;

        /// @brief Sets the pixelation's block width in pixels; 0 turns it off
        void SetPixelateBlockSize(const f32 value);
        const std::optional<f32>& GetPixelateBlockSize() const;

        /// @brief Sets the CRT look
        void SetCrtEffect(const CrtEffect& value);
        const std::optional<CrtEffect>& GetCrtEffect() const;

    private:
        std::array<std::vector<glm::vec4>, 5> _colors {}; //per ColorState, in its order.
        std::optional<glm::vec4> _cornerRadii {};
        std::optional<CutCorners> _cutCorners {};
        std::optional<f32> _width {};
        std::optional<std::string> _image {};
        FlavorEffect
            _dropShadow {},
            _glow {},
            _innerGlow {};
        std::optional<FillType> _fillType {};
        std::optional<f32> _gradientDegree {};
        std::optional<f32> _gradientOffset {};
        std::optional<f32> _gradientAnimationSpeed {};
        std::optional<f32> _gradientRotationSpeed {};
        std::optional<RasterPattern> _rasterPattern {};
        std::optional<glm::vec4> _primaryRasterColor {};
        std::optional<glm::vec4> _secondaryRasterColor {};
        std::optional<f32> _rasterSpacing {};
        std::optional<f32> _rasterDegree {};
        std::optional<std::vector<f32>> _primaryRasterWidthTransfer {};
        std::optional<f32> _dotTransparencyTransfer {};
        std::optional<f32> _rasterAnimationSpeed {};
        std::optional<u32> _windowImageTextureID {};
        std::optional<GlassNoise> _glassNoise {};
        std::optional<f32> _fogAlpha {};
        std::optional<f32> _fogSpeed {};
        std::optional<std::vector<f32>> _fogDensity {};
        std::optional<f32> _fogClearing {};
        std::optional<FogNoise> _fogNoise {};
        std::optional<f32> _imagePadding {};
        std::optional<ImageFit> _imageFit {};
        std::optional<u32> _maskTextureID {};
        std::optional<MaskMode> _maskMode {};
        std::optional<ImageFit> _maskFit {};
        std::optional<BackgroundGap> _backgroundGaps {};
        std::optional<std::vector<BorderGap>> _borderGaps {};
        std::optional<f32> _pixelateBlockSize {};
        std::optional<CrtEffect> _crtEffect {};
    };

    /// @brief A named style rule, like a CSS class: what it sets for the background, the border and the text of every
    ///        widget that has it (IWidget::AddFlavor). Kept by the StylingManager
    RFG_EXPORT class Flavor final
    {
    public:
        Flavor() = default;
        Flavor(const Flavor&) = delete;
        Flavor(Flavor&&) = delete;
        auto operator =(const Flavor&) = delete;
        auto operator =(Flavor&&) = delete;

        /// @brief Returns what the flavor sets for its widgets' backgrounds
        FlavorPart& GetBackground();
        const FlavorPart& GetBackground() const;

        /// @brief Returns what the flavor sets for its widgets' borders
        FlavorPart& GetBorder();
        const FlavorPart& GetBorder() const;

        /// @brief Returns what the flavor sets for its widgets' texts
        FlavorPart& GetText();
        const FlavorPart& GetText() const;

        /// @brief Returns what the flavor sets for one of its widgets' parts
        const FlavorPart& GetPart(const StylePart part) const;

    private:
        FlavorPart
            _background {},
            _border {},
            _text {};
    };
}
