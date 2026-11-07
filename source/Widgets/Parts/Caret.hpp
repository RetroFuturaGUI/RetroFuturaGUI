#pragma once
#include "IncludeHelper.hpp"
#include "Plane.hpp"
#include <span>
#include <vector>

namespace RetroFuturaGUI
{
    /// @brief The caret of an editable text. The widget owns it and hands it out through GetCaret(), so it is styled as
    ///        textBox.GetCaret().SetColors(...)
    class Caret final
    {
    public:
        explicit Caret(Projection* projection);
        Caret() = delete;
        Caret(const Caret&) = delete;
        Caret(Caret&&) = delete;
        ~Caret() = default;
        auto operator =(const Caret&) = delete;
        auto operator =(Caret&&) = delete;

        /// @brief Sets the caret color(s)
        void SetColors(std::span<const glm::vec4> colors);

        /// @brief Sets the fill type (solid, linear/radial/huestar gradient)
        void SetFillType(const FillType fillType);

        /// @brief Sets the speed at which the gradient animates over time
        void SetGradientAnimationSpeed(const f32 speed);

        /// @brief Sets how long, in milliseconds, the caret stays visible/hidden per blink cycle. At or below one frame it stays solid, since a blink couldn't be seen
        void SetBlinkTime(const f64 milliseconds);

        /// @brief Returns how long, in milliseconds, the caret stays visible/hidden per blink cycle
        f64 GetBlinkTime() const;

        /// @brief Returns whether the caret stays solid
        bool NeverBlinks() const;

    //Called by the owning widget
        /// @brief Draws the caret
        void Draw();

        /// @brief Sizes the caret to the text it edits
        void SetSize(const glm::vec2& size);

        /// @brief Places the caret in the text
        void SetPosition(const glm::vec3& position);

        /// @brief Has the colors move to new values over the widget's transitions
        void SetTransitionSettings(const TransitionSettings* settings);

    private:
        Plane _plane;
        std::vector<glm::vec4> _colors { glm::vec4(1.0f) };
        f64 _blinkTime { 650.0 };
        bool _neverBlinks { false };
    };
}