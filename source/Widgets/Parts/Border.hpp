#pragma once
#include "IncludeHelper.hpp"
#include <array>
#include <memory>
#include <span>
#include "Frame.hpp"
#include "Projection.hpp"
#include "IWidgetDecoration.hpp"

namespace RetroFuturaGUI
{
    /// @brief A widget's border: a filled or gradient rectangle with optional raster, fog, glass, mask and image. The widget
    ///        owns it and hands it out through GetBorder(), so it is styled as button.GetBorder().SetColors(...)
    class Border final : public IWidgetDecoration
    {
    public:
        Border(Projection* projection);
        Border() = delete;
        Border(const Border&) = delete;
        Border(Border&&) = delete;
        ~Border() = default;
        auto operator =(const Border&) = delete;
        auto operator =(Border&&) = delete;

        /// @brief Draws the fill, then an over it. Called by the owning widget's Draw.
        void Draw() override;

        /// @brief Sizes the background to the owning widget. Called by the widget as it is laid out.
        void SetSize(const glm::vec2& size) override;

        /// @brief Places the background where the owning widget puts it. Called by the widget as it is laid out.
        void SetPosition(const glm::vec3& position) override;

        /// @brief Turns the background within the owning widget's layout.
        void SetRotation(const glm::vec3& rotation) override;

        /// @brief returns the background's size
        glm::vec2 GetSize() override;

        /// @brief returns the background's position
        glm::vec3 GetPosition() override;

        /// @brief Returns the Background's rotation
        glm::vec3 GetRotation() override;

        f32 GetWidth() const;

        /// @brief Has the colors and corner radii move to new values over the owning widget's transitions.
        void SetTransitionSettings(const TransitionSettings* settings);

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

        /// @brief Sets the dot color used for the border's Dotted raster pattern.
        void SetPrimaryRasterColor(const glm::vec4& color);

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
        const std::vector<glm::vec4>& GetColor(const ColorState state) const;

        /// @brief Sets the border fill type (solid, linear/radial/huestar gradient).
        void SetFillType(const FillType fillType);

        /// @brief Sets the border width.
        void SetWidth(const f32 borderWidth);

        /// @brief Sets sections of the border to skip drawing; see Frame::SetBorderGaps.
        void SetGaps(std::span<BorderGap> gaps);

    private:
        void draw();
        void setColors() override;
        void setCornerRadii() override;

        std::unique_ptr<Frame> _borderFrame;

    };
}