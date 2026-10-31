#pragma once
#include "IncludeHelper.hpp"

namespace RetroFuturaGUI
{
    /// @brief The mouse position and button states a widget reacts to. A ScrollView may hide a press or take the pointer while scrolling.
    struct PointerState
    {
        glm::vec2 _Position { 0.0f }; // Window coordinates
        bool
            _HasPosition { false },
            _LeftButtonPressed { false },
            _RightButtonPressed { false },
            _IsCaptured { false }; // A container above already uses the pointer, block clicks and hover for this widget this frame
        glm::vec2 _Wheel { 0.0f }; // Notches the wheel turned this frame: x to the right, y to the top. Parts a widget inside took are 0
        f32 _PinchScale { 1.0f }; // How far two fingers spread apart this frame, as a factor; 1 when none, or when a widget inside took it
    };

    /// @brief Parts of the frame's wheel and pinch. The widget that scrolls by one takes it (Window::TakeWheel), so the containers around it don't scroll too.
    enum class WheelParts : u32
    {
        None = 0,
        Horizontal = 1,
        Vertical = 1 << 1,
        Pinch = 1 << 2
    };

    constexpr WheelParts operator |(const WheelParts left, const WheelParts right)
    {
        return static_cast<WheelParts>(static_cast<u32>(left) | static_cast<u32>(right));
    }

    constexpr bool HasWheelPart(const WheelParts parts, const WheelParts part)
    {
        return 0 != (static_cast<u32>(parts) & static_cast<u32>(part));
    }

    /// @brief Implemented by containers that decide what their descendants see of the mouse. A ScrollView, for instance,
    ///        holds a press back until it knows whether the press is a click or the start of a scroll.
    class IPointerFilter
    {
    public:
        virtual ~IPointerFilter() = default;

        /// @brief Adjusts the mouse as this container's descendants see it. IWidget::readPointer calls it for every filtering ancestor.
        virtual void FilterPointer(PointerState& pointer) const = 0;
    };
}