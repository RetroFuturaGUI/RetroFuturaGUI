#pragma once
#include "IncludeHelper.hpp"
#include <chrono>
#include <optional>
#include <span>
#include <vector>

namespace RetroFuturaGUI
{
    /// @brief How a transition speeds up and slows down on its way to the target.
    enum class Easing : u32
    {
        Linear,
        EaseIn,
        EaseOut,
        EaseInOut
    };

    /// @brief How long a value takes to reach a new target, and along which curve. A duration of 0 switches at once.
    struct Transition
    {
        std::chrono::milliseconds _Duration { 0 };
        Easing _Easing { Easing::EaseInOut };
    };

    /// @brief The values a transition carries, as a mask: SetTransition(Colors | CornerRadii, transition)
    enum class TransitionProperty : u32
    {
        None = 0,
        Colors = 1,
        CornerRadii = 1 << 1
    };

    constexpr TransitionProperty operator |(const TransitionProperty left, const TransitionProperty right)
    {
        return static_cast<TransitionProperty>(static_cast<u32>(left) | static_cast<u32>(right));
    }

    constexpr bool HasTransitionProperty(const TransitionProperty properties, const TransitionProperty property)
    {
        return 0 != (static_cast<u32>(properties) & static_cast<u32>(property));
    }

    /// @brief A widget's own transitions. A property it has none for follows the global default (Transitions::SetDefault).
    struct TransitionSettings
    {
        std::optional<Transition>
            _Colors,
            _CornerRadii;
    };

    /// @brief The global transition defaults, and the arithmetic of a running transition. A transition is worked out from the
    ///        clock whenever its value is drawn, so it runs the same at any frame rate and costs nothing once it has arrived.
    class Transitions
    {
    public:
        /// @brief While one with active set lives, every transition takes no time. A window draws under one after it was hidden,
        ///        so it shows at once whatever changed meanwhile instead of animating it into view.
        class InstantScope
        {
        public:
            InstantScope(const bool active);
            InstantScope(const InstantScope&) = delete;
            InstantScope(InstantScope&&) = delete;
            auto operator =(const InstantScope&) = delete;
            auto operator =(InstantScope&&) = delete;
            ~InstantScope();

        private:
            bool _wasInstant { false };
        };

        /// @brief Sets the transition of every element whose widget has none of its own for these properties. Off (0 ms) until set.
        static void SetDefault(const TransitionProperty properties, const Transition& transition);

        /// @brief Returns the global default of one property.
        static Transition GetDefault(const TransitionProperty property);

        /// @brief Returns the widget's own transition for the property, or else the global default. None (0 ms) inside an InstantScope.
        static Transition Resolve(const TransitionSettings& settings, const TransitionProperty property);

        /// @brief Returns how far a transition started at start has come by now, eased: 0 at the start, 1 once arrived.
        static f32 GetProgress(const std::chrono::steady_clock::time_point start, const std::chrono::steady_clock::time_point now, const Transition& transition);

        /// @brief Bends linear progress (0 to 1) along the easing curve. The curves are cubic.
        static f32 Ease(const f32 progress, const Easing easing);

        /// @brief Blends two colors with their alpha premultiplied, so a fade from transparent doesn't pass through a dark fringe.
        static glm::vec4 BlendColor(const glm::vec4& from, const glm::vec4& to, const f32 amount);

        /// @brief Samples the colors at stopCount evenly spaced stops, wrapping around the way the gradient shader does, so two
        ///        gradients with different stop counts can blend stop by stop.
        static void ResampleStops(std::span<const glm::vec4> colors, const uSize stopCount, std::vector<glm::vec4>& stops);

    private:
        static inline Transition
            _defaultColors {},
            _defaultCornerRadii {};
        static inline bool _areInstant { false }; //set by an InstantScope
    };
}