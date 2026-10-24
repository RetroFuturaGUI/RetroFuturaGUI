#include "FileUndoHistory.hpp"

void RetroFuturaGUI::FileUndoHistory::Record(Step&& step)
{
    if(step._Items.empty())
        return;

    if(_stepLimit <= _steps.size())
        _steps.erase(_steps.begin());

    _steps.push_back(std::move(step));
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