#pragma once
#include <functional>
#include <vector>
#include <mutex>
#include <thread>
#include <algorithm>

namespace RetroFuturaGUI
{
    /// @brief A thread-safe observer-pattern signal: holds a list of slots that can be invoked together via Emit/EmitAsync.
    template <typename... Args>
    class Signal
    {
    public:
        using Slot = std::function<void(Args...)>;

        Signal() = default;

        /// @brief Adds a slot to be called on Emit/EmitAsync.
        void Connect(const Slot& slot)
        {
            std::lock_guard<std::mutex> lock(_mutex);
            _slots.push_back(slot);
        }

        /// @brief Removes a previously connected slot, matched by target function pointer.
        void Disconnect(const Slot& slot)
        {
            std::lock_guard<std::mutex> lock(_mutex);
            _slots.erase(
                std::remove_if(
                    _slots.begin(),
                    _slots.end(),
                    [&slot](const Slot& s) // make this also work with lambdas and functors if needed
                    {
                        if (!s || !slot)
                            return false;

                        return s.template target<void(*)(Args...)>() == slot.template target<void(*)(Args...)>();
                    }
                ),
                _slots.end()
            );
        }

        /// @brief Calls all connected slots synchronously, in connection order, with the given arguments.
        void Emit(Args... args)
        {
            std::vector<Slot> slotsCopy;
            {
                std::lock_guard<std::mutex> lock(_mutex);
                slotsCopy = _slots;
            }

            for (const auto& slot : slotsCopy)
                slot(args...);
        }

        /// @brief Calls all connected slots, each on its own detached thread, with the given arguments.
        void EmitAsync(Args... args)
        {
            std::vector<Slot> slotsCopy;
            {
                std::lock_guard<std::mutex> lock(_mutex);
                slotsCopy = _slots;
            }

            for (const auto& slot : slotsCopy)
            {
                std::thread([slot, args...]() 
                {
                    slot(args...);
                }).detach();
            }
        }

    private:
        std::vector<Slot> _slots;
        std::mutex _mutex;
    };
}