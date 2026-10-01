#include "Packet.hpp"

RetroFuturaGUI::Packet::Packet() 
    : _packet(av_packet_alloc()) 
{}
        
RetroFuturaGUI::Packet::Packet(Packet&& other) noexcept
   : _packet(other._packet)
{
    other._packet = nullptr;
}

RetroFuturaGUI::Packet::~Packet()
{
    av_packet_free(&_packet); 
}

RetroFuturaGUI::Packet& RetroFuturaGUI::Packet::operator=(Packet&& other) noexcept
{
    if(this == &other)
        return *this;

    av_packet_free(&_packet);
    _packet = other._packet;
    other._packet = nullptr;

    return *this;
}

bool RetroFuturaGUI::Packet::IsValid() const
{
    return _packet != nullptr;
}

AVPacket* RetroFuturaGUI::Packet::Get() const 
{ 
    return _packet; 
}

AVPacket* RetroFuturaGUI::Packet::operator->() const 
{ 
    return _packet; 
}