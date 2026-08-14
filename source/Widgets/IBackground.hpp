#pragma once
#include "IncludeHelper.hpp"
#include <memory>
#include <span>
#include "Rectangle.hpp"
#include "Texture.hpp"

namespace RetroFuturaGUI
{
    /// @brief Mixin providing a filled/gradient background rectangle for widgets that inherit from it.
    class IBackground
    {
    public:
        /// @brief Sets a single background color for the given color state.
        void SetBackgroundColor(const glm::vec4& color, const ColorState state);

        /// @brief Sets multiple background colors (e.g. gradient stops) for the given color state.
        void SetBackgroundColors(std::span<glm::vec4> colors, const ColorState state);

        /// @brief Sets the offset applied to the background gradient's start position.
        void SetBackgroundGradientOffset(const f32 gradientOffset);

        /// @brief Sets the speed at which the background gradient animates over time.
        void SetBackgroundGradientAnimationSpeed(const f32 animationSpeed);

        /// @brief Sets the angle of the background's linear gradient, in degrees.
        void SetBackgroundGradientDegree(const f32 degree);

        /// @brief Sets the speed at which the background gradient rotates over time.
        void SetBackgroundGradientRotationSpeed(const f32 rotationSpeed);

        /// @brief Sets the texture ID sampled for the background's glass-effect-with-image shader feature.
        void SetWindowBackgroundImageTextureID(const u32 textureID);

        /// @brief Sets the dot color used for the background's DottedPattern shader feature.
        void SetBackgroundDotColor(const glm::vec4& color);

        /// @brief Sets the spacing between dot centers, in pixels, for the background's DottedPattern shader feature.
        void SetBackgroundDotDistance(const f32 distance);

        /// @brief Sets the direction, in degrees, along which the background's dot radii/animation transfer.
        void SetBackgroundDotSizeTransferDegree(const f32 degree);

        /// @brief Sets the per-position dot radius curve, in pixels, for the background's DottedPattern shader feature. Enables the feature when non-empty.
        void SetBackgroundDotRadiusTransfer(std::span<f32> radiusTransfer);

        /// @brief Sets how far each background dot's opacity reaches from its center before fading to transparent.
        void SetBackgroundDotTransparencyTransfer(const f32 transparencyTransfer);

        /// @brief Sets the speed at which the background's dotted pattern animates.
        void SetBackgroundDotAnimationSpeed(const f32 animationSpeed);

        /// @brief Loads an image from disk and applies it as the background's glass-effect-with-image texture.
        virtual void SetBackgroundImage(std::string_view imagePath);

        /// @brief Returns the background colors configured for the given color state.
        const std::vector<glm::vec4>& GetBackgroundColors(const ColorState state) const;

        /// @brief Sets the background fill type (solid, linear/radial/huestar gradient).
        void SetBackgroundFillType(const FillType fillType);

    protected:
        void setBackgroundColors();
        void drawBackground();
        void setBackgroundCornerRadii(const glm::vec4& radii);

    //Elements
        std::unique_ptr<Rectangle> _background { nullptr };
        std::unique_ptr<Texture> _backgroundImage { nullptr };

    //Style
        std::vector<glm::vec4> 
            _backgroundColorEnabled { glm::vec4(0.5f, 0.5f, 0.5f, 1.0f) },
            _backgroundColorDisabled { glm::vec4(0.25f, 0.25f, 0.25f, 1.0f) },
            _backgroundColorClicked { glm::vec4(0.6f, 0.6f,0.6f, 1.0f) },
            _backgroundColorHover { glm::vec4(0.55f, 0.55f, 0.55f, 1.0f) };
        FillType _backgroundFillType { FillType::SOLID };
        ColorState _backgroundColorState { ColorState::Enabled };
        std::vector<f32> _backgroundDotRadiusTransfer;

    private:
        void setBackgroundColorElement(std::vector<glm::vec4>& color);
    };
}