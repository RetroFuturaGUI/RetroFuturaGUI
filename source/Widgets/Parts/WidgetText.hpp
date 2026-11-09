#pragma once
#include "IncludeHelper.hpp"
#include "Outline.hpp"
#include "Signal.hpp"
#include "Text.hpp"
#include <functional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace RetroFuturaGUI
{
    /// @brief What changed in a WidgetText, for its owning widget's hook
    enum class WidgetTextChange : u32
    {
        String,
        Font,
        Padding
    };

    /// @brief A widget's text: its string, font, colors per state and effects. The widget owns it and hands it out through
    ///        GetText(), so it is used as button.GetText().SetString("OK", false) or button.GetText().SetColors(...)
    class WidgetText final
    {
    public:
        /// @param changeHook What the owning widget does when the string, font or padding change, before OnTextChange fires: lay itself out again, say, or keep its caret in place
        /// @param follower A second text taking the same font, alignment and padding: a text box's placeholder
        explicit WidgetText(Projection* projection, std::function<void(WidgetTextChange)> changeHook = {}, WidgetText* follower = nullptr);
        WidgetText() = delete;
        WidgetText(const WidgetText&) = delete;
        WidgetText(WidgetText&&) = delete;
        ~WidgetText() = default;
        auto operator =(const WidgetText&) = delete;
        auto operator =(WidgetText&&) = delete;

    //String
        /// @brief Sets the string (UTF-8)
        void SetString(std::string_view string, const bool emitSignal);

        /// @brief Returns the string (in UTF-8)
        const std::string& GetString() const;

        /// @brief Returns the string (UTF-8) as a null-terminated array. It stays valid until the string changes
        const char* GetCString() const;

        /// @brief Connects a slot to be called when the string changes
        /// @param async If true, the slot is invoked asynchronously
        void Connect_OnTextChange(const typename Signal<>::Slot& slot, const bool async);

        /// @brief Disconnects a previously connected OnTextChange slot
        void Disconnect_OnTextChange(const typename Signal<>::Slot& slot);

    //Layout
        /// @brief Sets the font family, size and style, loading it if necessary
        void SetFontFamily(std::string_view fontFamily, const f32 fontSize, const FontSizeUnit sizeUnit, const PlatformBridge::Fonts::Slant slant, const PlatformBridge::Fonts::Weight fontWeight);

        /// @brief Returns the font size in the given unit
        f32 GetFontSize(const FontSizeUnit sizeUnit) const;

        /// @brief Sets the horizontal alignment
        void SetAlignment(const TextAlignment alignment);

        /// @brief Sets how the lines run: horizontally (default), or vertically, laid out left to right or right to left. Each line break starts the next column
        void SetFlow(const TextFlow textFlow);

        /// @brief Sets the padding applied around the text
        void SetPadding(const f32 padding);

        /// @brief Returns the padding applied around the text
        f32 GetPadding() const;

        /// @brief Returns the rendered width of the string (pixels)
        f32 GetWidth() const;

    //Fill
        /// @brief Sets the colors for the given color state. One for a solid fill, the stops for a gradient. Selected colors are
        ///        optional: without any, the Selected state shows the Enabled colors. A widget that can't be selected, like a Label, never shows them.
        void SetColors(std::span<const glm::vec4> colors, const ColorState state);

        /// @brief Returns the colors configured for the given color state
        const std::vector<glm::vec4>& GetColors(const ColorState state) const;

        /// @brief Sets the fill type (solid, linear shift, radial or hue star gradient). A gradient runs across the whole text
        void SetFillType(const FillType fillType);

        /// @brief Sets the offset applied to the gradient's start position
        void SetGradientOffset(const f32 gradientOffset);

        /// @brief Sets the speed at which the gradient animates over time
        void SetGradientAnimationSpeed(const f32 animationSpeed);

        /// @brief Sets the angle of the linear shift (degrees)
        void SetGradientDegree(const f32 degree);

        /// @brief Sets the speed at which the gradient rotates over time (degrees per second)
        void SetGradientRotationSpeed(const f32 rotationSpeed);

        /// @brief Sets how far emoji take on the fill color, from 0, their own colors (default), to 1. The raster, fog, glass and mask apply to them either way
        void SetColorGlyphTint(const f32 tint);

    //Glass and image
        /// @brief Sets the window-wide backdrop the glass shows, which turns the glass on. 0 turns it off
        void SetWindowBackgroundImageTextureID(const u32 textureID);

        /// @brief Sets the noise the glass is made of. How big its cells are, how far it shifts the window behind, its smoothing and seed
        void SetGlassNoise(const GlassNoise& noise);

        /// @brief Sets an image shown inside the glyphs, stretched across the whole text behind its fill. The colors' alpha decides how much of it shows: 0 shows the image alone. 0 shows no image
        /// @note The text only borrows the texture. A video's texture changes only while its VideoPlayback is updated, and gets a new ID when its picture size changes, so pass its current ID again then
        void SetImageTextureID(const u32 textureID);

        /// @brief Sets how far the image stays clear of the text's edges, per side in pixels
        void SetImagePadding(const f32 padding);

        /// @brief Sets how the image is laid across the text Stretched by default, or keeping its proportions while filling the text (cropped) or fitting into it
        void SetImageFit(const ImageFit fit);

    //Mask
        /// @brief Sets a grayscale mask laid across the text, its red channel, the gray as transparency or as shading (SetMaskMode). A shading mask of bright and dark bands over a metal image gives chrome text. 0 shows no mask
        void SetMaskTextureID(const u32 textureID);

        /// @brief Sets what the mask does. Make the text see-through (Alpha, the default) or shade it (Shade).
        void SetMaskMode(const MaskMode mode);

        /// @brief Sets how the mask is laid across the text. Stretched (default) or keeping its proportions
        void SetMaskFit(const ImageFit fit);

        /// @brief Sets what the mask is laid over. The whole text (default) or each line on its own, so a chrome mask shades every line of a multiline text as it would a one-line logo
        void SetMaskSpan(const MaskSpan maskSpan);

    //Raster
        /// @brief Sets which pattern the raster draws
        void SetRasterPattern(const RasterPattern rasterPattern);

        /// @brief Sets the raster's primary color. The dots, one of the two square colors, every other stripe, or the grid's lines
        ///        across the raster degree
        void SetPrimaryRasterColor(const glm::vec4& color);

        /// @brief Sets the raster's secondary color. The other square color, the stripes between the primary ones, or the grid's
        ///        lines along the raster degree
        void SetSecondaryRasterColor(const glm::vec4& color);

        /// @brief Sets the gap (in pixels) between the largest elements of the raster. 0 lets them touch. Spreads the grid without changing any element's size
        void SetRasterSpacing(const f32 spacing);

        /// @brief Sets the direction (in degrees) along which the raster's widths are sampled and along which it animates
        void SetRasterDegree(const f32 degree);

        /// @brief Sets the raster's width curve, in pixels, sampled across the text. Turns the raster on when non-empty.
        void SetPrimaryRasterWidthTransfer(std::span<const f32> widthTransfer);

        /// @brief Sets how far each of the raster's dots stays opaque before fading out
        void SetDotTransparencyTransfer(const f32 transparencyTransfer);

        /// @brief Sets the speed at which the raster animates
        void SetRasterAnimationSpeed(const f32 animationSpeed);

    //Fog
        /// @brief Sets the overall opacity of the fog
        void SetFogAlpha(const f32 alpha);

        /// @brief Sets the speed at which the fog drifts over time
        void SetFogSpeed(const f32 speed);

        /// @brief Sets the per-octave density/weight curve driving the fog. Turns the fog on when non-empty
        void SetFogDensity(std::span<const f32> density);

        /// @brief Sets the coverage threshold above which the fog appears
        void SetFogClearing(const f32 clearing);

        /// @brief Sets the noise the fog is made of. How big its cloud and shading cells are, its smoothing and seed
        void SetFogNoise(const FogNoise& noise);

    //Outline
        /// @brief Returns the outline around the glyphs, to style it: GetOutline().SetWidth(...)
        Outline& GetOutline();
        const Outline& GetOutline() const;

        /// @brief Returns the text's drop shadow, to style it: GetDropShadow().Show(true). Colors per state, following the text's
        DropShadow& GetDropShadow();
        const DropShadow& GetDropShadow() const;

        /// @brief Returns the text's glow, to style it: GetGlow().Show(true). Colors per state, following the text's
        Glow& GetGlow();
        const Glow& GetGlow() const;

        /// @brief Returns the text's inner glow, to style it: GetInnerGlow().Show(true). Colors per state, following the text's.
        ///        Set off with SetDistance in a dark color, it is an inner shadow
        InnerGlow& GetInnerGlow();
        const InnerGlow& GetInnerGlow() const;

    //Called by the owning widget
        /// @brief Draws the text
        void Draw();

        /// @brief Sets the area the text lays itself out in. Called by the widget as it is laid out
        void SetSize(const glm::vec2& size);

        /// @brief Places the text. Called by the widget as it is laid out
        void SetPosition(const glm::vec3& position);

        /// @brief Shows the colors of the given state. Called by the widget when it is hovered, pressed, selected, enabled or disabled.
        void SetColorState(const ColorState state);

        /// @brief Has the colors move to new values over the widget's transitions
        void SetTransitionSettings(const TransitionSettings* settings);

        /// @brief Emits OnTextChange for text the widget changed through GetElement
        void EmitChange();

        /// @brief Returns the drawn text, for the widget's layout and editing
        Text& GetElement();
        const Text& GetElement() const;

    private:
        /// @brief The colors of one text part, per color state
        struct StateColors
        {
            std::vector<glm::vec4>
                _Enabled {},
                _Disabled {},
                _Clicked {},
                _Selected {},
                _Hover {};
        };

        static std::vector<glm::vec4>& colorsOf(StateColors& colors, const ColorState state);
        static const std::vector<glm::vec4>& colorsOf(const StateColors& colors, const ColorState state);

        /// @brief The colors a state shows. Its own, or the Enabled ones for a Selected state that has none
        static const std::vector<glm::vec4>& shownColorsOf(const StateColors& colors, const ColorState state);

        /// @brief Hands the text and its outline the colors of the current state
        void setColors();

        /// @brief Tells the widget what changed, if it asked to know
        void notifyChange(const WidgetTextChange change);

        Text _text;
        StateColors _colors { ._Enabled = { glm::vec4(1.0f) }, ._Disabled = { glm::vec4(0.5f, 0.5f, 0.5f, 1.0f) }, ._Clicked = { glm::vec4(1.0f) }, ._Selected = {}, ._Hover = { glm::vec4(1.0f) } };
        Outline _outline; //goes around _text, so it is declared after it
        ColorState _colorState { ColorState::Enabled };
        std::vector<f32>
            _primaryRasterWidthTransfer {},
            _fogDensity {}; //the text keeps spans over these
        std::function<void(WidgetTextChange)> _changeHook {};
        WidgetText* _follower { nullptr };
        Signal<>
            _onTextChange,
            _onTextChangeAsync;
    };
}