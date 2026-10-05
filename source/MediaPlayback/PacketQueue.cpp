#include "PacketQueue.hpp"
#include <algorithm>

RetroFuturaGUI::PacketQueue::PacketQueue(const u64 byteLimit)
    : _byteLimit(byteLimit)
{}

void RetroFuturaGUI::PacketQueue::Finish()
{
    {
        std::lock_guard lock(_mutex);
        _finish = true;
    }

    _notEmpty.notify_all();
}

void RetroFuturaGUI::PacketQueue::Abort()
{
    {
        std::lock_guard lock(_mutex);
        _aborted = true;
    }

    _notFull.notify_all();
    _notEmpty.notify_all();
}

void RetroFuturaGUI::PacketQueue::Clear()
{
    {
        std::lock_guard lock(_mutex);
        _packets.clear();
        _byteSize = 0;
        _finish = false;
    }

    _notFull.notify_all();
}

void RetroFuturaGUI::PacketQueue::Reset()
{
    std::lock_guard lock(_mutex);
    _packets.clear();
    _byteSize = 0;
    _finish = false;
    _aborted = false;
}

bool RetroFuturaGUI::PacketQueue::PushPacket(Packet&& packet)
{
    if(!packet.IsValid())
        return false;

    {
        std::unique_lock lock(_mutex);
        _notFull.wait(lock, [this] { return _aborted || _byteSize < _byteLimit || _packets.empty(); });

        if(_aborted)
            return false;

        _byteSize += static_cast<u64>(packet->size);
        _packets.push_back(std::move(packet));
    }

    _notEmpty.notify_one();
    return true;
}

bool RetroFuturaGUI::PacketQueue::PopPacket(Packet& packet)
{
    {
        std::unique_lock lock(_mutex);

        _notEmpty.wait(lock, [this] { return _aborted || !_packets.empty() || _finish; });

        if(_aborted)
            return false;

        // Woken but still empty can only mean finished: the end of the stream
        if(_packets.empty())
            return false;

        packet = std::move(_packets.front());
        _packets.pop_front();
        _byteSize -= static_cast<u64>(packet->size);
    }

    _notFull.notify_one();
    return true;
}

u64 RetroFuturaGUI::PacketQueue::GetByteSize() const
{
    std::lock_guard lock(_mutex);
    return _byteSize;
}

u64 RetroFuturaGUI::PacketQueue::GetPacketCount() const
{
    std::lock_guard lock(_mutex);
    return _packets.size();
}

bool RetroFuturaGUI::PacketQueue::IsAborted() const
{
    std::lock_guard lock(_mutex);
    return _aborted;
}