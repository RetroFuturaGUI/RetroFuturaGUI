#include "FileUndoHistory.hpp"

void RetroFuturaGUI::FileUndoHistory::Record(Step&& step)
{
    if(step._Items.empty())
        return;

    _undoneSteps.clear();
    RecordAgain(std::move(step));
}

std::optional<RetroFuturaGUI::FileUndoHistory::Step> RetroFuturaGUI::FileUndoHistory::TakeLast()
{
    if(_steps.empty())
        return std::nullopt;

    std::optional<Step> step { std::move(_steps.back()) };
    _steps.pop_back();
    return step;
}

bool RetroFuturaGUI::FileUndoHistory::HasSteps()
{
    return !_steps.empty();
}

void RetroFuturaGUI::FileUndoHistory::KeepUndone(Step&& step)
{
    if(step._Items.empty())
        return;

    if(_stepLimit <= _undoneSteps.size())
        _undoneSteps.erase(_undoneSteps.begin());

    _undoneSteps.push_back(std::move(step));
}

std::optional<RetroFuturaGUI::FileUndoHistory::Step> RetroFuturaGUI::FileUndoHistory::TakeLastUndone()
{
    if(_undoneSteps.empty())
        return std::nullopt;

    std::optional<Step> step { std::move(_undoneSteps.back()) };
    _undoneSteps.pop_back();
    return step;
}

bool RetroFuturaGUI::FileUndoHistory::HasUndoneSteps()
{
    return !_undoneSteps.empty();
}

void RetroFuturaGUI::FileUndoHistory::RecordAgain(Step&& step)
{
    if(step._Items.empty())
        return;

    if(_stepLimit <= _steps.size())
        _steps.erase(_steps.begin());

    _steps.push_back(std::move(step));
}