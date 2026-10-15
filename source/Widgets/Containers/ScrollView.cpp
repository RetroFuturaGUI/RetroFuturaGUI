#include "ScrollView.hpp"
#include "IWidget.hpp"
#include "IncludeHelper.hpp"
#include "Rectangle.hpp"
#include <memory>
#include <algorithm>
#include <cmath>
#include <utility>

#if defined(TARGET_PLATFORM_LINUX)
    #define GLFW_EXPOSE_NATIVE_X11
#elif defined(TARGET_PLATFORM_WINDOWS)
    #define GLFW_EXPOSE_NATIVE_WIN32
#endif
#include <GLFW/glfw3native.h>

RetroFuturaGUI::ScrollView::ScrollView(std::string_view name, Projection* projection, IHierarchyNode* parentWidget, const WidgetTypeID parentWidgetTypeID, GLFWwindow* parentWindow)
    : IWidget(name, projection, parentWidget, parentWidgetTypeID, parentWindow)
{
    _widgetTypeID = WidgetTypeID::ScrollView;

}

void RetroFuturaGUI::ScrollView::Draw()
{
    performPendingRemoval();
    interact();

    for(ScrollViewCell& cell : _items)
    {
        // A cell removed during this loop, by the slot of a widget drawn earlier, is skipped but stays alive until the next frame.
        if(cell._PendingRemoval)
            continue;

        if(!cell._Widget)
            continue;

        cell._Widget->Draw();
    }
}

bool RetroFuturaGUI::ScrollView::RemoveWidget(std::string_view name)
{
    ScrollViewCell* cell { findCell(name) };

    if(!cell)
        return false;

    // A widget mustn't be destroyed right now since one or more of its slot might be running 
    cell->_PendingRemoval = true;
    _hasPendingRemovals = true;
    layoutCells();
    return true;
}

void RetroFuturaGUI::ScrollView::adoptWidget(std::unique_ptr<IWidget> widget)
{
    if(!widget)
        return;

    const glm::vec3 size { widget->GetSize() };
    _items.push_back(ScrollViewCell { ._Position = glm::vec3(0.0f), ._Size = size, ._Widget = std::move(widget), ._PendingRemoval = false });
    layoutCells();
}

RetroFuturaGUI::ScrollView::ScrollViewCell* RetroFuturaGUI::ScrollView::findCell(std::string_view name)
{
    return const_cast<ScrollViewCell*>(std::as_const(*this).findCell(name));
}

const RetroFuturaGUI::ScrollView::ScrollViewCell* RetroFuturaGUI::ScrollView::findCell(std::string_view name) const
{
    const auto cellIterator { std::ranges::find_if(_items, [name](const ScrollViewCell& cell)
    {
        if(!cell._Widget)
            return false;

        return !cell._PendingRemoval && name == cell._Widget->GetName();
    }) };

    if(cellIterator == _items.end())
        return nullptr;

    return &*cellIterator;
}

void RetroFuturaGUI::ScrollView::SetSize(const glm::vec3& size)
{
    IWidget::SetSize(size);
    layoutCells();
}

void RetroFuturaGUI::ScrollView::SetPosition(const glm::vec3& position)
{
    IWidget::SetPosition(position);
    layoutCells();
}

void RetroFuturaGUI::ScrollView::SetRotation(const glm::vec3& rotation)
{
    IWidget::SetRotation(rotation);
}

void RetroFuturaGUI::ScrollView::EnableScrolling(const ScrollEnable enableMask)
{
    _enableMask = enableMask;
}

f32 RetroFuturaGUI::ScrollView::GetScrollPosition(const ScrollAxis axis) const
{
    switch(axis)
    {
        case ScrollAxis::Y:
            return _scrollPosition.y;
        case ScrollAxis::Z:
            return _scrollPosition.z;
        default:
            return _scrollPosition.x;
    }
}

void RetroFuturaGUI::ScrollView::SetScrollPosition(const ScrollAxis axis, const f32 scrollPosition)
{
    // ScrollAxis counts X, Y, Z from 0, which is how glm indexes a vector's components.
    glm::vec3 newScrollPosition { _scrollPosition };
    newScrollPosition[static_cast<glm::length_t>(axis)] = scrollPosition;
    applyScrollPosition(newScrollPosition);
}

void RetroFuturaGUI::ScrollView::interact()
{
    const PointerState pointer { readPointer() };

    if(pointer._IsCaptured)
        return; //enclosing ScrollView has taken the pointer

    const bool //drop presses that started outside and were dragged into this view
        isMouseInside { pointer._HasPosition && isPointInside(pointer._Position) },
        isPressStart { pointer._LeftButtonPressed && !_wasClicked };
    _wasClicked = pointer._LeftButtonPressed;

    // A press keeps going when the pointer leaves the view and ends only when the button is released, so it is advanced before the hover state
    updatePress(pointer, isMouseInside);

    if(!_isEnabledFlag || !isMouseInside)
    {
        if(_mouseEnteredFlag)
        {
            _mouseEnteredFlag = false;
            _onMouseLeaveAsync.EmitAsync();
            _onMouseLeave.Emit();

            // A press that left the view stays in its Clicked colors until the button is released.
            if(_pressPhase == PressPhase::None)
                setColors(ColorState::Enabled);
        }

        return;
    }

    _whileHoverAsync.EmitAsync();
    _whileHover.Emit();

    if(!_mouseEnteredFlag)
    {
        _mouseEnteredFlag = true;
        _onMouseEnterAsync.EmitAsync();
        _onMouseEnter.Emit();

        if(_pressPhase == PressPhase::None)
            setColors(ColorState::Hover);
    }

    if(isPressStart)
    {
        // Anywhere inside the view, on a widget or between them, the press is held back while the view decides whether it is a click or a scroll.
        _pressPhase = canScrollAlong(ScrollEnable::X) || canScrollAlong(ScrollEnable::Y) ? PressPhase::Deciding : PressPhase::HandedOver;
        _pressPosition = pointer._Position;
        _previousMousePosition = pointer._Position;
        _onClickAsync.EmitAsync();
        _onClick.Emit();
        setColors(ColorState::Clicked);
    }
}

void RetroFuturaGUI::ScrollView::updatePress(const PointerState& pointer, const bool isMouseInside)
{
    switch(_pressPhase)
    {
        case PressPhase::Deciding:
        {
            // Only movement decides, never time: a press held still for a while can still turn into a scroll
            const glm::vec2 travel { pointer._Position - _pressPosition };

            // The threshold is checked before the release, so a quick flick that ends within one frame still counts as a move.
            if(pointer._HasPosition && _dragThreshold < glm::length(travel))
            {
                // The axis the pointer mostly moved along decides
                const ScrollEnable movedAxis { std::abs(travel.y) < std::abs(travel.x) ? ScrollEnable::X : ScrollEnable::Y };

                if(canScrollAlong(movedAxis))
                {
                    _pressPhase = PressPhase::Scrolling;
                    dragScroll(pointer._Position);
                }
                else
                    _pressPhase = PressPhase::HandedOver;
            }
            else if(!pointer._LeftButtonPressed) // Released before it moved, so it was a click
            {
                _pressPhase = PressPhase::Replaying;
                releasePress(isMouseInside);
            }
        }
        break;
        case PressPhase::Scrolling:
            if(!pointer._LeftButtonPressed)
            {
                _pressPhase = PressPhase::None;
                releasePress(isMouseInside);
            }
            else if(pointer._HasPosition && _isEnabledFlag)
            {
                dragScroll(pointer._Position);
            }
        break;
        case PressPhase::Replaying:
            _pressPhase = PressPhase::None;
        break;
        case PressPhase::HandedOver:
            if(!pointer._LeftButtonPressed)
            {
                _pressPhase = PressPhase::None;
                releasePress(isMouseInside);
            }
        break;
        default:
        break;
    }
}

void RetroFuturaGUI::ScrollView::FilterPointer(PointerState& pointer) const
{
    switch(_pressPhase)
    {
        case PressPhase::Deciding:
            pointer._LeftButtonPressed = false;
        break;
        case PressPhase::Scrolling:
            pointer._LeftButtonPressed = false;
            pointer._IsCaptured = true;
        break;
        case PressPhase::Replaying:
            pointer._LeftButtonPressed = true;
        break;
        default:
        break;
    }
}

bool RetroFuturaGUI::ScrollView::canScrollAlong(const ScrollEnable axis) const
{
    if(!isScrollEnabled(axis))
        return false;

    const glm::vec3 overflow { _contentExtent - innerSize() };

    switch(axis)
    {
        case ScrollEnable::X:
            return 0.0f < overflow.x;
        case ScrollEnable::Y:
            return 0.0f < overflow.y;
        case ScrollEnable::Z:
            return 0.0f < overflow.z;
        default:
            return false;
    }
}

void RetroFuturaGUI::ScrollView::dragScroll(const glm::vec2& mousePosition)
{
    const glm::vec2 mouseDelta { mousePosition - _previousMousePosition };
    _previousMousePosition = mousePosition;

    // The content sticks to the pointer, so dragging pulls it along. The scroll position counts how far the view has moved
    // right and down into the content, so dragging right or down lowers it.
    glm::vec3 scrollPosition { _scrollPosition };

    if(isScrollEnabled(ScrollEnable::X))
        scrollPosition.x -= mouseDelta.x; //dragging right gives a positive delta

    if(isScrollEnabled(ScrollEnable::Y))
        scrollPosition.y += mouseDelta.y; //window y counts upward, so dragging down gives a negative delta

    applyScrollPosition(scrollPosition);
}

void RetroFuturaGUI::ScrollView::releasePress(const bool isMouseInside)
{
    _onReleaseAsync.EmitAsync();
    _onRelease.Emit();
    setColors(isMouseInside ? ColorState::Hover : ColorState::Enabled);
}

void RetroFuturaGUI::ScrollView::applyScrollPosition(const glm::vec3& scrollPosition)
{
    const glm::vec3 previousScrollPosition { _scrollPosition };
    _scrollPosition = scrollPosition;

    // Clamps the scroll position to the content and moves every widget to where it now shows.
    layoutCells();

    // Only an axis that actually moved emits, so dragging against the end of the content stays silent.
    if(previousScrollPosition.x != _scrollPosition.x)
    {
        _onXScrolledAsync.EmitAsync();
        _onXScrolled.Emit();
    }

    if(previousScrollPosition.y != _scrollPosition.y)
    {
        _onYScrolledAsync.EmitAsync();
        _onYScrolled.Emit();
    }

    if(previousScrollPosition.z != _scrollPosition.z)
    {
        _onZScrolledAsync.EmitAsync();
        _onZScrolled.Emit();
    }
}

bool RetroFuturaGUI::ScrollView::isScrollEnabled(const ScrollEnable axis) const
{
    return 0 != (static_cast<u32>(_enableMask) & static_cast<u32>(axis));
}

void RetroFuturaGUI::ScrollView::EnableBackground(const bool enable)
{
    _showBackground = enable;
    
    if(_showBorder)
    {
        _background = std::make_unique<Rectangle>(&_projection);
        _background->SetRectangleMode(RectangleMode::Plane);
    }
    else
        _background.reset();
}

void RetroFuturaGUI::ScrollView::EnableBorder(const bool enable)
{
    _showBorder = enable;

    if(_showBorder)
    {
        _border = std::make_unique<Rectangle>(&_projection);
        _border->SetRectangleMode(RectangleMode::Plane);
    }
    else
        _border.reset();
}

void RetroFuturaGUI::ScrollView::layoutCells()
{
    // glm indexes its components with length_t, so the stacking axis is kept in that type.
    const glm::length_t stackAxis { static_cast<glm::length_t>(_alignmentDirection) };
    const glm::vec3 viewSize { innerSize() };

    // First pass, measuring: the cells add up along the stacking axis. Across it, every cell spans the view,
    // or the widest widget when that is wider, so each widget can be aligned within the same span.
    glm::vec3 contentExtent { viewSize };
    contentExtent[stackAxis] = 0.0f;
    uSize cellCount { 0 };

    for(ScrollViewCell& cell : _items)
    {
        if(cell._PendingRemoval)
            continue;

        if(!cell._Widget)
            continue;

        cell._Size = cell._Widget->GetSize();

        for(glm::length_t axis = 0; 3 > axis; ++axis)
        {
            if(axis == stackAxis)
                contentExtent[axis] += cell._Size[axis];
            else
                contentExtent[axis] = (std::max)(contentExtent[axis], cell._Size[axis]);
        }

        ++cellCount;
    }

    if(1 < cellCount)
        contentExtent[stackAxis] += _cellPadding * static_cast<f32>(cellCount - 1);

    _contentExtent = contentExtent;

    // Content that shrank, after a removal or a resize, can leave the view scrolled past its end, so the scroll position is pulled back within reach.
    _scrollPosition = glm::clamp(_scrollPosition, glm::vec3(0.0f), (glm::max)(contentExtent - viewSize, glm::vec3(0.0f)));

    // Second pass, placing: each widget follows the previous one along the stacking axis, and sits in its span by the cell span alignment across it.
    const glm::vec3 alignment { spanAlignmentFactors() };
    f32 offset { 0.0f };

    for(ScrollViewCell& cell : _items)
    {
        if(cell._PendingRemoval)
            continue;

        if(!cell._Widget)
            continue;

        cell._Position = (contentExtent - cell._Size) * alignment;
        cell._Position[stackAxis] = offset;
        offset += cell._Size[stackAxis] + _cellPadding;

        cell._Widget->SetPosition(contentToWindow(cell._Position + cell._Size * 0.5f));
    }
}

glm::vec3 RetroFuturaGUI::ScrollView::innerSize() const
{
    if(!_border)
        return _size;

    // The border is drawn inside the widget's rectangle, so the cells scroll through what is left inside it.
    const f32 inset { 2.0f * _border->GetBorderWidth() };
    return glm::vec3((std::max)(_size.x - inset, 0.0f), (std::max)(_size.y - inset, 0.0f), _size.z);
}

glm::vec3 RetroFuturaGUI::ScrollView::contentToWindow(const glm::vec3& point) const
{
    // Content starts at the view's top-left front corner and runs rightward, downward and away from the camera.
    // Scrolling moves it the opposite way, which brings what lies further along into view.
    const glm::vec3 viewSize { innerSize() };
    const glm::vec3 scrolledPoint { point - _scrollPosition };

    return glm::vec3(_position.x - viewSize.x * 0.5f + scrolledPoint.x,
                     _position.y + viewSize.y * 0.5f - scrolledPoint.y,
                     _position.z + viewSize.z * 0.5f - scrolledPoint.z);
}

glm::vec3 RetroFuturaGUI::ScrollView::spanAlignmentFactors() const
{
    glm::vec3 factors { 0.5f };

    switch(_cellSpanAlignment._Horizontal)
    {
        case HorizontalSpanAlignment::Left:
            factors.x = 0.0f;
        break;
        case HorizontalSpanAlignment::Right:
            factors.x = 1.0f;
        break;
        default:
        break;
    }

    switch(_cellSpanAlignment._Vertical)
    {
        case VerticalSpanAlignment::Top:
            factors.y = 0.0f;
        break;
        case VerticalSpanAlignment::Bottom:
            factors.y = 1.0f;
        break;
        default:
        break;
    }

    switch(_cellSpanAlignment._Applicatal)
    {
        case ApplicatalSpanAlignment::Front:
            factors.z = 0.0f;
        break;
        case ApplicatalSpanAlignment::Back:
            factors.z = 1.0f;
        break;
        default:
        break;
    }

    return factors;
}

void RetroFuturaGUI::ScrollView::setColors(const ColorState state)
{
    _backgroundColorState = state;
    _borderColorState = state;
    setBackgroundColors();
    setBorderColors();
}

void RetroFuturaGUI::ScrollView::performPendingRemoval()
{
    if(!_hasPendingRemovals)
        return;

    std::erase_if(_items, [](const ScrollViewCell& cell) { return cell._PendingRemoval; });
    _hasPendingRemovals = false;
}