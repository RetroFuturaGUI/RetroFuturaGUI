#include "IDropDown.hpp"
#include "ResourceManager.hpp"
#include "PlatformBridge.hpp"
#include "Window.hpp"
#include <algorithm>
#include <cmath>
#include <utility>

RetroFuturaGUI::IDropDown::IDropDown(std::string_view name, Projection* projection, IHierarchyNode* parentWidget, const WidgetTypeID parentWidgetTypeID, GLFWwindow* parentWindow)
    : IWidget(name, projection, parentWidget, parentWidgetTypeID, parentWindow)
{
    _list.SetTransitionSettings(&_transitionSettings);
    _background.SetTransitionSettings(&_transitionSettings);
    _border.SetTransitionSettings(&_transitionSettings);
    useBorderGap(&_background, &_border);
    useFlavors(&_background, &_border, nullptr);
    useSubPartFlavors(_list.GetBackground(), StylePart::ListBackground);
    useSubPartFlavors(_list.GetBorder(), StylePart::ListBorder);
    useSubPartFlavors(_list.GetHighlight(), StylePart::ListHighlight);
    useSubPartFlavors(_arrow, StylePart::Arrow);

    _dropDowns.push_back(this);
}

RetroFuturaGUI::DropDownList& RetroFuturaGUI::IDropDown::GetList()
{
    return _list;
}

const RetroFuturaGUI::DropDownList& RetroFuturaGUI::IDropDown::GetList() const
{
    return _list;
}

RetroFuturaGUI::Icon& RetroFuturaGUI::IDropDown::GetArrow()
{
    return _arrow;
}

const RetroFuturaGUI::Icon& RetroFuturaGUI::IDropDown::GetArrow() const
{
    return _arrow;
}

RetroFuturaGUI::Background& RetroFuturaGUI::IDropDown::GetBackground()
{
    return _background;
}

const RetroFuturaGUI::Background& RetroFuturaGUI::IDropDown::GetBackground() const
{
    return _background;
}

RetroFuturaGUI::Border& RetroFuturaGUI::IDropDown::GetBorder()
{
    return _border;
}

const RetroFuturaGUI::Border& RetroFuturaGUI::IDropDown::GetBorder() const
{
    return _border;
}

RetroFuturaGUI::IDropDown::~IDropDown()
{
    std::erase(_dropDowns, this);
    std::erase(_shownDropDowns, this);
}

void RetroFuturaGUI::IDropDown::DrawShown()
{
    if(_shownDropDowns.empty())
        return;

    //By index: an ExtendedComboBox's row may be a drop-down itself, whose list joins while the panel draws
    for(uSize index = 0; index < _shownDropDowns.size(); ++index)
    {
        IDropDown* dropDown { _shownDropDowns[index] };

        if(!dropDown)
            continue;

        glClear(GL_DEPTH_BUFFER_BIT); //over everything drawn before, a row's list over its panel too
        const Shader::OpacityScope opacity { dropDown->_listOpacity.Get() };
        const Shader::FrameScope frame { dropDown->_listFrame, Shader::FrameComposition::Instead }; //tilted with its widget.
        dropDown->drawDropDown();
    }

    _shownDropDowns.clear();
}

void RetroFuturaGUI::IDropDown::CaptureUnderOpenList(const IHierarchyNode* reader, GLFWwindow* readerWindow, PointerState& pointer)
{
    if(!pointer._HasPosition)
        return;

    for(const IDropDown* dropDown : _dropDowns)
    {
        if(!dropDown)
            continue;

        if(!dropDown->isListOver(reader, readerWindow, pointer._WindowPosition))
            continue;

        pointer._IsCaptured = true;
        return;
    }
}

bool RetroFuturaGUI::IDropDown::IsOpenListAt(GLFWwindow* window, const glm::vec2& windowPoint)
{
    return std::any_of(_dropDowns.begin(), _dropDowns.end(), [window, &windowPoint](const IDropDown* dropDown)
    {
        return dropDown && dropDown->isListOver(nullptr, window, windowPoint); //no reader: every list counts.
    });
}

bool RetroFuturaGUI::IDropDown::isListOver(const IHierarchyNode* reader, GLFWwindow* readerWindow, const glm::vec2& windowPoint) const
{
    if(!_showDropDown && !_holdsPickPress)
        return false;

    if(readerWindow != _parentWindow)
        return false;

    for(const IHierarchyNode* node { reader }; node; node = node->GetParent())
    {
        if(node == this)
            return false; //the drop-down itself, or a row of an ExtendedComboBox
    }

    const std::optional<glm::vec2> listPoint { toLayoutPoint(windowPoint, _listFrame) }; //the list lies in the frame its widget drew in.
    return listPoint.has_value() && isPointInsideDropDown(*listPoint);
}

void RetroFuturaGUI::IDropDown::showDropDownLater()
{
    //Fades in while open and out once closed, over the drop-down's Opacity transition, and draws until it has faded out
    const f32 target { _showDropDown ? 1.0f : 0.0f };

    if(target != _listOpacity.GetTarget())
        _listOpacity.Set(target, Transitions::Resolve(_transitionSettings, TransitionProperty::Opacity));

    if(0.0f >= _listOpacity.Get())
        return;

    _listFrame = Shader::GetDrawFrame(); //called while the widget draws, so the list draws in the same frame later

    if(std::find(_shownDropDowns.begin(), _shownDropDowns.end(), this) != _shownDropDowns.end())
        return;

    _shownDropDowns.push_back(this);
}

std::vector<RetroFuturaGUI::IDropDown*> RetroFuturaGUI::IDropDown::takeShown()
{
    return std::exchange(_shownDropDowns, {});
}

void RetroFuturaGUI::IDropDown::restoreShown(std::vector<IDropDown*>&& shown)
{
    _shownDropDowns = std::move(shown);
}

void RetroFuturaGUI::IDropDown::updateDropDownGeometry()
{
    const f32 panelHeight { _size.y * static_cast<f32>(getVisibleItemCount()) };
    const glm::vec3 center { getDropDownCenter() };

    _list.GetBackground().SetSize(glm::vec2(getDropDownWidth(), panelHeight));
    _list.GetBackground().SetPosition(center);
    _list.GetBorder().SetSize(_list.GetBackground().GetSize());
    _list.GetBorder().SetPosition(center + glm::vec3(0.0f, 0.0f, _zStep * 3.0f));

    _list.GetHighlight().SetSize(glm::vec2(getDropDownWidth(), _size.y));
    updateDropDownArrowGeometry();
}

void RetroFuturaGUI::IDropDown::SetDropDownWidth(const f32 width)
{
    _dropDownWidth = 0.0f < width ? width : 0.0f;
    updateDropDownGeometry();
}

f32 RetroFuturaGUI::IDropDown::getDropDownWidth() const
{
    return 0.0f < _dropDownWidth ? _dropDownWidth : _size.x;
}

glm::vec3 RetroFuturaGUI::IDropDown::getDropDownCenter() const
{
    const f32
        panelHeight { _size.y * static_cast<f32>(getVisibleItemCount()) },
        panelWidth { getDropDownWidth() },
        nearEdgeShift { (panelWidth - _size.x) * 0.5f }; //a list wider than the box keeps its left edge on the box's
    glm::vec2 localOffset {};

    switch(_dropDownPosition)
    {
        case DropDownPosition::Right:
            localOffset = glm::vec2( (_size.x + panelWidth) * 0.5f, (_size.y - panelHeight) * 0.5f);
        break;
        case DropDownPosition::Left:
            localOffset = glm::vec2(-(_size.x + panelWidth) * 0.5f, (_size.y - panelHeight) * 0.5f);
        break;
        case DropDownPosition::Top:
            localOffset = glm::vec2(nearEdgeShift,  (_size.y + panelHeight) * 0.5f);
        break;
        default:
            localOffset = glm::vec2(nearEdgeShift, -(_size.y + panelHeight) * 0.5f);
    }

    const glm::vec2 offset { localOffset }; //laid out untilted. The frame turns it with the widget
    return glm::vec3(_position.x + offset.x, _position.y + offset.y, _position.z + _zOffset);
}

void RetroFuturaGUI::IDropDown::drawDropDownArrow()
{
    _arrow.Draw();
}

void RetroFuturaGUI::IDropDown::updateDropDownArrowGeometry()
{
    const bool isQuarterTurned { _dropDownPosition == DropDownPosition::Right || _dropDownPosition == DropDownPosition::Left };
    const f32
        padding { _size.y * 0.25f < _arrowPadding ? _size.y * 0.25f : _arrowPadding }, //a box under 60 px tall keeps half its height for the arrow, instead of none below 30 px
        extent { 0.0f < _size.y - padding * 2.0f ? _size.y - padding * 2.0f : 0.0f },
        aspectRatio { 0.0f < _arrow.GetAspectRatio() ? _arrow.GetAspectRatio() : 1.0f },
        width { 1.0f < aspectRatio ? extent : extent * aspectRatio },
        height { 1.0f < aspectRatio ? extent / aspectRatio : extent },
        drawnWidth { isQuarterTurned ? height : width },
        drawnHeight { isQuarterTurned ? width : height },
        rightMargin { (_size.y - drawnHeight) * 0.5f };
    const glm::vec2 offset { glm::vec2(_size.x * 0.5f - rightMargin - drawnWidth * 0.5f, 0.0f) };
    _arrowSpace = rightMargin + drawnWidth;

    _arrow.SetSize(glm::vec2(width, height));
    _arrow.SetPosition(glm::vec3(_position.x + offset.x, _position.y + offset.y, _position.z + _zStep * 2.0f));

    switch(_dropDownPosition)
    {
        case DropDownPosition::Right:
            _arrow.SetRotation(glm::vec3(0.0f, 0.0f, 90.0f));
        break;
        case DropDownPosition::Left:
            _arrow.SetRotation(glm::vec3(0.0f, 0.0f, 270.0f));
        break;
        case DropDownPosition::Top:
            _arrow.SetRotation(glm::vec3(0.0f, 0.0f, 180.0f));
        break;
        default:
            _arrow.SetRotation(glm::vec3(0.0f));
    }
}

uSize RetroFuturaGUI::IDropDown::getVisibleItemCount() const
{
    return _itemCount < _maxVisibleItemCount ? _itemCount : _maxVisibleItemCount;
}

uSize RetroFuturaGUI::IDropDown::getFirstVisibleItemIndex() const
{
    return _firstVisibleItemIndex;
}

bool RetroFuturaGUI::IDropDown::getItemRow(const uSize itemIndex, uSize& outRow) const
{
    if(itemIndex < _firstVisibleItemIndex || _firstVisibleItemIndex + getVisibleItemCount() <= itemIndex)
        return false;

    outRow = itemIndex - _firstVisibleItemIndex;
    return true;
}

glm::vec3 RetroFuturaGUI::IDropDown::itemRowCenter(const uSize row) const
{
    const glm::vec3 panelCenter { getDropDownCenter() };
    const f32 panelHeight { _size.y * static_cast<f32>(getVisibleItemCount()) };
    const glm::vec2 offset { glm::vec2(0.0f, panelHeight * 0.5f - _size.y * (static_cast<f32>(row) + 0.5f)) };

    return glm::vec3(panelCenter.x + offset.x, panelCenter.y + offset.y, panelCenter.z + _zStep * 2.0f);
}

void RetroFuturaGUI::IDropDown::setColors(const ColorState state)
{
    _arrow.SetColorState(state);
    _list.SetColorState(state);
}

void RetroFuturaGUI::IDropDown::Connect_OnItemClicked(const typename Signal<>::Slot& slot, const bool async)
{
    if(async)
        _onItemClickedAsync.Connect(slot);
    else
        _onItemClicked.Connect(slot);
}

void RetroFuturaGUI::IDropDown::Connect_OnIndexChanged(const typename Signal<>::Slot& slot, const bool async)
{
    if(async)
        _onIndexChangedAsync.Connect(slot);
    else
        _onIndexChanged.Connect(slot);
}

void RetroFuturaGUI::IDropDown::Disconnect_OnItemClicked(const typename Signal<>::Slot& slot)
{
    _onItemClickedAsync.Disconnect(slot);
    _onItemClicked.Disconnect(slot);
}

void RetroFuturaGUI::IDropDown::Disconnect_OnIndexChanged(const typename Signal<>::Slot& slot)
{
    _onIndexChangedAsync.Disconnect(slot);
    _onIndexChanged.Disconnect(slot);
}

uSize RetroFuturaGUI::IDropDown::GetItemCount() const
{
    return _itemCount;
}

f32 RetroFuturaGUI::IDropDown::GetFontSize(const FontSizeUnit sizeUnit) const
{
    return FontManager::FromPixels(_fontSize, sizeUnit);
}

uSize RetroFuturaGUI::IDropDown::GetSelectedIndex() const
{
    return _currentIndex;
}

void RetroFuturaGUI::IDropDown::SetIndex(const uSize index, const bool emitSignal)
{
    if(0 == _itemCount)
        return;

    const uSize clampedIndex { index < _itemCount ? index : _itemCount - 1 };

    if(clampedIndex == _currentIndex)
        return;

    _currentIndex = clampedIndex;

    if(!emitSignal)
        return;

    _onIndexChangedAsync.EmitAsync();
    _onIndexChanged.Emit();
}

void RetroFuturaGUI::IDropDown::SetDropDownOpen(const bool open)
{
    setListOpen(open);
    updateDropDownGeometry();
}

void RetroFuturaGUI::IDropDown::SetMaxVisibleItemCount(const uSize count)
{
    _maxVisibleItemCount = 0 < count ? count : 1;
    clampListScroll();
    updateDropDownGeometry();
}

uSize RetroFuturaGUI::IDropDown::GetMaxVisibleItemCount() const
{
    return _maxVisibleItemCount;
}

void RetroFuturaGUI::IDropDown::setListOpen(const bool open)
{
    if(open && !_showDropDown)
        scrollItemIntoView(_currentIndex);

    _showDropDown = open;
}

void RetroFuturaGUI::IDropDown::scrollListByWheel(const PointerState& pointer, const bool isOverList)
{
    if(!_showDropDown || !isOverList || 0.0f == pointer._Wheel.y)
        return;

    const uSize visibleCount { getVisibleItemCount() };

    if(_itemCount <= visibleCount)
        return;

    Window::TakeWheel(_parentWindow, WheelParts::Vertical); //even at either end, so nothing under the list scrolls instead
    const u32 linesPerNotch { PlatformBridge::Input::GetWheelScrollLines() };
    const f32 rowsPerNotch { static_cast<f32>(0 == linesPerNotch ? visibleCount : linesPerNotch) };
    _wheelRowRemainder -= pointer._Wheel.y * rowsPerNotch; //away from the user scrolls up, toward the first item
    const f32 wholeRows { std::trunc(_wheelRowRemainder) };
    _wheelRowRemainder -= wholeRows;

    const i64
        lastFirstIndex { static_cast<i64>(_itemCount - visibleCount) },
        firstIndex { static_cast<i64>(_firstVisibleItemIndex) + static_cast<i64>(wholeRows) };
    _firstVisibleItemIndex = static_cast<uSize>(std::clamp<i64>(firstIndex, 0, lastFirstIndex));
}

void RetroFuturaGUI::IDropDown::scrollItemIntoView(const uSize itemIndex)
{
    const uSize visibleCount { getVisibleItemCount() };

    if(itemIndex < _firstVisibleItemIndex)
        _firstVisibleItemIndex = itemIndex;
    else if(0 < visibleCount && _firstVisibleItemIndex + visibleCount <= itemIndex)
        _firstVisibleItemIndex = itemIndex + 1 - visibleCount;

    clampListScroll();
}

void RetroFuturaGUI::IDropDown::clampListScroll()
{
    const uSize lastFirstIndex { _itemCount - getVisibleItemCount() };

    if(lastFirstIndex < _firstVisibleItemIndex)
        _firstVisibleItemIndex = lastFirstIndex;
}

void RetroFuturaGUI::IDropDown::SetLabelHiddenWhenOpen(const bool hide)
{
    _hideLabelWhenOpen = hide;
}

bool RetroFuturaGUI::IDropDown::IsPointInsideOpenDropDown(const glm::vec2& point) const
{
    if(!_showDropDown)
        return false;

    return isPointInsideDropDown(point);
}

f32 RetroFuturaGUI::IDropDown::getLabelWidth() const
{
    return 0.0f < _size.x - _arrowSpace ? _size.x - _arrowSpace : 0.0f;
}

glm::vec3 RetroFuturaGUI::IDropDown::getLabelCenter() const
{
    return _position + glm::vec3(-_arrowSpace * 0.5f, 0.0f, _zStep * 2.0f);
}

bool RetroFuturaGUI::IDropDown::isPointInsideDropDown(const glm::vec2& point) const
{
    return isPointInsideRect(point, glm::vec3(_list.GetBackground().GetSize(), 0.0f), _list.GetBackground().GetPosition(), glm::vec3(0.0f));
}

bool RetroFuturaGUI::IDropDown::hoveredItemIndex(const glm::vec2& point, uSize& outIndex) const
{
    const glm::vec3 rowSize { getDropDownWidth(), _size.y, 0.0f };

    for(uSize row { 0 }; row < getVisibleItemCount(); ++row)
    {
        if(!isPointInsideRect(point, rowSize, itemRowCenter(row), glm::vec3(0.0f)))
            continue;

        outIndex = _firstVisibleItemIndex + row;
        return true;
    }

    return false;
}