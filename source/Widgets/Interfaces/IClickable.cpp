#include "IClickable.hpp"

void RetroFuturaGUI::IClickable::Connect_OnClick(const typename Signal<>::Slot &slot, const bool async)
{
    if (async)
        _onClickAsync.Connect(slot);
    else
        _onClick.Connect(slot); 
}

void RetroFuturaGUI::IClickable::Connect_OnRelease(const typename Signal<>::Slot &slot, const bool async)
{
    if (async)
        _onReleaseAsync.Connect(slot);
    else 
        _onRelease.Connect(slot);
}

void RetroFuturaGUI::IClickable::Connect_WhileHover(const typename Signal<>::Slot &slot, const bool async)
{
    if (async)
        _whileHoverAsync.Connect(slot);
    else
        _whileHover.Connect(slot);
}

void RetroFuturaGUI::IClickable::Connect_OnMouseEnter(const typename Signal<>::Slot &slot, const bool async)
{
    if (async)
        _onMouseEnterAsync.Connect(slot);
    else
        _onMouseEnter.Connect(slot);
}

void RetroFuturaGUI::IClickable::Connect_OnMouseLeave(const typename Signal<>::Slot &slot, const bool async)
{
    if (async)
        _onMouseLeaveAsync.Connect(slot);
    else
        _onMouseLeave.Connect(slot);
}

void RetroFuturaGUI::IClickable::Disconnect_OnClick(const typename Signal<>::Slot &slot)
{
    _onClick.Disconnect(slot);
    _onClickAsync.Disconnect(slot);
}

void RetroFuturaGUI::IClickable::Disconnect_OnRelease(const typename Signal<>::Slot &slot)
{
    _onRelease.Disconnect(slot);
    _onReleaseAsync.Disconnect(slot);
}

void RetroFuturaGUI::IClickable::Disconnect_WhileHover(const typename Signal<>::Slot &slot)
{
    _whileHover.Disconnect(slot);
    _whileHoverAsync.Disconnect(slot);
}

void RetroFuturaGUI::IClickable::Disconnect_OnMouseEnter(const typename Signal<>::Slot &slot)
{
    _onMouseEnter.Disconnect(slot);
    _onMouseEnterAsync.Disconnect(slot);
}

void RetroFuturaGUI::IClickable::Disconnect_OnMouseLeave(const typename Signal<>::Slot &slot)
{
    _onMouseLeave.Disconnect(slot);
    _onMouseLeaveAsync.Disconnect(slot);
}