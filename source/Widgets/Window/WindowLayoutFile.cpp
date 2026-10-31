#include "Window.hpp"
#include "WindowTab.hpp"
#include "PlatformBridge.hpp"
#include <algorithm>
#include <charconv>
#include <filesystem>
#include <format>
#include <fstream>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

void RetroFuturaGUI::Window::SetLayoutFile(std::string_view path)
{
    _layoutFile = path;
}

void RetroFuturaGUI::Window::SaveLayout()
{
    _isLayoutChanged = false;

    if(_layoutFile.empty())
        return; //switched off

    const std::u8string path(_layoutFile.begin(), _layoutFile.end()); //the name is UTF-8
    std::ofstream file { std::filesystem::path(path), std::ios::binary | std::ios::trunc };

    if(!file)
        return;

    file << writeLayout();
}

void RetroFuturaGUI::Window::loadLayoutOnce()
{
    if(_isLayoutLoaded)
        return;

    _isLayoutLoaded = true;

    if(_layoutFile.empty())
        return;

    const std::u8string path(_layoutFile.begin(), _layoutFile.end());
    std::ifstream file { std::filesystem::path(path), std::ios::binary };

    //Not there yet: it starts out with the layout the code set up
    if(!file)
    {
        SaveLayout();
        return;
    }

    std::stringstream text;
    text << file.rdbuf();
    PlatformBridge::Monitors::Refresh(); //monitors may have come or gone since the program started
    applyLayout(text.str());
}

std::string RetroFuturaGUI::Window::writeLayout()
{
    static constexpr std::string_view edgeNames[] { "Top", "Right", "Bottom", "Left" };
    std::string text { "; RetroFuturaGUI's window layout, written on its own. Windows are found again by name, WindowTabs by path.\n" };

    for(const Window* window : getAllWindows())
    {
        if(!window)
            continue;

        if(!window->_window)
            continue;

        //While maximized, minimized or in a tab its own rect isn't on screen, so the last one it had there is kept
        const bool
            isMaximized { window->IsMaximized() },
            isNormal { !isMaximized && GLFW_FALSE == glfwGetWindowAttrib(window->_window, GLFW_ICONIFIED) && !window->IsInTab() };
        const DockRect current { window->getScreenRect() };
        const glm::ivec2
            position { isNormal ? current._Position : window->_normalPosition },
            size { isNormal ? current._Size : window->_normalSize };

        text += std::format("\n[Window][{}]\nPosition={},{}\nSize={},{}\nMaximized={}\nUndockedLength={}\nUntabbedSize={},{}\n",
                            window->GetName(), position.x, position.y, size.x, size.y, isMaximized ? 1 : 0, window->_undockedLength,
                            window->_untabbedSize.x, window->_untabbedSize.y);

        const Window* host { window->_dockHost };

        if(!host)
            continue;

        const std::list<Window*>& onEdge { host->dockedWindows(window->_dockHostEdge) };
        const uSize index { static_cast<uSize>(std::distance(onEdge.begin(), std::find(onEdge.begin(), onEdge.end(), window))) };
        text += std::format("DockHost={}\nDockEdge={}\nDockIndex={}\n", host->GetName(), edgeNames[static_cast<uSize>(window->_dockHostEdge) % 4], index);
    }

    for(const Window* window : getAllWindows())
    {
        if(!window)
            continue;

        for(const WindowTab* tab : window->_windowTabs)
        {
            if(!tab)
                continue;

            text += std::format("\n[WindowTab][{}]\n", tab->GetPath());

            for(uSize index = 0; index < tab->GetWindowCount(); ++index)
            {
                const Window* tabbed { tab->GetWindow(index) };

                if(!tabbed)
                    continue;

                text += std::format("Window={}\n", tabbed->GetName());
            }

            if(!tab->GetSelectedWindow())
                continue;

            text += std::format("Selected={}\n", tab->GetSelectedWindow()->GetName());
        }
    }

    return text;
}

void RetroFuturaGUI::Window::applyLayout(const std::string& text)
{
    struct SavedWindow
    {
        std::string
            _Name {},
            _DockHost {};
        glm::ivec2
            _Position { 0 },
            _Size { 0 },
            _UntabbedSize { 0 };
        i32 _UndockedLength { 0 };
        DockEdge _DockEdge { DockEdge::Top };
        uSize _DockIndex { 0 };
        bool
            _HasRect { false },
            _IsMaximized { false };
    };

    struct SavedWindowTab
    {
        std::string
            _Path {},
            _Selected {};
        std::vector<std::string> _Windows {};
    };

    std::vector<SavedWindow> savedWindows;
    std::vector<SavedWindowTab> savedTabs;

    const auto readInt = [](std::string_view value) -> i32
    {
        i32 number { 0 };
        std::from_chars(value.data(), value.data() + value.size(), number);
        return number;
    };

    const auto readPair = [&readInt](std::string_view value) -> glm::ivec2
    {
        const uSize comma { value.find(',') };

        if(std::string_view::npos == comma)
            return glm::ivec2(0);

        return glm::ivec2(readInt(value.substr(0, comma)), readInt(value.substr(comma + 1)));
    };

    //Sections start with [Window][name] or [WindowTab][path]; lines starting with ; are comments
    std::istringstream lines { text };
    std::string line;
    bool isInWindowTab { false };

    while(std::getline(lines, line))
    {
        if(!line.empty() && '\r' == line.back())
            line.pop_back();

        if(line.empty() || ';' == line.front())
            continue;

        constexpr std::string_view
            windowHeader { "[Window][" },
            windowTabHeader { "[WindowTab][" };

        if(line.starts_with(windowHeader) && line.ends_with(']'))
        {
            savedWindows.push_back(SavedWindow
            {
                ._Name = line.substr(windowHeader.size(), line.size() - windowHeader.size() - 1),
                ._DockHost = {},
                ._Position = glm::ivec2(0),
                ._Size = glm::ivec2(0),
                ._UntabbedSize = glm::ivec2(0),
                ._UndockedLength = 0,
                ._DockEdge = DockEdge::Top,
                ._DockIndex = 0,
                ._HasRect = false,
                ._IsMaximized = false
            });
            isInWindowTab = false;
            continue;
        }

        if(line.starts_with(windowTabHeader) && line.ends_with(']'))
        {
            savedTabs.push_back(SavedWindowTab
            {
                ._Path = line.substr(windowTabHeader.size(), line.size() - windowTabHeader.size() - 1),
                ._Selected = {},
                ._Windows = {}
            });
            isInWindowTab = true;
            continue;
        }

        const uSize equals { line.find('=') };

        if(std::string::npos == equals)
            continue;

        const std::string_view
            key { std::string_view(line).substr(0, equals) },
            value { std::string_view(line).substr(equals + 1) };

        if(isInWindowTab && !savedTabs.empty())
        {
            if("Window" == key)
                savedTabs.back()._Windows.emplace_back(value);
            else if("Selected" == key)
                savedTabs.back()._Selected = value;

            continue;
        }

        if(savedWindows.empty())
            continue;

        SavedWindow& saved { savedWindows.back() };

        if("Position" == key)
        {
            saved._Position = readPair(value);
            saved._HasRect = true;
        }
        else if("Size" == key)
            saved._Size = readPair(value);
        else if("Maximized" == key)
            saved._IsMaximized = 0 != readInt(value);
        else if("UndockedLength" == key)
            saved._UndockedLength = readInt(value);
        else if("UntabbedSize" == key)
            saved._UntabbedSize = readPair(value);
        else if("DockHost" == key)
            saved._DockHost = value;
        else if("DockIndex" == key)
            saved._DockIndex = static_cast<uSize>(std::max(readInt(value), 0));
        else if("DockEdge" == key)
            saved._DockEdge = "Right" == value ? DockEdge::Right : "Bottom" == value ? DockEdge::Bottom : "Left" == value ? DockEdge::Left : DockEdge::Top;
    }

    const auto isInSavedTab = [&savedTabs](std::string_view name)
    {
        return std::ranges::any_of(savedTabs, [name](const SavedWindowTab& tab) { return std::ranges::find(tab._Windows, name) != tab._Windows.end(); });
    };

    //1. Out of the tabs and docking groups the file doesn't put them in, then each free window's size and place
    for(const SavedWindow& saved : savedWindows)
    {
        Window* window { findWindow(saved._Name) };

        if(!window)
            continue;

        if(window->_tabHost && !isInSavedTab(saved._Name))
            window->_tabHost->RemoveWindow(window);

        if(window->_dockHost && (saved._DockHost != window->_dockHost->GetName() || saved._DockEdge != window->_dockHostEdge))
            window->leaveDockHost();

        if(!saved._HasRect || 0 >= saved._Size.x || 0 >= saved._Size.y || window->IsInTab())
            continue;

        const glm::ivec4 rect { keepOnMonitors(glm::ivec4(saved._Position, saved._Size)) };
        window->SetWindowSize(rect.z, rect.w);
        glfwSetWindowPos(window->_window, rect.x, rect.y);
        window->_normalPosition = glm::ivec2(rect.x, rect.y);
        window->_normalSize = glm::ivec2(rect.z, rect.w);
    }

    //2. Docking, each edge in its saved order
    std::vector<const SavedWindow*> docked;

    for(const SavedWindow& saved : savedWindows)
    {
        if(saved._DockHost.empty())
            continue;

        docked.push_back(&saved);
    }

    std::ranges::stable_sort(docked, [](const SavedWindow* left, const SavedWindow* right) { return left->_DockIndex < right->_DockIndex; });

    for(const SavedWindow* saved : docked)
    {
        Window* window { findWindow(saved->_Name) };
        Window* host { findWindow(saved->_DockHost) };

        if(!window || !host)
            continue;

        if(window->_dockHost != host)
            host->dockWindowAt(window, saved->_DockEdge, saved->_DockIndex);

        if(0 < saved->_UndockedLength)
            window->_undockedLength = saved->_UndockedLength;
    }

    //3. Tabs in their saved order with the saved one selected
    for(const SavedWindowTab& savedTab : savedTabs)
    {
        WindowTab* tab { findWindowTab(savedTab._Path) };

        if(!tab)
            continue;

        uSize place { 0 };

        for(const std::string& name : savedTab._Windows)
        {
            Window* window { findWindow(name) };

            if(!window)
                continue;

            if(window->_tabHost && window->_tabHost != tab)
                window->_tabHost->RemoveWindow(window);

            window->leaveDockHost();

            if(tab->indexOf(window) != place)
            {
                if(window->_tabHost == tab)
                    tab->RemoveWindow(window);

                tab->InsertWindow(window, place);
            }

            ++place;
        }

        Window* selected { findWindow(savedTab._Selected) };

        if(!selected)
            continue;

        tab->SelectWindow(selected, true);
    }

    //4. The size before tabbing, which going into a tab just took from the window, and maximized last, so restoring gives back the size and place set above
    for(const SavedWindow& saved : savedWindows)
    {
        Window* window { findWindow(saved._Name) };

        if(!window)
            continue;

        if(0 < saved._UntabbedSize.x && 0 < saved._UntabbedSize.y)
            window->_untabbedSize = saved._UntabbedSize;

        if(!saved._IsMaximized || window->IsInTab() || window->IsMaximized())
            continue;

        glfwMaximizeWindow(window->_window);
    }
}

void RetroFuturaGUI::Window::markLayoutChanged()
{
    _isLayoutChanged = true;
}

void RetroFuturaGUI::Window::saveLayoutIfChanged()
{
    if(!_isLayoutChanged)
        return;

    SaveLayout();
}

void RetroFuturaGUI::Window::watchLayout()
{
    if(!_window)
        return;

    if(WindowRole::Normal != _role)
        return;

    if(IsInTab())
        return; //its own rect isn't on screen, the WindowTab's is

    const bool
        isMaximized { IsMaximized() },
        isMinimized { GLFW_TRUE == glfwGetWindowAttrib(_window, GLFW_ICONIFIED) };

    if(isMaximized != _wasMaximized || isMinimized != _wasMinimized)
    {
        _wasMaximized = isMaximized;
        _wasMinimized = isMinimized;
        markLayoutChanged();
    }

    if(isMaximized || isMinimized)
        return; //the rect it had before stays the one kept

    const DockRect rect { getScreenRect() };

    if(rect._Position == _normalPosition && rect._Size == _normalSize)
        return;

    if(PlatformBridge::Input::IsMouseButtonDown(PlatformBridge::MouseButton::Left))
        return; //a move or resize still going on, kept once the button is up

    _normalPosition = rect._Position;
    _normalSize = rect._Size;
    markLayoutChanged();
}

glm::ivec4 RetroFuturaGUI::Window::keepOnMonitors(const glm::ivec4& rect)
{
    const std::vector<PlatformBridge::MonitorInfo>& monitors { PlatformBridge::Monitors::GetMonitors() };

    if(monitors.empty())
        return rect;

    //Enough of its top on a monitor's work area to grab it there by its bar
    for(const PlatformBridge::MonitorInfo& monitor : monitors)
    {
        const i32
            overlapWidth { std::min(rect.x + rect.z, monitor._WorkX + monitor._WorkWidth) - std::max(rect.x, monitor._WorkX) },
            overlapHeight { std::min(rect.y + _grabStripHeight, monitor._WorkY + monitor._WorkHeight) - std::max(rect.y, monitor._WorkY) };

        if(_grabStripMinimumWidth <= overlapWidth && _grabStripHeight / 2 <= overlapHeight)
            return rect;
    }

    const PlatformBridge::MonitorInfo* primary { PlatformBridge::Monitors::GetPrimaryMonitor() };

    if(!primary)
        return rect;

    //Centered on the primary monitor's work area, and no larger than it
    const i32
        width { std::min(rect.z, primary->_WorkWidth) },
        height { std::min(rect.w, primary->_WorkHeight) };
    return glm::ivec4(primary->_WorkX + (primary->_WorkWidth - width) / 2, primary->_WorkY + (primary->_WorkHeight - height) / 2, width, height);
}

RetroFuturaGUI::Window* RetroFuturaGUI::Window::findWindow(std::string_view name)
{
    if(name.empty())
        return nullptr;

    for(Window* window : getAllWindows())
    {
        if(!window)
            continue;

        if(window->GetName() == name)
            return window;
    }

    return nullptr;
}

RetroFuturaGUI::WindowTab* RetroFuturaGUI::Window::findWindowTab(std::string_view path)
{
    for(Window* window : getAllWindows())
    {
        if(!window)
            continue;

        for(WindowTab* tab : window->_windowTabs)
        {
            if(!tab)
                continue;

            if(tab->GetPath() == path)
                return tab;
        }
    }

    return nullptr;
}