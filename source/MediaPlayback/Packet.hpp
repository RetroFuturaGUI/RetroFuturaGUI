#pragma once
#include "MediaSource.hpp"

namespace RetroFuturaGUI
{
/// @brief Owns an AVPacket for its whole lifetime.
    class Packet
    {
    public:
        Packet();
        Packet(const Packet&) = delete;
        Packet(Packet&& other) noexcept;
        ~Packet();
        Packet& operator=(const Packet&) = delete;
        Packet& operator=(Packet&& other) noexcept;

        /// @brief Allocation can fail, so check this once after construction.
        bool IsValid() const;
        AVPacket* operator->() const;
        AVPacket* Get() const;

    private:
        AVPacket* _packet { nullptr };
    };    
}