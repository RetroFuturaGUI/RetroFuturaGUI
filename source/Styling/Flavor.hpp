#pragma once
#include "IncludeHelper.hpp"
#include "IRectangle.hpp"
#include "Plane.hpp"
#include "Frame.hpp"
#include "FillStyle.hpp"
#include "Noise.hpp"
#include "FontManager.hpp"
#include "TextLayout.hpp"
#include "Transition.hpp"
#include <array>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace RetroFuturaGUI
{
    /// @brief The part of a widget a flavor's part style applies to: the widget's own background, border and text, then the
    ///        parts inside it, named after the flavor's getters that reach them (IndicatorBackground: GetIndicator().GetBackground())
    RFG_EXPORT enum class StylePart : u32
    {
        Background,
        Border,
        Text,
        IndicatorBackground,
        IndicatorBorder,
        Graph,
        ListBackground,
        ListBorder,
        ListHighlight,
        ItemStyleBackground,
        ItemStyleBorder,
        SeparatorLine,
        FieldStyleBackground,
        FieldStyleBorder,
        CloseButtonBackground,
        SelectionBandBackground,
        SelectionBandBorder,
        HoverTextBackground,
        HoverTextText,
        Placeholder,
        Preview,
        Checkmark,
        Arrow,
        CloseButtonIcon,
        Caret,
        SelectedArea,
        ItemText,
        CellStyleBackground,
        CellStyleInnerBorder,
        CellStyleText,
        CellStyleIcon,
        CellStyleDropTarget,
        HeaderStyleBackground,
        HeaderStyleInnerBorder,
        HeaderStyleOuterBorder,
        HeaderStyleText,
        CellCheckBoxBackground,
        CellCheckBoxBorder,
        CellCheckBoxCheckmark
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
        CrtEffect,
        FontFamily,
        FontSize,
        FontSlant,
        FontWeight,
        Alignment,
        Padding,
        Flow,
        ColorGlyphTint,
        MaskSpan
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
    ///        width, an image, every shader setting (fill, raster, glass, fog, masks, gaps, pixelation, CRT), and for a text its
    ///        font, alignment, padding, flow and outline. Whatever it leaves unset comes from the widget's next flavor, then
    ///        .default, then the widget's built-in look; whatever the widget sets itself wins over all of them. A change shows on every widget with the flavor from its next draw.
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

        /// @brief Sets a text's font size, kept in pixels
        void SetFontSize(const f32 size, const FontSizeUnit sizeUnit);

        /// @brief Returns the font size set, in pixels
        const std::optional<f32>& GetFontSize() const;

        /// @brief Sets a text's font family, loaded when a text first shows it
        void SetFontFamily(std::string_view value);
        const std::optional<std::string>& GetFontFamily() const;

        /// @brief Sets a text's slant
        void SetFontSlant(const PlatformBridge::Fonts::Slant value);
        const std::optional<PlatformBridge::Fonts::Slant>& GetFontSlant() const;

        /// @brief Sets a text's weight
        void SetFontWeight(const PlatformBridge::Fonts::Weight value);
        const std::optional<PlatformBridge::Fonts::Weight>& GetFontWeight() const;

        /// @brief Sets a text's horizontal alignment
        void SetAlignment(const TextAlignment value);
        const std::optional<TextAlignment>& GetAlignment() const;

        /// @brief Sets the padding around a text, in pixels
        void SetPadding(const f32 value);
        const std::optional<f32>& GetPadding() const;

        /// @brief Sets how a text's lines run
        void SetFlow(const TextFlow value);
        const std::optional<TextFlow>& GetFlow() const;

        /// @brief Sets how far emoji take on a text's fill color, 0 to 1
        void SetColorGlyphTint(const f32 value);
        const std::optional<f32>& GetColorGlyphTint() const;

        /// @brief Sets what a text's mask is laid over
        void SetMaskSpan(const MaskSpan value);
        const std::optional<MaskSpan>& GetMaskSpan() const;

        /// @brief Returns what the flavor sets for a text's outline: its width, colors per state and fill. Shown and the
        ///        other effect numbers don't apply to it
        FlavorEffect& GetOutline();
        const FlavorEffect& GetOutline() const;

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
        std::optional<f32> _fontSize {}; //in pixels
        std::optional<std::string> _fontFamily {};
        std::optional<PlatformBridge::Fonts::Slant> _fontSlant {};
        std::optional<PlatformBridge::Fonts::Weight> _fontWeight {};
        std::optional<TextAlignment> _alignment {};
        std::optional<f32> _padding {};
        std::optional<TextFlow> _flow {};
        std::optional<f32> _colorGlyphTint {};
        std::optional<MaskSpan> _maskSpan {};
        FlavorEffect _outline {};
    };

    /// @brief What a flavor sets for a part made of a background and a border: a ranged value's or a color
    ///        picker's indicator (the color picker's has only the border), a menu bar's items, a window tab's fields, a table's
    ///        selection band.
    RFG_EXPORT class FlavorBox final
    {
    public:
        FlavorBox() = default;
        FlavorBox(const FlavorBox&) = delete;
        FlavorBox(FlavorBox&&) = delete;
        auto operator =(const FlavorBox&) = delete;
        auto operator =(FlavorBox&&) = delete;

        FlavorPart& GetBackground();
        const FlavorPart& GetBackground() const;

        FlavorPart& GetBorder();
        const FlavorPart& GetBorder() const;

    private:
        FlavorPart
            _background {},
            _border {};
    };

    /// @brief What a flavor sets for a drop-down's open list: its background, its border and the highlight
    ///        over the hovered row.
    RFG_EXPORT class FlavorList final
    {
    public:
        FlavorList() = default;
        FlavorList(const FlavorList&) = delete;
        FlavorList(FlavorList&&) = delete;
        auto operator =(const FlavorList&) = delete;
        auto operator =(FlavorList&&) = delete;

        FlavorPart& GetBackground();
        const FlavorPart& GetBackground() const;

        FlavorPart& GetBorder();
        const FlavorPart& GetBorder() const;

        FlavorPart& GetHighlight();
        const FlavorPart& GetHighlight() const;

    private:
        FlavorPart
            _background {},
            _border {},
            _highlight {};
    };

    /// @brief What a flavor sets for a window tab's close button: its background and its icon's colors per state and fill.
    RFG_EXPORT class FlavorCloseButton final
    {
    public:
        FlavorCloseButton() = default;
        FlavorCloseButton(const FlavorCloseButton&) = delete;
        FlavorCloseButton(FlavorCloseButton&&) = delete;
        auto operator =(const FlavorCloseButton&) = delete;
        auto operator =(FlavorCloseButton&&) = delete;

        FlavorPart& GetBackground();
        const FlavorPart& GetBackground() const;

        FlavorPart& GetIcon();
        const FlavorPart& GetIcon() const;

    private:
        FlavorPart
            _background {},
            _icon {};
    };

    /// @brief What a flavor sets for a line diagram's hover text: the box behind it and the text.
    RFG_EXPORT class FlavorHoverText final
    {
    public:
        FlavorHoverText() = default;
        FlavorHoverText(const FlavorHoverText&) = delete;
        FlavorHoverText(FlavorHoverText&&) = delete;
        auto operator =(const FlavorHoverText&) = delete;
        auto operator =(FlavorHoverText&&) = delete;

        FlavorPart& GetBackground();
        const FlavorPart& GetBackground() const;

        FlavorPart& GetText();
        const FlavorPart& GetText() const;

    private:
        FlavorPart
            _background {},
            _text {};
    };

    /// @brief The font, alignment and padding of a table's cell or header texts, each unset until set.
    RFG_EXPORT struct TableTextLayer
    {
        std::optional<std::string> _FontFamily {};
        std::optional<f32> _FontSize {}; //in pixels.
        std::optional<PlatformBridge::Fonts::Slant> _Slant {};
        std::optional<PlatformBridge::Fonts::Weight> _Weight {};
        std::optional<TextAlignment> _Alignment {};
        std::optional<f32> _Padding {};
    };

    /// @brief What a flavor sets for one track variant of its tables: the colors its cells show while enabled. A table's nth
    ///        variant takes the flavor's nth; how many variants alternate stays the table's (SetTrackAlternatingColorCount).
    ///        Reached by flavor.GetTrackStyle(n)
    RFG_EXPORT class FlavorTrackStyle final
    {
    public:
        FlavorTrackStyle() = default;
        FlavorTrackStyle(const FlavorTrackStyle&) = delete;
        FlavorTrackStyle(FlavorTrackStyle&&) = delete;
        auto operator =(const FlavorTrackStyle&) = delete;
        auto operator =(FlavorTrackStyle&&) = delete;

        /// @brief Sets the cells' background colors. No colors leave them unset
        void SetBackgroundColors(std::span<const glm::vec4> colors);
        const std::vector<glm::vec4>& GetBackgroundColors() const;

        /// @brief Sets the colors of the border around each cell
        void SetInnerBorderColors(std::span<const glm::vec4> colors);
        const std::vector<glm::vec4>& GetInnerBorderColors() const;

        /// @brief Sets the text colors
        void SetTextColors(std::span<const glm::vec4> colors);
        const std::vector<glm::vec4>& GetTextColors() const;

        /// @brief Sets the icon colors: the checkmark of the track's checkbox cells
        void SetIconColors(std::span<const glm::vec4> colors);
        const std::vector<glm::vec4>& GetIconColors() const;

    private:
        std::vector<glm::vec4>
            _background {},
            _innerBorder {},
            _text {},
            _icon {};
    };

    /// @brief What a flavor sets for its tables' cells: their colors in the states every track shares (Disabled, Hover, Clicked,
    ///        Selected), the drop target's colors, the inner border's width and the texts' font, alignment and padding.
    ///        Reached by flavor.GetCellStyle()
    RFG_EXPORT class FlavorCellStyle final
    {
    public:
        FlavorCellStyle() = default;
        FlavorCellStyle(const FlavorCellStyle&) = delete;
        FlavorCellStyle(FlavorCellStyle&&) = delete;
        auto operator =(const FlavorCellStyle&) = delete;
        auto operator =(FlavorCellStyle&&) = delete;

        /// @brief Sets the cells' background colors for a shared state. Enabled is each track's own, so it is ignored here
        void SetBackgroundColors(std::span<const glm::vec4> colors, const ColorState state);
        const std::vector<glm::vec4>& GetBackgroundColors(const ColorState state) const;

        /// @brief Sets the colors of the border around each cell for a shared state
        void SetInnerBorderColors(std::span<const glm::vec4> colors, const ColorState state);
        const std::vector<glm::vec4>& GetInnerBorderColors(const ColorState state) const;

        /// @brief Sets the text colors for a shared state
        void SetTextColors(std::span<const glm::vec4> colors, const ColorState state);
        const std::vector<glm::vec4>& GetTextColors(const ColorState state) const;

        /// @brief Sets the icon colors (the checkbox cells' checkmark) for a shared state
        void SetIconColors(std::span<const glm::vec4> colors, const ColorState state);
        const std::vector<glm::vec4>& GetIconColors(const ColorState state) const;

        /// @brief Sets the background colors of the track a drag would drop on
        void SetDropTargetColors(std::span<const glm::vec4> colors);
        const std::vector<glm::vec4>& GetDropTargetColors() const;

        /// @brief Sets the width of the border around each cell
        void SetInnerBorderWidth(const f32 width);
        const std::optional<f32>& GetInnerBorderWidth() const;

        /// @brief Set the texts' font family, size (kept in pixels), slant and weight, alignment and padding, each on its own
        void SetFontFamily(std::string_view fontFamily);
        void SetFontSize(const f32 fontSize, const FontSizeUnit sizeUnit);
        void SetFontSlant(const PlatformBridge::Fonts::Slant slant);
        void SetFontWeight(const PlatformBridge::Fonts::Weight fontWeight);
        void SetTextAlignment(const TextAlignment alignment);
        void SetTextPadding(const f32 padding);
        const TableTextLayer& GetTextSettings() const;

    private:
        friend class Flavor; //reaches its parts by StylePart.

        static bool isShared(const ColorState state);

        FlavorPart
            _background {},
            _innerBorder {},
            _text {},
            _icon {},
            _dropTarget {};
        TableTextLayer _textSettings {};
    };

    /// @brief What a flavor sets for its tables' headers: their colors, Enabled and Disabled, the borders' widths and the texts'
    ///        font, alignment and padding. Reached by flavor.GetHeaderStyle()
    RFG_EXPORT class FlavorHeaderStyle final
    {
    public:
        FlavorHeaderStyle() = default;
        FlavorHeaderStyle(const FlavorHeaderStyle&) = delete;
        FlavorHeaderStyle(FlavorHeaderStyle&&) = delete;
        auto operator =(const FlavorHeaderStyle&) = delete;
        auto operator =(FlavorHeaderStyle&&) = delete;

        /// @brief Sets the header cells' background colors for Enabled or Disabled. Any other state sets the Enabled ones
        void SetBackgroundColors(std::span<const glm::vec4> colors, const ColorState state);
        const std::vector<glm::vec4>& GetBackgroundColors(const ColorState state) const;

        /// @brief Sets the colors of the border around each header cell for Enabled or Disabled
        void SetInnerBorderColors(std::span<const glm::vec4> colors, const ColorState state);
        const std::vector<glm::vec4>& GetInnerBorderColors(const ColorState state) const;

        /// @brief Sets the colors of the border around the header band for Enabled or Disabled
        void SetOuterBorderColors(std::span<const glm::vec4> colors, const ColorState state);
        const std::vector<glm::vec4>& GetOuterBorderColors(const ColorState state) const;

        /// @brief Sets the header text colors for Enabled or Disabled
        void SetTextColors(std::span<const glm::vec4> colors, const ColorState state);
        const std::vector<glm::vec4>& GetTextColors(const ColorState state) const;

        /// @brief Sets the width of the border around each header cell
        void SetInnerBorderWidth(const f32 width);
        const std::optional<f32>& GetInnerBorderWidth() const;

        /// @brief Sets the width of the border around the header band
        void SetOuterBorderWidth(const f32 width);
        const std::optional<f32>& GetOuterBorderWidth() const;

        /// @brief Set the texts' font family, size (kept in pixels), slant and weight, alignment and padding, each on its own
        void SetFontFamily(std::string_view fontFamily);
        void SetFontSize(const f32 fontSize, const FontSizeUnit sizeUnit);
        void SetFontSlant(const PlatformBridge::Fonts::Slant slant);
        void SetFontWeight(const PlatformBridge::Fonts::Weight fontWeight);
        void SetTextAlignment(const TextAlignment alignment);
        void SetTextPadding(const f32 padding);
        const TableTextLayer& GetTextSettings() const;

    private:
        friend class Flavor; //reaches its parts by StylePart.

        static ColorState headerState(const ColorState state);

        FlavorPart
            _background {},
            _innerBorder {},
            _outerBorder {},
            _text {};
        TableTextLayer _textSettings {};
    };

    /// @brief What a flavor sets for its tables' cell checkbox: its background, its border and its checkmark's fill (the
    ///        checkmark's colors are the tracks' icon colors). Reached by flavor.GetCellCheckBox()
    RFG_EXPORT class FlavorCellCheckBox final
    {
    public:
        FlavorCellCheckBox() = default;
        FlavorCellCheckBox(const FlavorCellCheckBox&) = delete;
        FlavorCellCheckBox(FlavorCellCheckBox&&) = delete;
        auto operator =(const FlavorCellCheckBox&) = delete;
        auto operator =(FlavorCellCheckBox&&) = delete;

        FlavorPart& GetBackground();
        const FlavorPart& GetBackground() const;

        FlavorPart& GetBorder();
        const FlavorPart& GetBorder() const;

        FlavorPart& GetCheckmark();
        const FlavorPart& GetCheckmark() const;

    private:
        FlavorPart
            _background {},
            _border {},
            _checkmark {};
    };

    /// @brief A named style rule, like a CSS class: what it sets for the background, the border and the text of every
    ///        widget that has it, and for the widget itself (its transitions and border gap) (IWidget::AddFlavor). Kept by the StylingManager
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


        /// @brief Returns what the flavor sets for its widgets' indicator (a ranged value's handle, a color picker's ring), made the first time
        FlavorBox& GetIndicator();

        /// @brief Returns what the flavor sets for its widgets' indicator (a ranged value's handle, a color picker's ring), nullptr when it sets none
        const FlavorBox* FindIndicator() const;

        /// @brief Returns what the flavor sets for its ranged values' graph, made the first time
        FlavorPart& GetGraph();

        /// @brief Returns what the flavor sets for its ranged values' graph, nullptr when it sets none
        const FlavorPart* FindGraph() const;

        /// @brief Returns what the flavor sets for its drop-downs' open list, made the first time
        FlavorList& GetList();

        /// @brief Returns what the flavor sets for its drop-downs' open list, nullptr when it sets none
        const FlavorList* FindList() const;

        /// @brief Returns what the flavor sets for its menu bars' items, made the first time
        FlavorBox& GetItemStyle();

        /// @brief Returns what the flavor sets for its menu bars' items, nullptr when it sets none
        const FlavorBox* FindItemStyle() const;

        /// @brief Returns what the flavor sets for its menu bars' separator lines, made the first time
        FlavorPart& GetSeparatorLine();

        /// @brief Returns what the flavor sets for its menu bars' separator lines, nullptr when it sets none
        const FlavorPart* FindSeparatorLine() const;

        /// @brief Returns what the flavor sets for its window tabs' fields, the selected one in its Selected state, made the first time
        FlavorBox& GetFieldStyle();

        /// @brief Returns what the flavor sets for its window tabs' fields, the selected one in its Selected state, nullptr when it sets none
        const FlavorBox* FindFieldStyle() const;

        /// @brief Returns what the flavor sets for its window tabs' close buttons, made the first time
        FlavorCloseButton& GetCloseButton();

        /// @brief Returns what the flavor sets for its window tabs' close buttons, nullptr when it sets none
        const FlavorCloseButton* FindCloseButton() const;

        /// @brief Returns what the flavor sets for its tables' selection band, made the first time
        FlavorBox& GetSelectionBand();

        /// @brief Returns what the flavor sets for its tables' selection band, nullptr when it sets none
        const FlavorBox* FindSelectionBand() const;

        /// @brief Returns what the flavor sets for its line diagrams' hover text, made the first time
        FlavorHoverText& GetHoverText();

        /// @brief Returns what the flavor sets for its line diagrams' hover text, nullptr when it sets none
        const FlavorHoverText* FindHoverText() const;

        /// @brief Returns what the flavor sets for its text boxes' placeholder text, made the first time
        FlavorPart& GetPlaceholder();

        /// @brief Returns what the flavor sets for its text boxes' placeholder text, nullptr when it sets none
        const FlavorPart* FindPlaceholder() const;

        /// @brief Returns what the flavor sets for its extended combo boxes' preview text, made the first time
        FlavorPart& GetPreview();

        /// @brief Returns what the flavor sets for its extended combo boxes' preview text, nullptr when it sets none
        const FlavorPart* FindPreview() const;


        /// @brief Returns what the flavor sets for its check boxes' checkmark: colors per state and fill, made the first time
        FlavorPart& GetCheckmark();

        /// @brief Returns what the flavor sets for its check boxes' checkmark: colors per state and fill, nullptr when it sets none
        const FlavorPart* FindCheckmark() const;

        /// @brief Returns what the flavor sets for its drop-downs' arrow: colors per state and fill, made the first time
        FlavorPart& GetArrow();

        /// @brief Returns what the flavor sets for its drop-downs' arrow: colors per state and fill, nullptr when it sets none
        const FlavorPart* FindArrow() const;

        /// @brief Returns what the flavor sets for its editable texts' caret: its colors (the Enabled ones) and fill, made the first time
        FlavorPart& GetCaret();

        /// @brief Returns what the flavor sets for its editable texts' caret: its colors (the Enabled ones) and fill, nullptr when it sets none
        const FlavorPart* FindCaret() const;

        /// @brief Returns what the flavor sets for its editable texts' selection highlight: its colors (the Enabled ones), fill and corner radii, made the first time
        FlavorPart& GetSelectedArea();

        /// @brief Returns what the flavor sets for its editable texts' selection highlight: its colors (the Enabled ones), fill and corner radii, nullptr when it sets none
        const FlavorPart* FindSelectedArea() const;

        /// @brief Returns what the flavor sets for its combo boxes' item texts: colors per state, made the first time
        FlavorPart& GetItemText();

        /// @brief Returns what the flavor sets for its combo boxes' item texts: colors per state, nullptr when it sets none
        const FlavorPart* FindItemText() const;


        /// @brief Returns what the flavor sets for its tables' nth track variant, made the first time
        FlavorTrackStyle& GetTrackStyle(const uSize nthIndex);

        /// @brief Returns what the flavor sets for its tables' nth track variant, nullptr when it sets none
        const FlavorTrackStyle* FindTrackStyle(const uSize nthIndex) const;

        /// @brief Returns what the flavor sets for its tables' cells in the shared states, their drop target, inner border width and texts, made the first time
        FlavorCellStyle& GetCellStyle();

        /// @brief Returns what the flavor sets for its tables' cells in the shared states, their drop target, inner border width and texts, nullptr when it sets none
        const FlavorCellStyle* FindCellStyle() const;

        /// @brief Returns what the flavor sets for its tables' headers, made the first time
        FlavorHeaderStyle& GetHeaderStyle();

        /// @brief Returns what the flavor sets for its tables' headers, nullptr when it sets none
        const FlavorHeaderStyle* FindHeaderStyle() const;

        /// @brief Returns what the flavor sets for its tables' cell checkbox, made the first time
        FlavorCellCheckBox& GetCellCheckBox();

        /// @brief Returns what the flavor sets for its tables' cell checkbox, nullptr when it sets none
        const FlavorCellCheckBox* FindCellCheckBox() const;

        /// @brief Returns what the flavor sets for one of its widgets' parts, nullptr for a part inside the widget it sets nothing for
        const FlavorPart* FindPart(const StylePart part) const;

        /// @brief Sets the transition of one or more properties for every widget with the flavor, in place of the global default
        void SetTransition(const TransitionProperty properties, const Transition& transition);

        /// @brief Leaves the transition of one or more properties unset, so it comes from further down
        void ResetTransition(const TransitionProperty properties);

        /// @brief Returns the transitions the flavor sets
        const TransitionSettings& GetTransitions() const;

        /// @brief Sets the gap between the border and the background (top, right, bottom, left, in pixels) of every widget with
        ///        the flavor. See IWidget::SetBorderGap
        void SetBorderGap(const glm::vec4& gap);

        /// @brief Leaves the border gap unset, so it comes from further down
        void ResetBorderGap();
        const std::optional<glm::vec4>& GetBorderGap() const;

    private:
        FlavorPart
            _background {},
            _border {},
            _text {};
        TransitionSettings _transitions {};
        std::optional<glm::vec4> _borderGap {};
        std::unique_ptr<FlavorBox> _indicator {};
        std::unique_ptr<FlavorPart> _graph {};
        std::unique_ptr<FlavorList> _list {};
        std::unique_ptr<FlavorBox> _itemStyle {};
        std::unique_ptr<FlavorPart> _separatorLine {};
        std::unique_ptr<FlavorBox> _fieldStyle {};
        std::unique_ptr<FlavorCloseButton> _closeButton {};
        std::unique_ptr<FlavorBox> _selectionBand {};
        std::unique_ptr<FlavorHoverText> _hoverText {};
        std::unique_ptr<FlavorPart> _placeholder {};
        std::unique_ptr<FlavorPart> _preview {};
        std::unique_ptr<FlavorPart> _checkmark {};
        std::unique_ptr<FlavorPart> _arrow {};
        std::unique_ptr<FlavorPart> _caret {};
        std::unique_ptr<FlavorPart> _selectedArea {};
        std::unique_ptr<FlavorPart> _itemText {};
        std::vector<std::unique_ptr<FlavorTrackStyle>> _trackStyles {}; //by variant; nullptr for one it doesn't set.
        std::unique_ptr<FlavorCellStyle> _cellStyle {};
        std::unique_ptr<FlavorHeaderStyle> _headerStyle {};
        std::unique_ptr<FlavorCellCheckBox> _cellCheckBox {};
    };
}