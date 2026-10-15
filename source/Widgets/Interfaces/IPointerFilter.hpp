#pragma once
#include "IncludeHelper.hpp"

namespace RetroFuturaGUI
{
    /// @brief The mouse position and left button state a widget reacts to. A ScrollView may hide a press while it decides between click and scroll, or take the pointer away while scrolling
    struct PointerState
    {
        glm::vec2 _Position { 0.0f }; // Window coordinates
        bool
            _HasPosition { false },
            _LeftButtonPressed { false },
            _IsCaptured { false }; // A container above already uses the pointer, block clicks and hover for this widget this frame
    };

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
