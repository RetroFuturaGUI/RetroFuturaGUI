#pragma once
#include "IPopup.hpp"
#include "Signal.hpp"
#include <optional>
#include <tuple>
#include <type_traits>
#include <utility>

namespace RetroFuturaGUI
{
    /// @brief A popup that closes with values of the given types, e.g. Popup<bool, std::string> for a confirmation and a name.
    ///        Popup<> closes with no values, which still tells closing apart from dismissing.
    template<typename... Ts>
    class Popup final : public IPopup
    {
        static_assert((!std::is_reference_v<Ts> && ...), "A popup keeps its values until it closes, so they can't be references");

    public:
        using IPopup::IPopup;

        /// @brief Closes the popup with the given values. They go to the OnResult slots once the frame is drawn and the popup is closed
        /// @return false while another popup lies over this one
        bool Close(Ts... values)
        {
            if(!requestClose())
                return false;

            _result.emplace(std::move(values)...);
            return true;
        }

        /// @brief Closes the popup without returning values, so the OnDismiss slots fire instead. The window bar's close button does the same.
        /// @return false while another popup lies over this one
        bool Dismiss() override
        {
            if(!requestClose())
                return false;

            _result.reset();
            return true;
        }

        /// @brief Connects a slot to the values the popup closes with. Open a follow-up popup from a synchronous slot only: an
        ///        asynchronous one runs on another thread, where no GL context is current.
        void Connect_OnResult(const typename Signal<Ts...>::Slot& slot, const bool async)
        {
            if(async)
                _onResultAsync.Connect(slot);
            else
                _onResult.Connect(slot);
        }

        void Disconnect_OnResult(const typename Signal<Ts...>::Slot& slot)
        {
            _onResult.Disconnect(slot);
            _onResultAsync.Disconnect(slot);
        }

        /// @brief Connects a slot to the popup closing without values.
        void Connect_OnDismiss(const typename Signal<>::Slot& slot, const bool async)
        {
            if(async)
                _onDismissAsync.Connect(slot);
            else
                _onDismiss.Connect(slot);
        }

        void Disconnect_OnDismiss(const typename Signal<>::Slot& slot)
        {
            _onDismiss.Disconnect(slot);
            _onDismissAsync.Disconnect(slot);
        }

    private:
        std::optional<std::tuple<Ts...>> _result {}; //empty unless Close was called
        Signal<Ts...>
            _onResult,
            _onResultAsync;
        Signal<>
            _onDismiss,
            _onDismissAsync;

        void emitClosed() override
        {
            if(!_result)
            {
                _onDismissAsync.EmitAsync();
                _onDismiss.Emit();
                return;
            }

            std::apply([this](const Ts&... values)
                {
                    _onResultAsync.EmitAsync(values...);
                    _onResult.Emit(values...);
                }, *_result);
        }
    };
}