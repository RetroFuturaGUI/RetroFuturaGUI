#include "MediaFrame.hpp"

RetroFuturaGUI::MediaFrame::MediaFrame() 
    : _frame(av_frame_alloc()) 
{}
        
RetroFuturaGUI::MediaFrame::MediaFrame(MediaFrame&& other) noexcept
   : _frame(other._frame)
{
    other._frame = nullptr;
}

RetroFuturaGUI::MediaFrame::~MediaFrame()
{
    av_frame_free(&_frame); 
}

RetroFuturaGUI::MediaFrame& RetroFuturaGUI::MediaFrame::operator=(MediaFrame&& other) noexcept
{
    if(this == &other)
        return *this;

    av_frame_free(&_frame);
    _frame = other._frame;
    other._frame = nullptr;

    return *this;
}

bool RetroFuturaGUI::MediaFrame::IsValid() const
{
    return _frame != nullptr;
}

AVFrame* RetroFuturaGUI::MediaFrame::Get() const 
{ 
    return _frame; 
}

AVFrame* RetroFuturaGUI::MediaFrame::operator->() const 
{ 
    return _frame; 
}

void RetroFuturaGUI::MediaFrame::UnrefFrame() 
{ 
    if(!_frame)
        return;

    av_frame_unref(_frame);
}