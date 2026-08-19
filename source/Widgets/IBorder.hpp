#pragma once
#include "IncludeHelper.hpp"
#include <memory>
#include <span>
#include "Rectangle.hpp"

namespace RetroFuturaGUI
{
    /// @brief Mixin providing a filled/gradient border rectangle for widgets that inherit from it.
    class IBorder
    {
    public:
        /// @brief Sets a single border color for the given color state.
        void SetBorderColor(const glm::vec4& color, const ColorState state);

        /// @brief Sets multiple border colors (e.g. gradient stops) for the given color state.
        void SetBorderColors(std::span<glm::vec4> colors, const ColorState state);

        /// @brief Sets the offset applied to the border gradient's start position.
        void SetBorderGradientOffset(const f32 gradientOffset);

        /// @brief Sets the speed at which the border gradient animates over time.
        void SetBorderGradientAnimationSpeed(const f32 animationSpeed);

        /// @brief Sets the angle of the border's linear gradient, in degrees.
        void SetBorderGradientDegree(const f32 degree);

        /// @brief Sets the speed at which the border gradient rotates over time.
        void SetBorderGradientRotationSpeed(const f32 rotationSpeed);

        /// @brief Sets the texture ID sampled for the border's glass-effect-with-image shader feature.
        void SetWindowBorderImageTextureID(const u32 textureID);

        /// @brief Sets the dot color used for the border's DottedPattern shader feature.
        void SetBorderDotColor(const glm::vec4& color);

        /// @brief Sets the spacing between dot centers, in pixels, for the border's DottedPattern shader feature.
        void SetBorderDotDistance(const f32 distance);

        /// @brief Sets the direction, in degrees, along which the border's dot radii/animation transfer.
        void SetBorderDotSizeTransferDegree(const f32 degree);

        /// @brief Sets the per-position dot radius curve, in pixels, for the border's DottedPattern shader feature. Enables the feature when non-empty.
        void SetBorderDotRadiusTransfer(std::span<f32> radiusTransfer);

        /// @brief Sets how far each border dot's opacity reaches from its center before fading to transparent.
        void SetBorderDotTransparencyTransfer(const f32 transparencyTransfer);

        /// @brief Sets the speed at which the border's dotted pattern animates.
        void SetBorderDotAnimationSpeed(const f32 animationSpeed);

        /// @brief Returns the border colors configured for the given color state.
        const std::vector<glm::vec4>& GetBorderColor(const ColorState state) const;

        /// @brief Sets the border fill type (solid, linear/radial/huestar gradient).
        void SetBorderFillType(const FillType fillType);

        /// @brief Sets the border width.
        void SetBorderWidth(const f32 borderWidth);

        /// @brief Sets sections of the border to skip drawing; see Rectangle::SetBorderGaps.
        void SetBorderGaps(std::span<BorderGap> gaps);

    protected:
        void drawBorder();
        void setBorderColors();
        void setBorderCornerRadii(const glm::vec4& radii);

        std::unique_ptr<Rectangle> _border;
        FillType _borderFillType { FillType::SOLID };
        ColorState _borderColorState { ColorState::Enabled };

        std::vector<glm::vec4> 
            _borderColorEnabled { glm::vec4(0.25f, 0.25f, 0.25f, 1.0f) },
            _borderColorDisabled { glm::vec4(0.2f, 0.2f, 0.2f, 1.0f) },
            _borderColorClicked { glm::vec4(0.3f, 0.3f, 1.3f, 1.0f) },
            _borderColorHover { glm::vec4(0.275f, 0.275f, 0.275f, 1.0f) };

    private:
        void setBorderColorElement(std::vector<glm::vec4>& color);
    };
}