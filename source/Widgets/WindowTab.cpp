#include "WindowTab.hpp"
#include "Window.hpp"
#include "PlatformBridge.hpp"
#include <algorithm>
#include <cmath>

RetroFuturaGUI::WindowTab::WindowTab(std::string_view name, Projection* projection, IHierarchyNode* parentWidget, const WidgetTypeID parentWidgetTypeID, GLFWwindow* parentWindow)
    : ITab(name, projection, parentWidget, parentWidgetTypeID, parentWindow)
{
    _widgetTypeID = WidgetTypeID::WindowTab;
    _previewTitle = std::make_unique<Text>(projection);
    applyTitleFont(_previewTitle.get());

    Window* host { GetHostWindow() };

    if(!host)
        return;

    host->_windowTabs.push_back(this);
}

RetroFuturaGUI::WindowTab::~WindowTab()
{
    //Every window becomes a free one again, where the content area was. No signals: whoever connected them may be gone already.
    const glm::ivec4 content { getContentScreenRect() };
    std::vector<Field> fields { std::move(_fields) };
    _fields.clear();
    _selectedItem = nullptr;
    _pressedItem = nullptr;

    for(Field& field : fields)
    {
        Window* window { static_cast<Window*>(field._Item) };

        if(!window)
            continue;

        window->leaveTab(glm::ivec2(content.x, content.y));
    }

    Window* host { GetHostWindow() };

    if(!host)
        return;

    std::erase(host->_windowTabs, this);
}

void RetroFuturaGUI::WindowTab::Draw()
{
    syncTitles(); //again, since a widget after this WindowTab may have renamed a window
    syncFieldRects(); //again, since the pointer may have moved, closed or added a field since
    syncDropPreview();
    _background.Draw();
    _border.Draw();
    drawFields();
    drawDropPreview();
    drawSelectedWindow();
}

bool RetroFuturaGUI::WindowTab::AddWindow(Window* window)
{
    return InsertWindow(window, _fields.size());
}

bool RetroFuturaGUI::WindowTab::InsertWindow(Window* window, const uSize index)
{
    if(!window)
        return false;

    if(_kNoField != indexOf(window))
        return false;

    if(!window->enterTab(this))
        return false;

    //A window dropped on the strip takes its field from where the preview showed, and moves into its slot from there
    const uSize place { insertField(window, window->GetWindowTitle(), index) };
    _fields[place]._Shown.Jump(_previewShown.Get());
    _fields[place]._HasShown = _isPreviewShown;

    _onWindowAdded.Emit(window);
    _onWindowAddedAsync.EmitAsync(window);
    SelectWindow(window, true);
    return true;
}

bool RetroFuturaGUI::WindowTab::RemoveWindow(Window* window)
{
    if(!window)
        return false;

    const uSize index { indexOf(window) };

    if(_kNoField == index)
        return false;

    //It stays where its content was
    const glm::ivec4 content { getContentScreenRect() };

    if(eraseField(index))
        reselectAfterErase(index);

    window->leaveTab(glm::ivec2(content.x, content.y));
    _onWindowRemoved.Emit(window);
    _onWindowRemovedAsync.EmitAsync(window);
    return true;
}

void RetroFuturaGUI::WindowTab::SelectWindow(Window* window, const bool emitSignal)
{
    const uSize index { indexOf(window) };

    if(_kNoField == index)
        return;

    selectField(index, emitSignal);
}

RetroFuturaGUI::Window* RetroFuturaGUI::WindowTab::GetSelectedWindow() const
{
    return static_cast<Window*>(_selectedItem);
}

RetroFuturaGUI::Window* RetroFuturaGUI::WindowTab::GetWindow(const uSize index) const
{
    return windowOf(index);
}

uSize RetroFuturaGUI::WindowTab::GetWindowCount() const
{
    return _fields.size();
}

RetroFuturaGUI::Window* RetroFuturaGUI::WindowTab::GetHostWindow() const
{
    return Window::FromGlfwWindow(_parentWindow);
}

void RetroFuturaGUI::WindowTab::Connect_OnSelectionChanged(const typename Signal<Window*>::Slot& slot, const bool async)
{
    if(async)
        _onSelectionChangedAsync.Connect(slot);
    else
        _onSelectionChanged.Connect(slot);
}

void RetroFuturaGUI::WindowTab::Disconnect_OnSelectionChanged(const typename Signal<Window*>::Slot& slot)
{
    _onSelectionChanged.Disconnect(slot);
    _onSelectionChangedAsync.Disconnect(slot);
}

void RetroFuturaGUI::WindowTab::Connect_OnWindowAdded(const typename Signal<Window*>::Slot& slot, const bool async)
{
    if(async)
        _onWindowAddedAsync.Connect(slot);
    else
        _onWindowAdded.Connect(slot);
}

void RetroFuturaGUI::WindowTab::Disconnect_OnWindowAdded(const typename Signal<Window*>::Slot& slot)
{
    _onWindowAdded.Disconnect(slot);
    _onWindowAddedAsync.Disconnect(slot);
}

void RetroFuturaGUI::WindowTab::Connect_OnWindowRemoved(const typename Signal<Window*>::Slot& slot, const bool async)
{
    if(async)
        _onWindowRemovedAsync.Connect(slot);
    else
        _onWindowRemoved.Connect(slot);
}

void RetroFuturaGUI::WindowTab::Disconnect_OnWindowRemoved(const typename Signal<Window*>::Slot& slot)
{
    _onWindowRemoved.Disconnect(slot);
    _onWindowRemovedAsync.Disconnect(slot);
}

void RetroFuturaGUI::WindowTab::interact()
{
    _isShown = true;
    syncTitles();
    syncFieldRects(); //the pointer meets the fields where they are drawn
    syncDropPreview();
    followPointer();
    interactSelectedWindow();
}

void RetroFuturaGUI::WindowTab::interactSelectedWindow()
{
    Window* selectedWindow { GetSelectedWindow() };

    if(!selectedWindow)
        return;

    const ScreenRect area { getContentArea() };

    if(1.0f > area._Extent.x || 1.0f > area._Extent.y)
        return;

    const glm::ivec4 screen { getContentScreenRect() };
    selectedWindow->interactInHost(area, glm::ivec2(screen.x, screen.y), _contentCornerRadii); //nothing of this WindowTab is used after it: the window's widgets may take it out
}

bool RetroFuturaGUI::WindowTab::isPullingOut(const uSize index, const glm::vec2& point) const
{
    const glm::vec4 strip { getStripRect() };
    const f32 fontSize { _titleFontSize };

    //Above or below the strip. A long sideways drag raises the threshold, so reordering doesn't pull a field out by accident.
    const f32
        verticalThreshold { _pullOutDistance * fontSize + std::clamp((_sidewaysTravel - 2.0f * fontSize) * 0.2f, 0.0f, 4.0f * fontSize) }, //ImGui's numbers
        outsideStrip { std::max(strip.y - point.y, point.y - (strip.y + strip.w)) };

    if(outsideStrip >= verticalThreshold)
        return true;

    //Past the first or the last field
    const f32 pastEndThreshold { _pullOutPastEndDistance * fontSize };

    if(0 == index && slotRect(0).x - point.x > pastEndThreshold)
        return true;

    const glm::vec4 last { slotRect(_fields.size() - 1) };
    return _fields.size() - 1 == index && point.x - (last.x + last.z) > pastEndThreshold;
}

void RetroFuturaGUI::WindowTab::pullOut(const uSize index, const glm::vec2& point)
{
    Window* window { windowOf(index) };

    if(!window)
        return;

    //The cursor holds the window where it held the field, so the window's top-left lands where the field's was. Never past its
    //right edge, though, when the field was wider than the window.
    const i32 widest { std::max(window->_untabbedSize.x - 1, 0) };
    const glm::ivec2
        grabPoint { std::min(static_cast<i32>(std::lround(_grabOffset.x)), widest), static_cast<i32>(std::lround(_grabOffset.y)) },
        position { toScreen(point) - grabPoint };

    _pressedItem = nullptr;
    _isDraggingField = false;

    if(eraseField(index))
        reselectAfterErase(index);

    window->leaveTab(position);
    window->beginCursorDrag(grabPoint);
    _onWindowRemoved.Emit(window);
    _onWindowRemovedAsync.EmitAsync(window);
}

RetroFuturaGUI::Window* RetroFuturaGUI::WindowTab::windowOf(const uSize index) const
{
    if(index >= _fields.size())
        return nullptr;

    return static_cast<Window*>(_fields[index]._Item);
}

RetroFuturaGUI::ScreenRect RetroFuturaGUI::WindowTab::getContentArea() const
{
    const glm::vec4 content { getContentRect() };
    return ScreenRect { ._Origin = glm::vec2(content.x, content.y), ._Extent = glm::vec2(content.z, content.w) };
}

void RetroFuturaGUI::WindowTab::syncDropPreview()
{
    if(!_showsDropPreview)
    {
        _isPreviewShown = false;
        return;
    }

    const Transition transition { Transitions::Resolve(_transitionSettings, TransitionProperty::Position) };
    const glm::vec4 slot { slotRect(_fields.size()) };
    const glm::vec2 target { slot.x, slot.z };

    //The preview field opens from nothing where it will stand, while the others narrow to make room
    if(!_isPreviewShown)
    {
        _previewShown.Jump(glm::vec2(slot.x, 0.0f));
        _isPreviewShown = true;
    }

    if(target == _previewShown.GetTarget())
        return;

    _previewShown.Set(target, transition);
}

glm::ivec2 RetroFuturaGUI::WindowTab::toScreen(const glm::vec2& point) const
{
    //consider OPenGL-style window coordinates (bottom-left is 0,0)
    const glm::ivec2 local { static_cast<i32>(std::lround(point.x)), static_cast<i32>(std::lround(_projection.GetResolution().y - point.y)) };

    if(!_parentWindow)
        return local;

    glm::ivec2 windowPosition { 0 };
    glfwGetWindowPos(_parentWindow, &windowPosition.x, &windowPosition.y);
    return windowPosition + local;
}

void RetroFuturaGUI::WindowTab::drawSelectedWindow()
{
    Window* selectedWindow { GetSelectedWindow() };

    if(!selectedWindow)
        return;

    const ScreenRect area { getContentArea() };

    if(1.0f > area._Extent.x || 1.0f > area._Extent.y)
        return;

    const glm::ivec4 screen { getContentScreenRect() };
    selectedWindow->drawInHost(area, glm::ivec2(screen.x, screen.y), _contentCornerRadii); //nothing of this WindowTab is used after it: the window's widgets may take it out
}

void RetroFuturaGUI::WindowTab::drawDropPreview()
{
    if(!_showsDropPreview)
        return;

    glm::vec4 rect { slotRect(_fields.size()) };

    if(!fitsInStrip(rect))
        return;

    const glm::vec2 shown { _previewShown.Get() };
    rect.x = shown.x;
    rect.z = shown.y;

    //See-through, since the window isn't in yet: the selected field's look at half its opacity. Copies, since the title only points at the colors it draws
    std::vector<glm::vec4>
        background { _fieldStyle.GetBackground().GetShownColors(ColorState::Selected) },
        titleColors { _titleColors._Enabled };

    for(glm::vec4& color : background)
        color.a *= 0.5f;

    for(glm::vec4& color : titleColors)
        color.a *= 0.6f;

    const f32 z { _position.z + _zStep * 2.0f };
    _previewFieldStyle.GetBackground().SetColors(background, ColorState::Enabled);
    _previewFieldStyle.GetBackground().SetCornerRadii(_fieldStyle.GetBackground().GetCornerRadii());
    _previewFieldStyle.GetBorder().SetColors(_fieldStyle.GetBorder().GetShownColors(ColorState::Selected), ColorState::Enabled);
    _previewFieldStyle.GetBorder().SetCornerRadii(_fieldStyle.GetBorder().GetCornerRadii());
    _previewFieldStyle.GetBorder().SetWidth(_fieldStyle.GetBorder().GetWidth());
    drawFieldShape(rect, z, _previewFieldStyle, ColorState::Enabled);
    drawTitle(_previewTitle.get(), rect, z + _zStep, titleColors);
}

void RetroFuturaGUI::WindowTab::syncTitles()
{
    for(uSize index = 0; index < _fields.size(); ++index)
    {
        const Window* window { windowOf(index) };

        if(!window)
            continue;

        setFieldTitle(index, window->GetWindowTitle()); //laid out again only when it changed
    }
}

void RetroFuturaGUI::WindowTab::fieldSelected(IHierarchyNode* item, const bool emitSignal)
{
    Window* window { static_cast<Window*>(item) };

    if(!window)
        return;

    window->_ignoreHeldPress = true; //the press that selected it may still be down, and isn't a click on what lies under it
    Window::markLayoutChanged(); //the layout file keeps which tab shows

    if(!emitSignal)
        return;

    _onSelectionChanged.Emit(window);
    _onSelectionChangedAsync.EmitAsync(window);
}

void RetroFuturaGUI::WindowTab::selectionCleared()
{
    _onSelectionChanged.Emit(nullptr);
    _onSelectionChangedAsync.EmitAsync(nullptr);
}

void RetroFuturaGUI::WindowTab::closeField(const uSize index)
{
    const Window* window { windowOf(index) };

    if(!window)
        return;

    GLFWwindow* glfwWindow { window->GetGlfwWindow() };

    if(!glfwWindow)
        return;

    glfwSetWindowShouldClose(glfwWindow, GLFW_TRUE);
}

bool RetroFuturaGUI::WindowTab::dragsOutOfStrip(const uSize index, const glm::vec2& point)
{
    if(!isPullingOut(index, point))
        return false;

    pullOut(index, point);
    return true;
}

void RetroFuturaGUI::WindowTab::fieldsReordered()
{
    Window::markLayoutChanged(); //the layout file keeps the tabs' order
}

uSize RetroFuturaGUI::WindowTab::getSlotCount() const
{
    return _showsDropPreview ? _fields.size() + 1 : _fields.size();
}

void RetroFuturaGUI::WindowTab::titleFontChanged()
{
    applyTitleFont(_previewTitle.get());
}

bool RetroFuturaGUI::WindowTab::isStripUnder(const glm::ivec2& screenPoint) const
{
    if(!_isShown)
        return false;

    if(!_isEnabledFlag)
        return false;

    if(!_parentWindow)
        return false;

    glm::ivec2 windowPosition { 0 };
    glfwGetWindowPos(_parentWindow, &windowPosition.x, &windowPosition.y);
    const glm::ivec2 local { screenPoint - windowPosition };
    const glm::vec2 point { static_cast<f32>(local.x), _projection.GetResolution().y - static_cast<f32>(local.y) };
    return contains(getStripRect(), point);
}

glm::ivec4 RetroFuturaGUI::WindowTab::getContentScreenRect() const
{
    const ScreenRect area { getContentArea() };
    const glm::ivec2 topLeft { toScreen(glm::vec2(area._Origin.x, area._Origin.y + area._Extent.y)) };
    return glm::ivec4
    (
        topLeft.x,
        topLeft.y,
        std::max(static_cast<i32>(std::lround(area._Extent.x)), 1),
        std::max(static_cast<i32>(std::lround(area._Extent.y)), 1)
    );
}

void RetroFuturaGUI::WindowTab::showDropPreview(const std::string& title)
{
    _showsDropPreview = true;

    if(!_previewTitle)
        return;

    if(_previewTitle->GetTextUTF8() == title)
        return;

    _previewTitle->SetTextUTF8(title);
}

void RetroFuturaGUI::WindowTab::hideDropPreview()
{
    _showsDropPreview = false;
}
