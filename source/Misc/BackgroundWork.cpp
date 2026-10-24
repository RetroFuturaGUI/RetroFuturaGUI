#include "BackgroundWork.hpp"

RetroFuturaGUI::BackgroundWork::BackgroundWork(const WhenDestroyed whenDestroyed)
    : _whenDestroyed(whenDestroyed),
      _worker([state = _state](std::stop_token stop) { run(*state, stop); })
{
}

RetroFuturaGUI::BackgroundWork::~BackgroundWork()
{
    _worker.request_stop();

    if(WhenDestroyed::WaitForJob == _whenDestroyed)
        return; //_worker's own destructor joins

    {
        //Nothing that waits drains it anymore, and the queued jobs needn't run
        std::scoped_lock lock { _state->_Mutex };
        _state->_Jobs.clear();
        _state->_Finished.clear();
    }

    _worker.detach();
}

void RetroFuturaGUI::BackgroundWork::queue(Job&& job)
{
    {
        std::scoped_lock lock { _state->_Mutex };
        _state->_Jobs.push_back(std::move(job));
    }

    _state->_Wake.notify_one();
}

void RetroFuturaGUI::BackgroundWork::run(State& state, std::stop_token stop)
{
    while(!stop.stop_requested())
    {
        Job job {};

        {
            std::unique_lock lock { state._Mutex };

            if(!state._Wake.wait(lock, stop, [&state]() { return !state._Jobs.empty(); }))
                return; //stopped while waiting

            job = std::move(state._Jobs.front());
            state._Jobs.pop_front();
            state._IsRunningJob = true;
        }

        Finish finish { job(stop) };
        std::scoped_lock lock { state._Mutex };
        state._IsRunningJob = false;

        if(stop.stop_requested())
            return;

        state._Finished.push_back(std::move(finish));
    }
}

void RetroFuturaGUI::BackgroundWork::Drain()
{
    std::vector<Finish> ready {};

    {
        std::scoped_lock lock { _state->_Mutex };
        ready.swap(_state->_Finished);
    }

    //Outside the lock, since a slot may post the next job
    for(Finish& finish : ready)
    {
        if(!finish)
            continue;

        finish();
    }
}

void RetroFuturaGUI::BackgroundWork::ClearPending()
{
    std::scoped_lock lock { _state->_Mutex };
    _state->_Jobs.clear();
    _state->_Finished.clear();
}

bool RetroFuturaGUI::BackgroundWork::IsBusy() const
{
    std::scoped_lock lock { _state->_Mutex };
    return _state->_IsRunningJob || !_state->_Jobs.empty() || !_state->_Finished.empty();
}