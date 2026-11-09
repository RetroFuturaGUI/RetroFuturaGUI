#ifdef RETROFUTURAGUI_MODULES
module;
#include "ModuleExternals.hpp"
module RetroFuturaGUI;
#else
#include "WidgetRegistry.hpp"
#include "IWidget.hpp"
#endif
#include <algorithm>

RetroFuturaGUI::IWidget* RetroFuturaGUI::WidgetRegistry::Find(std::string_view path)
{
    refresh();
    const auto found { _byPathHash.find(Fnv1aHash(path)) };

    if(_byPathHash.end() == found)
        return nullptr;

    for(IWidget* widget : found->second)
    {
        if(widget->GetPath() == path)
            return widget;
    }

    return nullptr;
}

RetroFuturaGUI::IWidget* RetroFuturaGUI::WidgetRegistry::Find(const u32 pathHash)
{
    refresh();
    const auto found { _byPathHash.find(pathHash) };

    if(_byPathHash.end() == found || found->second.empty())
        return nullptr;

    return found->second.front();
}

uSize RetroFuturaGUI::WidgetRegistry::GetWidgetCount()
{
    return _widgets.size();
}

void RetroFuturaGUI::WidgetRegistry::add(IWidget* widget)
{
    if(!widget)
        return;

    _widgets.push_back(widget);
    markChanged();
}

void RetroFuturaGUI::WidgetRegistry::remove(IWidget* widget)
{
    std::erase(_widgets, widget);
    markChanged();
}

void RetroFuturaGUI::WidgetRegistry::markChanged()
{
    _isChanged = true;
}

u32 RetroFuturaGUI::WidgetRegistry::getPathHash(const IWidget& widget)
{
    refresh();
    return widget._pathHash;
}

void RetroFuturaGUI::WidgetRegistry::refresh()
{
    if(!_isChanged)
        return;

    _byPathHash.clear();

    for(IWidget* widget : _widgets)
    {
        widget->_pathHash = Fnv1aHash(widget->GetPath());
        _byPathHash[widget->_pathHash].push_back(widget);
    }

    _isChanged = false;
}