#pragma once
#include "IncludeHelper.hpp"
#include <array>
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
        CornerRadii = 1 << 1,
        Position = 1 << 2,
        Size = 1 << 3,
        Rotation = 1 << 4
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
            _CornerRadii,
            _Position,
            _Size,
            _Rotation;
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

        /// @brief Every property, one at a time, for going through a mask.
        static constexpr std::array<TransitionProperty, 5> AllProperties
        {
            TransitionProperty::Colors,
            TransitionProperty::CornerRadii,
            TransitionProperty::Position,
            TransitionProperty::Size,
            TransitionProperty::Rotation
        };

        /// @brief Sets the transition of every element and widget that has none of its own for these properties. Off (0 ms) until set.
        static void SetDefault(const TransitionProperty properties, const Transition& transition);

        /// @brief Returns the global default of one property.
        static Transition GetDefault(const TransitionProperty property);

        /// @brief Returns the widget's own transition for the property, or else the global default. None (0 ms) inside an InstantScope.
        static Transition Resolve(const TransitionSettings& settings, const TransitionProperty property);

        /// @brief Sets these properties of the settings to the transition, as a widget's SetTransition does.
        static void Override(TransitionSettings& settings, const TransitionProperty properties, const Transition& transition);

        /// @brief Has these properties of the settings follow the global default again.
        static void Reset(TransitionSettings& settings, const TransitionProperty properties);

        /// @brief Whether an InstantScope is active, in which every transition takes no time.
        static bool IsInstant();

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
        /// @brief The settings' own transition of one property.
        static std::optional<Transition>& select(TransitionSettings& settings, const TransitionProperty property);
        static const std::optional<Transition>& select(const TransitionSettings& settings, const TransitionProperty property);

        static inline TransitionSettings _defaults {}; //each property's global default, none meaning off
        static inline bool _areInstant { false }; //set by an InstantScope
    };

    /// @brief One value carried to its target over a transition, worked out from the clock whenever it is read. Works for every
    ///        type glm::mix blends: f32, glm::vec2, glm::vec3 and glm::vec4.
    template<typename T> class TransitionValue
    {
    public:
        /// @brief Moves toward a new target from the value shown now, over the transition: at once with 0 ms.
        void Set(const T& target, const Transition& transition)
        {
            const std::chrono::steady_clock::time_point now { std::chrono::steady_clock::now() };
            _from = getAt(now);
            _target = target;
            _transition = transition;
            _start = now;
        }

        /// @brief Shows the value at once, ending a running transition.
        void Jump(const T& value)
        {
            _from = value;
            _target = value;
            _transition = Transition {};
        }

        /// @brief Returns the value shown now, which is the target once the transition has arrived or inside an InstantScope.
        T Get() const
        {
            return getAt(std::chrono::steady_clock::now());
        }

        /// @brief Returns the value it is moving toward.
        const T& GetTarget() const
        {
            return _target;
        }

        /// @brief Whether it hasn't arrived at its target yet.
        bool IsRunning() const
        {
            return 1.0f > Transitions::GetProgress(_start, std::chrono::steady_clock::now(), _transition) && !Transitions::IsInstant();
        }

    private:
        T getAt(const std::chrono::steady_clock::time_point now) const
        {
            if(Transitions::IsInstant())
                return _target;

            const f32 progress { Transitions::GetProgress(_start, now, _transition) };

            if(1.0f <= progress)
                return _target;

            return glm::mix(_from, _target, progress);
        }

        T
            _from {},
            _target {};
        Transition _transition {};
        std::chrono::steady_clock::time_point _start {};
    };
}