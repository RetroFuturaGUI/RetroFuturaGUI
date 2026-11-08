#include "MenuBar.hpp"
#include "Window.hpp"
#include "PlatformBridge.hpp"

#if defined(TARGET_PLATFORM_LINUX)
    #define GLFW_EXPOSE_NATIVE_X11
#elif defined(TARGET_PLATFORM_WINDOWS)
    #define GLFW_EXPOSE_NATIVE_WIN32
#endif
#include <GLFW/glfw3native.h>

RetroFuturaGUI::MenuBar::MenuBar(std::string_view name, Projection* projection, IWidget* parentWidget, const WidgetTypeID parentWidgetTypeID, GLFWwindow* parentWindow)
    : IWidget(name, projection, parentWidget, parentWidgetTypeID, parentWindow)
{
    _widgetTypeID = WidgetTypeID::MenuBar;
    //The item look draws once per item, in its colors, so it switches at once and takes no transitions
    _background.SetTransitionSettings(&_transitionSettings);
    _border.SetTransitionSettings(&_transitionSettings);
    useBorderGap(&_background, &_border);

    //Items show nothing behind their widgets until the app gives them colors.
    for(const ColorState state : { ColorState::Enabled, ColorState::Disabled, ColorState::Hover, ColorState::Clicked })
    {
        _itemStyle.GetBackground().SetColors(std::array { glm::vec4(0.0f) }, state);
        _itemStyle.GetBorder().SetColors(std::array { glm::vec4(0.0f) }, state);
    }
}

void RetroFuturaGUI::MenuBar::Draw()
{
    _background.Draw();
    _border.Draw();
    drawSeparatorLines();
    drawItems();
}

void RetroFuturaGUI::MenuBar::applyPosition(const glm::vec3& position)
{
    IWidget::applyPosition(position);
    _background.SetPosition(position);
    _border.SetPosition(position + glm::vec3(0.0f, 0.0f, 0.01f));

    placeItems();
}

void RetroFuturaGUI::MenuBar::applySize(const glm::vec3& size)
{
    IWidget::applySize(size);
    _background.SetSize(size);
    _border.SetSize(size);
    updateLayout();
}

void RetroFuturaGUI::MenuBar::SetMargin(const f32 margin)
{
    _margin = margin;
    updateLayout();
}

void RetroFuturaGUI::MenuBar::SetDockingEdge(const DockingEdge dockingEdge)
{
    _dockingEdge = dockingEdge;
    updateLayout();
}

void RetroFuturaGUI::MenuBar::SetEdgeAlignment(const EdgeAlignment edgeAlignment)
{
    _edgeAlignment = edgeAlignment;
    updateLayout();
}

void RetroFuturaGUI::MenuBar::SetFlexCount(const uSize count)
{
    releaseCellsFrom(count);
    _itemFlex.resize(count);

    //Cells added here have no policy of their own so they take an even share. Cells that already had one keep it. An additional call on SetFlexDefinition readjusts all
    _flexDefinition.resize(count, _kEvenShare);

    if(_hoveredItemIndex >= _itemFlex.size())
        _hoveredItemIndex = _kNoItem;

    placeItems();
}

void RetroFuturaGUI::MenuBar::SetFlexDefinition(std::span<FlexDefinition> flexDefinitions)
{
    _flexDefinition.assign(flexDefinitions.begin(), flexDefinitions.end());

    //The definitions express how many cells there are, so the flex follows them
    releaseCellsFrom(_flexDefinition.size());
    _itemFlex.resize(_flexDefinition.size());

    if(_hoveredItemIndex >= _itemFlex.size())
        _hoveredItemIndex = _kNoItem;

    placeItems();
}

const RetroFuturaGUI::MenuBar::FlexDefinition& RetroFuturaGUI::MenuBar::GetFlexDefinition(const uSize index) const
{
    if(index >= _flexDefinition.size())
        return _kEvenShare;

    return _flexDefinition[index];
}

bool RetroFuturaGUI::MenuBar::RemoveWidget(const uSize index)
{
    if(index >= _itemFlex.size())
        return false;

    std::unique_ptr<IWidget>& cell { *std::next(_itemFlex.begin(), static_cast<i64>(index)) };

    if(!cell)
        return false;

    //Kept alive until the next Interact, since one of its own slots may be running right now!!
    _removedWidgets.push_back(std::move(cell));
    return true;
}

bool RetroFuturaGUI::MenuBar::isNameTaken(std::string_view name) const
{
    for(const std::unique_ptr<IWidget>& item : _itemFlex)
        if(item && name == item->GetName())
            return true;

    return false;
}

void RetroFuturaGUI::MenuBar::releaseCellsFrom(const uSize count)
{
    if(count >= _itemFlex.size())
        return;

    for(auto iterator { std::next(_itemFlex.begin(), static_cast<i64>(count)) }; iterator != _itemFlex.end(); ++iterator)
        if(*iterator)
            _removedWidgets.push_back(std::move(*iterator));
}

void RetroFuturaGUI::MenuBar::ShowSeparatorLines(const bool show)
{
    _useSeparatorLines = show;
}

void RetroFuturaGUI::MenuBar::SetSeparatorLinesThickness(const f32 thickness)
{
    _separatorLinesThickness = thickness;
}

//Item border

void RetroFuturaGUI::MenuBar::interact()
{
    _removedWidgets.clear(); //no slot of theirs must run
    syncUndockCells();
    followPointer();

    for(const std::unique_ptr<IWidget>& item : _itemFlex)
    {
        if(!item)
            continue;

        item->Interact();
    }
}

void RetroFuturaGUI::MenuBar::update()
{
    for(const std::unique_ptr<IWidget>& item : _itemFlex)
    {
        if(!item)
            continue;

        item->Update();
    }
}

void RetroFuturaGUI::MenuBar::followPointer()
{
    const PointerState pointer { readPointer() };

    if(pointer._IsCaptured)
        return; //ScrollView above has taken the pointer

    glm::vec2 mousePos { pointer._Position };
    bool
        isMouseButtonPressed { pointer._LeftButtonPressed },
        isMouseInside { pointer._HasPosition && isPointInside(mousePos) };

    updateWindowDrag(MouseState
    {
        ._WorldPoint = mousePos,
        ._WindowPoint = glm::i32vec2(static_cast<i32>(mousePos.x), static_cast<i32>(_projection.GetResolution().y - mousePos.y)), //undo the bottom-up flip, the window moves in top-down coordinates
        ._HasPosition = pointer._HasPosition,
        ._IsPressed = isMouseButtonPressed
    });

    if(!_isEnabledFlag || !isMouseInside) //no action and mouse leave
    {
        _hoveredItemIndex = _kNoItem;

        if(_mouseEnteredFlag)
        {
            _mouseEnteredFlag = false;
            _onMouseLeaveAsync.EmitAsync();
            _onMouseLeave.Emit();
        }

        _wasClicked = isMouseButtonPressed;
        return;
    }

    _hoveredItemIndex = _kNoItem;

    for(uSize index { 0 }; index < _itemFlex.size(); ++index)
    {
        if(!isPointInsideRect(mousePos, glm::vec3(cellSize(index), _size.z), cellPosition(index), glm::vec3(0.0f)))
            continue;

        _hoveredItemIndex = index;
        break;
    }

    _whileHoverAsync.EmitAsync();
    _whileHover.Emit();

    if(!_mouseLeftFlag && !_mouseEnteredFlag) //enter
    {
        _mouseEnteredFlag = true;
        _onMouseEnterAsync.EmitAsync();
        _onMouseEnter.Emit();
    }

    if(isMouseButtonPressed && !_wasClicked) //click
    {
        _onClickAsync.EmitAsync();
        _onClick.Emit();
    }
    else if(!isMouseButtonPressed && _wasClicked) //release
    {
        _onReleaseAsync.EmitAsync();
        _onRelease.Emit();
    }

    _wasClicked = isMouseButtonPressed;
}

void RetroFuturaGUI::MenuBar::updateWindowDrag(const MouseState& mouse)
{
    if(!_parentWindow)
        return;

    const bool pressedThisFrame { mouse._IsPressed && !_wasDragMousePressed };
    _wasDragMousePressed = mouse._IsPressed;

    if(!mouse._IsPressed)
    {   //dock window where its preview showed
        if(_isDraggingWindow)
            endParentWindowDrag();

        _isDraggingWindow = false;
        return;
    }

    if(!mouse._HasPosition)
        return;

    if(pressedThisFrame)
    {
        const bool
            isDragCell { isPointInsideDragCell(mouse._WorldPoint) },
            isUndockCell { isPointInsideUndockCell(mouse._WorldPoint) };

        //Only the press that lands on a drag or undock cell begins one
        if(!isDragCell && !isUndockCell)
            return;

        //From an undock cell the window leaves its docking group first, so the drag moves it alone instead of the whole group
        if(isUndockCell)
            undockParentWindow();

        _isDraggingWindow = true;
        _dragGrabPoint = mouse._WindowPoint;
        beginParentWindowDrag();
        return;
    }

    if(!_isDraggingWindow)
        return;

    // How far the cursor has moved from the point it took hold of
    const glm::i32vec2 slip { mouse._WindowPoint - _dragGrabPoint };

    if(0 == slip.x && 0 == slip.y)
        return;

    i32 windowX { 0 }, windowY { 0 };
    glfwGetWindowPos(_parentWindow, &windowX, &windowY);
    glfwSetWindowPos(_parentWindow, windowX + slip.x, windowY + slip.y);
}

bool RetroFuturaGUI::MenuBar::isPointInsideDragCell(const glm::vec2& point) const
{
    uSize index { 0 };

    for(const std::unique_ptr<IWidget>& item : _itemFlex)
    {
        const uSize cell { index };
        ++index;

        if(!GetFlexDefinition(cell)._IsWindowDrag)
            continue;

        //The flag only counts on a Label, so a cell holding something clickable keeps its clicks.
        if(!dynamic_cast<Label*>(item.get()))
            continue;

        if(!isPointInsideRect(point, glm::vec3(cellSize(cell), _size.z), cellPosition(cell), glm::vec3(0.0f)))
            continue;

        return true;
    }

    return false;
}

bool RetroFuturaGUI::MenuBar::isPointInsideUndockCell(const glm::vec2& point) const
{
    uSize index { 0 };

    for(const std::unique_ptr<IWidget>& item : _itemFlex)
    {
        const uSize cell { index };
        ++index;

        if(!GetFlexDefinition(cell)._IsWindowUndock)
            continue;

        //An empty cell has nothing to grab, a clickable widget keeps its clicks, and a disabled one has nothing to undock
        if(!item)
            continue;

        if(dynamic_cast<const IClickable*>(item.get()))
            continue;

        if(!item->IsEnabled())
            continue;

        if(!isPointInsideRect(point, glm::vec3(cellSize(cell), _size.z), cellPosition(cell), glm::vec3(0.0f)))
            continue;

        return true;
    }

    return false;
}

void RetroFuturaGUI::MenuBar::undockParentWindow() const
{
    Window* window { Window::FromGlfwWindow(_parentWindow) };

    if(!window)
        return;

    window->Undock();
}

void RetroFuturaGUI::MenuBar::syncUndockCells()
{
    Window* window { Window::FromGlfwWindow(_parentWindow) };

    if(!window)
        return;

    //A window docked to nothing has nothing to be pulled out of
    const bool isDocked { window->IsDocked() };
    uSize index { 0 };

    for(const std::unique_ptr<IWidget>& item : _itemFlex)
    {
        const uSize cell { index };
        ++index;

        if(!item)
            continue;

        if(!GetFlexDefinition(cell)._IsWindowUndock)
            continue;

        if(item->IsEnabled() == isDocked)
            continue;

        item->SetEnabled(isDocked, false);
    }
}

bool RetroFuturaGUI::MenuBar::isItemDisabled(const uSize index) const
{
    if(index >= _itemFlex.size())
        return false;

    const IWidget* item { std::next(_itemFlex.begin(), static_cast<i64>(index))->get() };

    if(!item)
        return false;

    return !item->IsEnabled();
}

void RetroFuturaGUI::MenuBar::beginParentWindowDrag() const
{
    Window* window { Window::FromGlfwWindow(_parentWindow) };

    if(!window)
        return;

    window->BeginDrag();
}

void RetroFuturaGUI::MenuBar::endParentWindowDrag() const
{
    Window* window { Window::FromGlfwWindow(_parentWindow) };

    if(!window)
        return;

    window->EndDrag();
}

void RetroFuturaGUI::MenuBar::updateLayout()
{
    SetPosition(calculateDockedPosition());
}

void RetroFuturaGUI::MenuBar::placeItems()
{
    const Transitions::InstantScope instant { true }; //a container lays its widgets out at once, whatever transitions they have
    resolveCellSizes();

    uSize index { 0 };

    for(const std::unique_ptr<IWidget>& item : _itemFlex)
    {
        const glm::vec3 center { cellPosition(index) };
        ++index;

        if(!item)
            continue;

        item->SetPosition(glm::vec3(center.x, center.y, center.z + 0.02f));
    }
}

f32 RetroFuturaGUI::MenuBar::alignAlongEdge(const f32 edgeLength, const f32 barLength) const
{
    f32 center { edgeLength * 0.5f };

    switch(_edgeAlignment)
    {
        case EdgeAlignment::Start:
            center = _margin + barLength * 0.5f;
        break;
        case EdgeAlignment::Center:
            center = edgeLength * 0.5f;
        break;
        case EdgeAlignment::End:
            center = edgeLength - _margin - barLength * 0.5f;
        break;
        default: [[unlikely]]
        break;
    }

    return center;
}

glm::vec3 RetroFuturaGUI::MenuBar::calculateDockedPosition() const
{
    const glm::vec2 resolution { _projection.GetResolution() };
    f32
        x { resolution.x * 0.5f },
        y { resolution.y * 0.5f };

    switch(_dockingEdge)
    {
        case DockingEdge::Left:
            x = _margin + _size.x * 0.5f;
            y = alignAlongEdge(resolution.y, _size.y);
        break;
        case DockingEdge::Right:
            x = resolution.x - _margin - _size.x * 0.5f;
            y = alignAlongEdge(resolution.y, _size.y);
        break;
        case DockingEdge::Top:
            x = alignAlongEdge(resolution.x, _size.x);
            y = resolution.y - _margin - _size.y * 0.5f;
        break;
        case DockingEdge::Bottom:
            x = alignAlongEdge(resolution.x, _size.x);
            y = _margin + _size.y * 0.5f;
        break;
        default:
            [[unlikely]]
        break;
    }

    return glm::vec3(x, y, _position.z);
}

bool RetroFuturaGUI::MenuBar::isHorizontal() const
{
    return _dockingEdge == DockingEdge::Top || _dockingEdge == DockingEdge::Bottom;
}

void RetroFuturaGUI::MenuBar::resolveCellSizes()
{
    _resolvedCellSizes.assign(_itemFlex.size(), 0.0f);

    if(_itemFlex.empty())
        return;

    const bool horizontal { isHorizontal() };
    const f32
        lengthExtent { horizontal ? _size.x : _size.y },   // along the bar
        thicknessExtent { horizontal ? _size.y : _size.x };// across it

    f32
        sizedTotal { 0.0f },
        starWeightTotal { 0.0f };
    uSize index { 0 };

    //First pass: every cell that knows its own extent claims it, and the Star weights are counted up.
    for(const std::unique_ptr<IWidget>& item : _itemFlex)
    {
        const FlexDefinition& definition { GetFlexDefinition(index) };
        const f32 value { 0.0f < definition._Width ? definition._Width : 0.0f };

        switch(definition._FlexSizing)
        {
            case FlexSizing::Star:
                starWeightTotal += value;
            break;
            case FlexSizing::Fixed:
                _resolvedCellSizes[index] = value;
            break;
            case FlexSizing::Auto:
                _resolvedCellSizes[index] = item ? (horizontal ? item->GetSize().x : item->GetSize().y) : 0.0f;
            break;
            case FlexSizing::Square:
                _resolvedCellSizes[index] = thicknessExtent;
            break;
            default:
                [[unlikely]]
            break;
        }

        sizedTotal += _resolvedCellSizes[index];
        ++index;
    }

    const f32 leftover { lengthExtent > sizedTotal ? lengthExtent - sizedTotal : 0.0f };

    for(index = 0; index < _resolvedCellSizes.size(); ++index)
    {
        const FlexDefinition& definition { GetFlexDefinition(index) };

        if(definition._FlexSizing != FlexSizing::Star)
            continue;

        const f32 value { 0.0f < definition._Width ? definition._Width : 0.0f };
        _resolvedCellSizes[index] = 0.0f < starWeightTotal ? leftover * (value / starWeightTotal) : 0.0f;
    }
}

glm::vec2 RetroFuturaGUI::MenuBar::cellSize(const uSize index) const
{
    if(index >= _resolvedCellSizes.size())
        return glm::vec2(0.0f);

    if(isHorizontal())
        return glm::vec2(_resolvedCellSizes[index], _size.y);

    return glm::vec2(_size.x, _resolvedCellSizes[index]);
}

f32 RetroFuturaGUI::MenuBar::cellOffset(const uSize index) const
{
    f32 offset { 0.0f };

    for(uSize cell { 0 }; cell < index && cell < _resolvedCellSizes.size(); ++cell)
        offset += _resolvedCellSizes[cell];

    return offset;
}

glm::vec3 RetroFuturaGUI::MenuBar::cellPosition(const uSize index) const
{
    if(index >= _resolvedCellSizes.size())
        return _position;

    const f32
        offset { cellOffset(index) },
        extent { _resolvedCellSizes[index] };

    if(isHorizontal())
        return glm::vec3(_position.x - _size.x * 0.5f + offset + extent * 0.5f, _position.y, _position.z);

    return glm::vec3(_position.x, _position.y + _size.y * 0.5f - offset - extent * 0.5f, _position.z);
}

RetroFuturaGUI::ColorState RetroFuturaGUI::MenuBar::itemState(const uSize index) const
{
    if(!_isEnabledFlag)
        return ColorState::Disabled;

    if(isItemDisabled(index))
        return ColorState::Disabled;

    if(index != _hoveredItemIndex)
        return ColorState::Enabled;

    if(_wasClicked)
        return ColorState::Clicked;

    return ColorState::Hover;
}

void RetroFuturaGUI::MenuBar::drawItems()
{
    uSize index { 0 };

    for(const std::unique_ptr<IWidget>& item : _itemFlex)
    {
        const glm::vec3 center { cellPosition(index) };
        const glm::vec2 cell { cellSize(index) };
        _itemStyle.SetColorState(itemState(index));
        ++index;

        _itemStyle.Place(cell, center);
        _itemStyle.Draw();

        if(!item)
            continue;

        item->DrawAtOpacity();
    }
}

void RetroFuturaGUI::MenuBar::drawSeparatorLines()
{
    if(!_useSeparatorLines)
        return;

    if(2 > _itemFlex.size())
        return;

    const glm::vec2 lineSize { isHorizontal()
        ? glm::vec2(_separatorLinesThickness, _size.y)
        : glm::vec2(_size.x, _separatorLinesThickness) };

    _separatorLine.SetSize(lineSize);

    // A line goes on every seam between two cells. one fewer than there are cells
    for(uSize seam { 1 }; seam < _itemFlex.size(); ++seam)
    {
        //The seam is the near edge of this cell, which is where the previous one ended.
        const f32 offset { cellOffset(seam) };
        const glm::vec3 seamCenter { isHorizontal()
            ? glm::vec3(_position.x - _size.x * 0.5f + offset, _position.y, _position.z)
            : glm::vec3(_position.x, _position.y + _size.y * 0.5f - offset, _position.z) };

        _separatorLine.SetPosition(seamCenter + glm::vec3(0.0f, 0.0f, 0.015f));
        _separatorLine.Draw();
    }
}

RetroFuturaGUI::Box& RetroFuturaGUI::MenuBar::GetItemStyle()
{
    return _itemStyle;
}

const RetroFuturaGUI::Box& RetroFuturaGUI::MenuBar::GetItemStyle() const
{
    return _itemStyle;
}

RetroFuturaGUI::Background& RetroFuturaGUI::MenuBar::GetSeparatorLine()
{
    return _separatorLine;
}

const RetroFuturaGUI::Background& RetroFuturaGUI::MenuBar::GetSeparatorLine() const
{
    return _separatorLine;
}

RetroFuturaGUI::Background& RetroFuturaGUI::MenuBar::GetBackground()
{
    return _background;
}

const RetroFuturaGUI::Background& RetroFuturaGUI::MenuBar::GetBackground() const
{
    return _background;
}

RetroFuturaGUI::Border& RetroFuturaGUI::MenuBar::GetBorder()
{
    return _border;
}

const RetroFuturaGUI::Border& RetroFuturaGUI::MenuBar::GetBorder() const
{
    return _border;
}