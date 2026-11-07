#pragma once
#include "IncludeHelper.hpp"
#include "Plane.hpp"
#include <span>
#include <vector>

namespace RetroFuturaGUI
{
    /// @brief The highlight behind an editable text's selection. The widget owns it and hands it out through GetSelectedArea(),
    ///        so it is styled as textBox.GetSelectedArea().SetColors(...)
    class SelectedArea final
    {
    public:
        /// @brief One box of the highlight. A stretch of selected text drawn without a break
        struct LineArea
        {
            glm::vec2
                _Center { 0.0f },
                _Size { 0.0f };
        };

        explicit SelectedArea(Projection* projection);
        SelectedArea() = delete;
        SelectedArea(const SelectedArea&) = delete;
        SelectedArea(SelectedArea&&) = delete;
        ~SelectedArea() = default;
        auto operator =(const SelectedArea&) = delete;
        auto operator =(SelectedArea&&) = delete;

        /// @brief Sets the highlight color(s)
        void SetColors(std::span<const glm::vec4> colors);

        /// @brief Sets the fill type (solid, linear/radial/huestar gradient)
        void SetFillType(const FillType fillType);

        /// @brief Sets the speed at which the gradient animates over time
        void SetGradientAnimationSpeed(const f32 speed);

        /// @brief Sets the offset applied to the gradient's start position
        void SetGradientOffset(const f32 gradientOffset);

        /// @brief Sets the angle of the linear gradient, in degrees
        void SetGradientDegree(const f32 degree);

        /// @brief Sets the speed at which the gradient rotates over time
        void SetGradientRotationSpeed(const f32 rotationSpeed);

        /// @brief Sets the corner rounding radii
        void SetCornerRadii(const glm::vec4& radii);

    //Called by the owning widget
        /// @brief Draws one box per area, at the depth SetPosition gave
        void Draw(std::span<const LineArea> areas);

        /// @brief Places the highlight. Only its depth lasts: Draw places every box
        void SetPosition(const glm::vec3& position);

        /// @brief Has the colors move to new values over the widget's transitions
        void SetTransitionSettings(const TransitionSettings* settings);

    private:
        Plane _plane;
        std::vector<glm::vec4> _colors { glm::vec4(0.24f, 0.47f, 0.85f, 0.4f) };
    };
}