#include "Frame.hpp"

RetroFuturaGUI::Frame::Frame() 
    : _frame(av_frame_alloc()) 
{}
        
RetroFuturaGUI::Frame::Frame(Frame&& other) noexcept
   : _frame(other._frame)
{
    other._frame = nullptr;
}

RetroFuturaGUI::Frame::~Frame()
{
    av_frame_free(&_frame); 
}

RetroFuturaGUI::Frame& RetroFuturaGUI::Frame::operator=(Frame&& other) noexcept
{
    if(this == &other)
        return *this;

    av_frame_free(&_frame);
    _frame = other._frame;
    other._frame = nullptr;

    return *this;
}

bool RetroFuturaGUI::Frame::IsValid() const
{
    return _frame != nullptr;
}

AVFrame* RetroFuturaGUI::Frame::Get() const 
{ 
    return _frame; 
}

AVFrame* RetroFuturaGUI::Frame::operator->() const 
{ 
    return _frame; 
}

void RetroFuturaGUI::Frame::UnrefFrame() 
{ 
    if(_frame)
        av_frame_unref(_frame);
}