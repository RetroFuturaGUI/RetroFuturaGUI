#include "IDropDown.hpp"
#include "ResourceManager.hpp"
#include <algorithm>
#include <utility>

RetroFuturaGUI::IDropDown::IDropDown(std::string_view name, Projection* projection, IHierarchyNode* parentWidget, const WidgetTypeID parentWidgetTypeID, GLFWwindow* parentWindow)
    : IWidget(name, projection, parentWidget, parentWidgetTypeID, parentWindow)
{
    _dropDownBackground = std::make_unique<Plane>(projection);
    _dropDownBorder = std::make_unique<Frame>(projection);
    _dropDownItemHighlight = std::make_unique<Plane>(projection);
    useTransitions({ _dropDownBackground.get(), _dropDownBorder.get(), _dropDownItemHighlight.get() });
    _background.SetTransitionSettings(&_transitionSettings);
    _border.SetTransitionSettings(&_transitionSettings);

    if(!_dropDownBackground)
        return;

    if(!_dropDownBorder)
        return;

    if(!_dropDownItemHighlight)
        return;

    setDropDownBackgroundColors();
    setDropDownBorderColors();
    _dropDownItemHighlight->SetColors(_highLightColor);
    _dropDowns.push_back(this);
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
    if(!_dropDownBackground)
        return;

    if(!_dropDownBorder)
        return;

    if(!_dropDownItemHighlight)
        return;

    const f32 panelHeight { _size.y * static_cast<f32>(getVisibleItemCount()) };
    const glm::vec3 center { getDropDownCenter() };

    _dropDownBackground->SetSize(glm::vec2(getDropDownWidth(), panelHeight));
    _dropDownBackground->SetPosition(center);
    _dropDownBorder->SetSize(_dropDownBackground->GetSize());
    _dropDownBorder->SetPosition(center + glm::vec3(0.0f, 0.0f, _zStep * 3.0f));

    _dropDownItemHighlight->SetSize(glm::vec2(getDropDownWidth(), _size.y));
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

void RetroFuturaGUI::IDropDown::setDropDownBackgroundColors()
{
    if(!_dropDownBackground)
        return;

    switch(_dropDownBackgroundColorSate)
    {
        case ColorState::Enabled:
            _dropDownBackground->SetColors(_dropDownBackgroundColorsEnabled);
        break;
        case ColorState::Clicked:
             _dropDownBackground->SetColors(_dropDownBackgroundColorsClicked);
        break;
        case ColorState::Hover:
             _dropDownBackground->SetColors(_dropDownBackgroundColorsHover);
        break;
        default: //Disabled
             _dropDownBackground->SetColors(_dropDownBackgroundColorsDisabled);
    }
}

void RetroFuturaGUI::IDropDown::setDropDownBorderColors()
{
    if(!_dropDownBorder)
        return;

    switch(_dropDownBorderColorSate)
    {
        case ColorState::Enabled:
            _dropDownBorder->SetColors(_dropDownBorderColorsEnabled);
        break;
        case ColorState::Clicked:
             _dropDownBorder->SetColors(_dropDownBorderColorsClicked);
        break;
        case ColorState::Hover:
             _dropDownBorder->SetColors(_dropDownBorderColorsHover);
        break;
        default: //Disabled
             _dropDownBorder->SetColors(_dropDownBorderColorsDisabled);
    }
}

uSize RetroFuturaGUI::IDropDown::getVisibleItemCount() const
{
    return _itemCount < _maxItemsToDraw ? _itemCount : _maxItemsToDraw;
}

glm::vec3 RetroFuturaGUI::IDropDown::itemRowCenter(const uSize visibleIndex) const
{
    const glm::vec3 panelCenter { getDropDownCenter() };
    const f32 panelHeight { _size.y * static_cast<f32>(getVisibleItemCount()) };
    const glm::vec2 offset { glm::vec2(0.0f, panelHeight * 0.5f - _size.y * (static_cast<f32>(visibleIndex) + 0.5f)) };

    return glm::vec3(panelCenter.x + offset.x, panelCenter.y + offset.y, panelCenter.z + _zStep * 2.0f);
}

void RetroFuturaGUI::IDropDown::setColors(const ColorState state)
{
    _arrow.SetColorState(state);
    _dropDownBackgroundColorSate = state;
    _dropDownBorderColorSate = state;
    setDropDownBackgroundColors();
    setDropDownBorderColors();
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

void RetroFuturaGUI::IDropDown::SetIndex(const uSize index)
{
    _currentIndex = index;
}

void RetroFuturaGUI::IDropDown::SetDropDownBackgroundColors(std::span<const glm::vec4> colors, const ColorState state)
{
    _dropDownBackgroundColorSate = state;

    switch(state)
    {
        case ColorState::Enabled:
            _dropDownBackgroundColorsEnabled.assign(colors.begin(), colors.end());
        break;
        case ColorState::Clicked:
            _dropDownBackgroundColorsClicked.assign(colors.begin(), colors.end());
        break;
        case ColorState::Hover:
            _dropDownBackgroundColorsHover.assign(colors.begin(), colors.end());
        break;
        default: //disabled
            _dropDownBackgroundColorsDisabled.assign(colors.begin(), colors.end());
    }

    setDropDownBackgroundColors();
}

void RetroFuturaGUI::IDropDown::SetDropDownBorderColors(std::span<const glm::vec4> colors, const ColorState state)
{
    _dropDownBorderColorSate = state;

    switch(state)
    {
        case ColorState::Enabled:
            _dropDownBorderColorsEnabled.assign(colors.begin(), colors.end());
        break;
        case ColorState::Clicked:
            _dropDownBorderColorsClicked.assign(colors.begin(), colors.end());
        break;
        case ColorState::Hover:
            _dropDownBorderColorsHover.assign(colors.begin(), colors.end());
        break;
        default: //disabled
            _dropDownBorderColorsDisabled.assign(colors.begin(), colors.end());
    }

    setDropDownBorderColors();
}

void RetroFuturaGUI::IDropDown::SetDropDownCornerRadii(const glm::vec4& radii)
{
    if(!_dropDownBackground)
        return;

    if(!_dropDownBorder)
        return;

    _dropDownBackground->SetCornerRadii(radii);
    _dropDownBorder->SetCornerRadii(radii);
}

void RetroFuturaGUI::IDropDown::SetDropDownOpen(const bool open)
{
    _showDropDown = open;
    updateDropDownGeometry();
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

bool RetroFuturaGUI::IDropDown::isPointInsideDropDown(const glm::vec2& point) const
{
    if(!_dropDownBackground)
        return false;

    return isPointInsideRect(point, glm::vec3(_dropDownBackground->GetSize(), 0.0f), _dropDownBackground->GetPosition(), glm::vec3(0.0f));
}

bool RetroFuturaGUI::IDropDown::hoveredItemIndex(const glm::vec2& point, uSize& outIndex) const
{
    const glm::vec3 rowSize { getDropDownWidth(), _size.y, 0.0f };

    for(uSize index { 0 }; index < getVisibleItemCount(); ++index)
    {
        if(!isPointInsideRect(point, rowSize, itemRowCenter(index), glm::vec3(0.0f)))
            continue;

        outIndex = index;
        return true;
    }

    return false;
}