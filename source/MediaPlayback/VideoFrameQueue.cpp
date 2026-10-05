#include "VideoFrameQueue.hpp"

RetroFuturaGUI::VideoFrameQueue::VideoFrameQueue(const uSize capacity)
    : _capacity(capacity)
{}

bool RetroFuturaGUI::VideoFrameQueue::Push(VideoFrame&& frame)
{
    if(!frame._Frame.IsValid())
        return false;

    std::unique_lock lock(_mutex);
    _notFull.wait(lock, [this] { return _aborted || _frames.size() < _capacity; });

    if(_aborted)
        return false;

    _frames.push_back(std::move(frame));
    return true;
}

bool RetroFuturaGUI::VideoFrameQueue::TryPop(VideoFrame& frame)
{
    {
        std::lock_guard lock(_mutex);

        if(_frames.empty())
            return false;

        frame = std::move(_frames.front());
        _frames.pop_front();
    }

    _notFull.notify_one();
    return true;
}

std::optional<i64> RetroFuturaGUI::VideoFrameQueue::PeekPosition() const
{
    std::lock_guard lock(_mutex);

    if(_frames.empty())
        return std::nullopt;

    return _frames.front()._Position;
}

void RetroFuturaGUI::VideoFrameQueue::Abort()
{
    {
        std::lock_guard lock(_mutex);
        _aborted = true;
    }

    _notFull.notify_all();
}

void RetroFuturaGUI::VideoFrameQueue::Reset()
{
    std::lock_guard lock(_mutex);
    _frames.clear();
    _aborted = false;
}

uSize RetroFuturaGUI::VideoFrameQueue::GetFrameCount() const
{
    std::lock_guard lock(_mutex);
    return _frames.size();
}