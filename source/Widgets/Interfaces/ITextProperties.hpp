#pragma once
#include "IncludeHelper.hpp"
#include <memory>
#include <span>
#include "Signal.hpp"
#include "Text.hpp"
#include "IWindowAccessor.hpp"

namespace RetroFuturaGUI
{
    //An interface to specialize a widget with text capabilities. It also inherits from IWindowAccessor to let its children 💎✨ Shine bright like a diamond ✨💎
    class ITextProperties : virtual public IWindowAccessor
    {
    public:
        /// @brief Sets the text colors for the given color state: one for a solid fill, the stops for a gradient.
        void SetTextColors(std::span<const glm::vec4> colors, const ColorState state);

        /// @brief Returns the text colors configured for the given color state.
        const std::vector<glm::vec4>& GetTextColors(const ColorState state) const;

        /// @brief Sets the text's fill type (solid, linear shift, radial or hue star gradient). A gradient runs across the whole text.
        void SetTextFillType(const FillType fillType);

        /// @brief Sets the offset applied to the text gradient's start position.
        void SetTextGradientOffset(const f32 gradientOffset);

        /// @brief Sets the speed at which the text gradient animates over time.
        void SetTextGradientAnimationSpeed(const f32 animationSpeed);

        /// @brief Sets the angle of the text's linear shift, in degrees.
        void SetTextGradientDegree(const f32 degree);

        /// @brief Sets the speed at which the text gradient rotates over time.
        void SetTextGradientRotationSpeed(const f32 rotationSpeed);

        /// @brief Sets the window-wide backdrop the text's glass shows, which turns the glass on. 0 turns it off.
        void SetTextWindowBackgroundImageTextureID(const u32 textureID);

        /// @brief Sets an image shown inside the glyphs, stretched across the whole text behind its fill. The text colors'
        ///        alpha decides how much of it shows: 0 shows the image alone. 0 shows no image.
        /// @note The text only borrows the texture. A video's texture changes only while its VideoPlayback is updated, and gets
        ///       a new ID when its picture size changes, so pass its current ID again then.
        void SetTextBackgroundImageTextureID(const u32 textureID);

        /// @brief Sets how far the text's image stays clear of the text's edges, per side in pixels.
        void SetTextBackgroundImagePadding(const f32 padding);

        /// @brief Sets how the text's image is laid across the text: stretched by default, or keeping its proportions while
        ///        filling the text (cropped) or fitting into it. Kept proportions stop the image widening as the text grows.
        void SetTextBackgroundImageFit(const ImageFit fit);

        /// @brief Sets a grayscale mask laid across the text, its red channel the gray: as transparency or as shading
        ///        (SetTextMaskMode). A shading mask of bright and dark bands over a metal image gives chrome text. 0 shows no mask.
        void SetTextMaskTextureID(const u32 textureID);

        /// @brief Sets what the text's mask does: make the text see-through (Alpha, the default) or shade it (Shade).
        void SetTextMaskMode(const MaskMode mode);

        /// @brief Sets how the text's mask is laid across the text: stretched (default) or keeping its proportions.
        void SetTextMaskFit(const ImageFit fit);

        /// @brief Sets what the text's mask is laid over: the whole text (default) or each line on its own, so a chrome
        ///        mask shades every line of a multiline text as it would a one-line logo.
        void SetTextMaskSpan(const MaskSpan maskSpan);

        /// @brief Sets which pattern the text's raster draws.
        void SetTextRasterPattern(const RasterPattern rasterPattern);

        /// @brief Sets the text raster's primary color: the dot color for Dotted, one of the two square colors for Checkered.
        void SetTextPrimaryRasterColor(const glm::vec4& color);

        /// @brief Sets the second of the two square colors of the text's Checkered raster.
        void SetTextSecondaryRasterColor(const glm::vec4& color);

        /// @brief Sets the gap, in pixels, between the largest elements of the text's raster: 0 lets them touch. Spreads the grid without changing any element's size.
        void SetTextRasterSpacing(const f32 spacing);

        /// @brief Sets the direction, in degrees, along which the text raster's widths are sampled and along which it animates.
        void SetTextRasterDegree(const f32 degree);

        /// @brief Sets the text raster's width curve, in pixels, sampled across the text. Turns the raster on when non-empty.
        void SetTextPrimaryRasterWidthTransfer(std::span<const f32> widthTransfer);

        /// @brief Sets how far each of the text raster's dots stays opaque before fading out.
        void SetTextDotTransparencyTransfer(const f32 transparencyTransfer);

        /// @brief Sets the speed at which the text's raster animates.
        void SetTextRasterAnimationSpeed(const f32 animationSpeed);

        /// @brief Sets the overall opacity of the text's fog.
        void SetTextFogAlpha(const f32 alpha);

        /// @brief Sets the speed at which the text's fog drifts over time.
        void SetTextFogSpeed(const f32 speed);

        /// @brief Sets the per-octave density/weight curve driving the text's fog. Turns the fog on when non-empty.
        void SetTextFogDensity(std::span<const f32> density);

        /// @brief Sets the coverage threshold above which the text's fog appears.
        void SetTextFogClearing(const f32 clearing);

        /// @brief Sets how far Emoji take on the text's fill color, from 0, their own colors (the default), to 1.
        ///        The raster, fog, glass and mask apply to them either way.
        void SetTextColorGlyphTint(const f32 tint);

        /// @brief Sets how far the border around every glyph reaches out, in pixels. 0, the default, draws no border.
        void SetTextBorderWidth(const f32 width);

        /// @brief Sets the text border's colors shown in the given state (black by default). More than one color makes the border's fill a gradient (just like the text)
        void SetTextBorderColors(std::span<const glm::vec4> colors, const ColorState state);

        /// @brief Returns the text border's colors for the given state.
        const std::vector<glm::vec4>& GetTextBorderColors(const ColorState state) const;

        /// @brief Sets the text border's fill type
        void SetTextBorderFillType(const FillType fillType);

        /// @brief Sets the offset applied to the text border gradient's start position.
        void SetTextBorderGradientOffset(const f32 gradientOffset);

        /// @brief Sets the speed at which the text border's gradient animates.
        void SetTextBorderGradientAnimationSpeed(const f32 animationSpeed);

        /// @brief Sets the angle, in degrees, of the text border's linear shift.
        void SetTextBorderGradientDegree(const f32 degree);

        /// @brief Sets the speed at which the text border's gradient rotates.
        void SetTextBorderGradientRotationSpeed(const f32 rotationSpeed);

        /// @brief Sets the text border's own image, stretched across the whole text behind the border's fill, whose alpha
        ///        decides how much of it shows. 0 shows no image.
        void SetTextBorderBackgroundImageTextureID(const u32 textureID);

        /// @brief Sets how far the text border's image stays clear of the text's edges, per side in pixels.
        void SetTextBorderBackgroundImagePadding(const f32 padding);

        /// @brief Sets how the text border's image is laid across the text: stretched, the default, or keeping its proportions.
        void SetTextBorderBackgroundImageFit(const ImageFit fit);

        /// @brief Sets the text content, in UTF-8.
        virtual void SetText(std::string_view text, const bool emitSignal);

        /// @brief Returns the text content, in UTF-8.
        const std::string& GetText() const;

        /// @brief Sets the font family, size and style used to render the text, loading it if necessary.
        virtual void SetFontFamily(std::string_view fontFamily, const f32 fontSize, const FontSizeUnit sizeUnit, const PlatformBridge::Fonts::Slant slant, const PlatformBridge::Fonts::Weight fontWeight);

        /// @brief Returns the font size in the given unit.
        f32 GetFontSize(const FontSizeUnit sizeUnit) const;

        /// @brief Sets the horizontal alignment of the text.
        virtual void SetTextAlignment(const TextAlignment alignment);

        /// @brief Sets how the text's lines run: horizontally (by default), orvertically, laid out left to right or right to left. Each line break starts the next column/row.
        void SetTextFlow(const TextFlow textFlow);

        /// @brief Sets the padding applied around the text.
        virtual void SetTextPadding(const f32 padding);

        /// @brief Returns the padding applied around the text.
        f32 GetTextPadding() const;

        /// @brief Returns the rendered width of the text content, in pixels.
        f32 GetTextWidth() const;

        /// @brief Connects a slot to be called when the text content changes.
        /// @param async If true, the slot is invoked asynchronously.
        void Connect_OnTextChange(const typename Signal<>::Slot& slot, const bool async);

        /// @brief Disconnects a previously connected OnTextChange slot.
        void Disconnect_OnTextChange(const typename Signal<>::Slot& slot);

    protected:
        void setTextColors();
        void drawText();

    //Elements
        std::unique_ptr<Text> _text;
        inline static const std::string _dummy {};

    //Logic
        Signal<>
            _onTextChange,
            _onTextChangeAsync;

    // Style
        std::vector<glm::vec4>
            _textColorEnabled { glm::vec4(1.0f) },
            _textColorDisabled { glm::vec4(0.5f, 0.5f,0.5f, 1.0f) },
            _textColorClicked { glm::vec4(1.0f) },
            _textColorHover { glm::vec4(1.0f, 1.0f, 1.0f, 1.0f) },
            _textBorderColorEnabled { glm::vec4(0.0f, 0.0f, 0.0f, 1.0f) },
            _textBorderColorDisabled { glm::vec4(0.0f, 0.0f, 0.0f, 1.0f) },
            _textBorderColorClicked { glm::vec4(0.0f, 0.0f, 0.0f, 1.0f) },
            _textBorderColorHover { glm::vec4(0.0f, 0.0f, 0.0f, 1.0f) };
        ColorState _textColorState { ColorState::Enabled };
        TextAlignment _textAlignment { TextAlignment::Left };
        std::vector<f32>
            _textPrimaryRasterWidthTransfer {},
            _textFogDensity {}; //the text keeps spans over these

    private:
        /// @brief Hands the text and its border the colors of one state.
        void setTextColorElements(std::vector<glm::vec4>& color, std::vector<glm::vec4>& borderColor);
    };
}