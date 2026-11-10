#pragma once
#include "IncludeHelper.hpp"
#include "Plane.hpp"
#include "StyledPart.hpp"
#include "ShaderLayers.hpp"
#include <chrono>
#include <span>
#include <vector>

namespace RetroFuturaGUI
{
    /// @brief The caret of an editable text. The widget owns it and hands it out through GetCaret(), so it is styled as
    ///        textBox.GetCaret().SetColors(...)
    class Caret final : public IStyledPart
    {
    public:
        explicit Caret(Projection* projection);
        Caret() = delete;
        Caret(const Caret&) = delete;
        Caret(Caret&&) = delete;
        ~Caret() = default;
        auto operator =(const Caret&) = delete;
        auto operator =(Caret&&) = delete;

        /// @brief Sets the caret color(s), over every flavor. None leave them to the widget's flavors and the built-in ones
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

        /// @brief Sets how the caret moves when its place in the text changes (typing, clicks, arrow keys, insert mode): it
        ///        glides there over the transition instead of jumping. Every move takes the transition's duration, however far it
        ///        goes, so a line change or a jump across the text takes no longer than a step. Off (0 ms) by default. When the
        ///        widget itself moves, the caret moves with it at once
        void SetMoveTransition(const Transition& transition);
        const Transition& GetMoveTransition() const;

        /// @brief Leaves a fill setting to the widget's flavors and the built-in look (colors: set none instead)
        void ResetSetting(const StyleSetting setting);

    //Called by the owning widget
        /// @brief Draws the caret
        void Draw();

        /// @brief Sizes the caret to the text it edits
        void SetSize(const glm::vec2& size);

        /// @brief Places the caret in the text at once, as when the widget moves
        void SetPosition(const glm::vec3& position);

        /// @brief Moves the caret to a new place and size in the text, gliding over the move transition. The first move jumps
        void MoveTo(const glm::vec3& position, const glm::vec2& size);

        /// @brief Has the colors move to new values over the widget's transitions
        void SetTransitionSettings(const TransitionSettings* settings);

    private:
        void restyle() override;

        /// @brief Hands the plane the colors and the fill settings that changed through the layers
        void applyStyle();

        Plane _plane;
        LayeredColors _colors {}; //its colors are its Enabled ones
        LayeredSetting<FillType> _fillTypeLayer {};
        LayeredSetting<f32>
            _gradientAnimationSpeedLayer {};
        f64 _blinkTime { 650.0 };
        bool _neverBlinks { false };

    //Gliding
        Transition _moveTransition {};
        TransitionValue<glm::vec3> _position;
        TransitionValue<glm::vec2> _size;
        bool
            _hasPlace { false }, //moved once: the first move jumps
            _isMoving { false };
        std::chrono::steady_clock::time_point _lastMoveTime {}; //when the caret last moved, so moves coming faster than the transition keep up
    };
}