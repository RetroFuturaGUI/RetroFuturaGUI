#pragma once
#include <functional>
#include <vector>
#include <mutex>
#include <thread>
#include <algorithm>

namespace RetroFuturaGUI
{
    template <typename... Args>
    class Signal 
    {
    public:
        using Slot = std::function<void(Args...)>;

        Signal() = default;
        
        void Connect(const Slot& slot)
        {
            std::lock_guard<std::mutex> lock(_mutex);
            _slots.push_back(slot);
        }

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

                        return s.target<void(*)(Args...)>() == slot.target<void(*)(Args...)>();
                    }
                ),
                _slots.end()
            );
        }

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