#pragma once
#include "IncludeHelper.hpp"
#include "InputManager.hpp"
#include "Signal.hpp"

namespace RetroFuturaGUI
{
    /// @brief Mixin providing click/hover signals for widgets that inherit from it.
    class IClickable
    {
    public:
        /// @brief Connects a slot to be called when the widget is clicked.
        /// @param async If true, the slot is invoked asynchronously.
        void Connect_OnClick(const typename Signal<>::Slot& slot, const bool async);

        /// @brief Connects a slot to be called when the mouse button is released over the widget.
        /// @param async If true, the slot is invoked asynchronously.
        void Connect_OnRelease(const typename Signal<>::Slot& slot, const bool async);

        /// @brief Connects a slot to be called every frame while the widget is hovered.
        /// @param async If true, the slot is invoked asynchronously.
        void Connect_WhileHover(const typename Signal<>::Slot& slot, const bool async);

        /// @brief Connects a slot to be called when the mouse cursor enters the widget.
        /// @param async If true, the slot is invoked asynchronously.
        void Connect_OnMouseEnter(const typename Signal<>::Slot& slot, const bool async);

        /// @brief Connects a slot to be called when the mouse cursor leaves the widget.
        /// @param async If true, the slot is invoked asynchronously.
        void Connect_OnMouseLeave(const typename Signal<>::Slot& slot, const bool async);

        /// @brief Disconnects a previously connected OnClick slot.
        void Disconnect_OnClick(const typename Signal<>::Slot& slot);

        /// @brief Disconnects a previously connected OnRelease slot.
        void Disconnect_OnRelease(const typename Signal<>::Slot& slot);

        /// @brief Disconnects a previously connected WhileHover slot.
        void Disconnect_WhileHover(const typename Signal<>::Slot& slot);

        /// @brief Disconnects a previously connected OnMouseEnter slot.
        void Disconnect_OnMouseEnter(const typename Signal<>::Slot& slot);

        /// @brief Disconnects a previously connected OnMouseLeave slot.
        void Disconnect_OnMouseLeave(const typename Signal<>::Slot& slot);

    protected:
        //logic
        Signal<> 
            _onClick,
            _onClickAsync,
            _onRelease,
            _onReleaseAsync,
            _whileHover,
            _whileHoverAsync,
            _onMouseEnter,
            _onMouseEnterAsync,
            _onMouseLeave,
            _onMouseLeaveAsync;
        bool 
            _wasClicked { false },
            _mouseEnteredFlag { false },
            _mouseLeftFlag { false };
    };
}