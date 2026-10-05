#pragma once
#include "config.hpp"
#include "Packet.hpp"
#include <deque>
#include <mutex>
#include <condition_variable>

namespace RetroFuturaGUI
{
    class PacketQueue
    {
    public:
        explicit PacketQueue(const u64 byteLimit);
        PacketQueue(const PacketQueue&) = delete;
        PacketQueue(PacketQueue&&) = delete;
        ~PacketQueue() = default;
        auto operator =(const PacketQueue&) = delete;
        auto operator =(PacketQueue&&) = delete;

        void Finish();
        void Abort();
        void Clear();
        void Reset();
        bool PushPacket(Packet&& packet);
        bool PopPacket(Packet& packet);
        u64 GetByteSize() const;
        u64 GetPacketCount() const;
        bool IsAborted() const;


    private:
        std::deque<Packet> _packets {}; 
        mutable std::mutex _mutex {};
        std::condition_variable
            _notEmpty {},
            _notFull {};
        u64
            _byteLimit { 0 },
            _byteSize { 0 };
        bool
            _finish { false },
            _aborted { false };
        
    };
}