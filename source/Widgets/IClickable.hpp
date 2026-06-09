#pragma once
#include "IncludeHelper.hpp"
#include "InputManager.hpp"
#include "Signal.hpp"

namespace RetroFuturaGUI
{
    class IClickable
    {
    public:
        void Connect_OnClick(const typename Signal<>::Slot& slot, const bool async);
        void Connect_OnRelease(const typename Signal<>::Slot& slot, const bool async);
        void Connect_WhileHover(const typename Signal<>::Slot& slot, const bool async);
        void Connect_OnMouseEnter(const typename Signal<>::Slot& slot, const bool async);
        void Connect_OnMouseLeave(const typename Signal<>::Slot& slot, const bool async);
        void Disconnect_OnClick(const typename Signal<>::Slot& slot);
        void Disconnect_OnRelease(const typename Signal<>::Slot& slot);
        void Disconnect_WhileHover(const typename Signal<>::Slot& slot);
        void Disconnect_OnMouseEnter(const typename Signal<>::Slot& slot);
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