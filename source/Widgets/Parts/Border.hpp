#pragma once
#include "IncludeHelper.hpp"
#include <array>
#include <functional>
#include <memory>
#include <span>
#include "Frame.hpp"
#include "ShadowRectangle.hpp"
#include "GlowRectangle.hpp"
#include "InnerGlowRectangle.hpp"
#include "Projection.hpp"
#include "IWidgetDecoration.hpp"

namespace RetroFuturaGUI
{
    class Background;

    /// @brief What a border tells its owning widget, for a layout that depends on it
    struct BorderHooks
    {
        std::function<void()>
            _OnWidthChange {},
            _OnCornerRadiiChange {};
    };

    /// @brief A widget's border: a filled or gradient frame with optional raster and glass. The widget
    ///        owns it and hands it out through GetBorder(), so it is styled as button.GetBorder().SetColors(...)
    class Border final : public IWidgetDecoration
    {
        friend class IWidget; //links the background and tells whether there is a gap for the inward glow
        friend class Background; //draws the inward glow around its fill

    public:
        explicit Border(Projection* projection, BorderHooks hooks = {});
        Border() = delete;
        Border(const Border&) = delete;
        Border(Border&&) = delete;
        ~Border() = default;
        auto operator =(const Border&) = delete;
        auto operator =(Border&&) = delete;

        /// @brief Draws the border. Called by the owning widget's Draw
        void Draw() override;

        /// @brief Sizes the border to the owning widget. Called by the widget as it is laid out
        void SetSize(const glm::vec2& size) override;

        /// @brief Places the border where the owning widget puts it. Called by the widget as it is laid out
        void SetPosition(const glm::vec3& position) override;

        /// @brief Turns the border within the owning widget's layout
        void SetRotation(const glm::vec3& rotation) override;

        /// @brief Returns the border's size
        glm::vec2 GetSize() const override;

        /// @brief Returns the border's position
        glm::vec3 GetPosition() const override;

        /// @brief Returns the border's rotation
        glm::vec3 GetRotation() const override;

        f32 GetWidth() const;

        /// @brief Has the colors and corner radii move to new values over the owning widget's transitions.
        void SetTransitionSettings(const TransitionSettings* settings);

        /// @brief Returns the drop shadow of the border, cast by its outline: GetDropShadow().Show(true). Drawn just behind it
        DropShadow& GetDropShadow();
        const DropShadow& GetDropShadow() const;

        /// @brief Returns the glow around the border: GetGlow().Show(true). It glows outward from the outer edge, and inward
        ///        from the inner edge while the widget has a gap between border and background (SetBorderGap above 0)
        Glow& GetGlow();
        const Glow& GetGlow() const;

        /// @brief Returns the light inside the outline, fading in from its edges, drawn over it: GetInnerGlow().Show(true). Set off
        ///        with SetDistance in a dark color, it is an inner shadow
        InnerGlow& GetInnerGlow();
        const InnerGlow& GetInnerGlow() const;

        /// @brief Sets whether the glow from the inner edge is drawn over the widget's background, or under it, showing in the gap between
        ///        them only. Off by default
        void EnableGlowOverBackground(const bool enable);
        bool IsGlowOverBackgroundEnabled() const;

        /// @brief Sets the offset applied to the border gradient's start position.
        void SetGradientOffset(const f32 gradientOffset);

        /// @brief Sets the speed at which the border gradient animates over time.
        void SetGradientAnimationSpeed(const f32 animationSpeed);

        /// @brief Sets the angle of the border's linear gradient, in degrees.
        void SetGradientDegree(const f32 degree);

        /// @brief Sets the speed at which the border gradient rotates over time.
        void SetGradientRotationSpeed(const f32 rotationSpeed);

        /// @brief Sets the texture ID sampled for the border's glass-effect-with-image shader feature.
        void SetWindowImageTextureID(const u32 textureID);

        /// @brief Sets the noise the glass is made of. How big its cells are, how far it shifts the window behind, its smoothing and seed
        void SetGlassNoise(const GlassNoise& noise);

        /// @brief Sets which pattern the border's raster draws. Dotted (the default), Checkered, Striped or Grid
        void SetRasterPattern(const RasterPattern rasterPattern);

        /// @brief Sets the border's raster's primary color. The dots, one of the two square colors, every other stripe, or the grid's
        ///        lines across the raster degree
        void SetPrimaryRasterColor(const glm::vec4& color);

        /// @brief Sets the border's raster's secondary color. The other square color, the stripes between the primary ones, or the
        ///        grid's lines along the raster degree
        void SetSecondaryRasterColor(const glm::vec4& color);

        /// @brief Sets the gap, in pixels, between the largest elements of the border's raster: 0 lets them touch. Spreads the grid without changing any element's size.
        void SetRasterSpacing(const f32 spacing);

        /// @brief Sets the direction, in degrees, along which the border's raster widths/animation transfer.
        void SetRasterDegree(const f32 degree);

        /// @brief Sets the per-position dot width curve, in pixels (each value is a dot's full width across), for the border's Dotted raster pattern. Enables the Raster shader feature when non-empty.
        void SetPrimaryRasterWidthTransfer(std::span<f32> widthTransfer);

        /// @brief Sets how far each border dot's opacity reaches from its center before fading to transparent.
        void SetDotTransparencyTransfer(const f32 transparencyTransfer);

        /// @brief Sets the speed at which the border's raster pattern animates.
        void SetRasterAnimationSpeed(const f32 animationSpeed);

        /// @brief Returns the border colors configured for the given color state.
        const std::vector<glm::vec4>& GetColors(const ColorState state) const;

        /// @brief Sets the border fill type (solid, linear/radial/huestar gradient).
        void SetFillType(const FillType fillType);

        /// @brief Sets the border width.
        void SetWidth(const f32 borderWidth);

        /// @brief Sets sections of the border to skip drawing; see Frame::SetBorderGaps.
        void SetGaps(std::span<BorderGap> gaps);

    private:
        /// @brief Draws the inner glow over the element as it draws now, if it is shown.
        void drawInnerGlow();

        void setColors() override;
        void setCornerShape() override;

        /// @brief Draws the drop shadow of the element as it draws now, if the shadow is shown
        void drawShadow();

        /// @brief Draws the glow outward from the outer edge, just behind the outline
        void drawGlow();

        /// @brief Draws the glow inward from the inner edge at the given depth, while there is a gap to the background
        void drawInwardGlow(const f32 depth);

        /// @brief Draws the linked background's glow over the outline, when it may cover the border
        void drawBackgroundGlow();

        std::unique_ptr<Frame> _borderFrame;
        DropShadow _dropShadow;
        std::unique_ptr<ShadowRectangle> _shadowRectangle { nullptr };
        Glow _glow;
        std::unique_ptr<GlowRectangle> _glowRectangle { nullptr };
        InnerGlow _innerGlow;
        std::unique_ptr<InnerGlowRectangle> _innerGlowRectangle { nullptr };
        static constexpr f32 _innerGlowDepthOffset { 0.0005f }; //just in front of the border, which it lies over.
        Background* _linkedBackground { nullptr }; //the widget's background, linked by IWidget::useBorderGap
        bool
            _isGlowOverBackground { false },
            _hasBackgroundGap { false }; //set by IWidget: the inward glow shows only while the gap is above 0
        static constexpr f32
            _glowDepthOffset { 0.0015f }, //behind the outline, in front of the drop shadow
            _coverDepthOffset { 0.001f }; //the background's glow, in front of the outline
        std::function<void()> _onWidthChange {};//Border.hpp, private:
        std::vector<f32> _primaryRasterWidthTransfer {};
    };
}