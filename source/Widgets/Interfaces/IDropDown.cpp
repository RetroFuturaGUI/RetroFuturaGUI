#include "IDropDown.hpp"
#include "ResourceManager.hpp"

RetroFuturaGUI::IDropDown::IDropDown(std::string_view name, Projection* projection, IHierarchyNode* parentWidget, const WidgetTypeID parentWidgetTypeID, GLFWwindow* parentWindow)
    : IWidget(name, projection, parentWidget, parentWidgetTypeID, parentWindow)
{
    _dropDownBackground = std::make_unique<Rectangle>(projection);
        _dropDownBorder = std::make_unique<Rectangle>(projection);
        _dropDownItemHighlight = std::make_unique<Rectangle>(projection);
        _dropDownArrow = ResourceManager::GetDropDownArrowIcon();

    if(_dropDownBackground)
        _dropDownBackground->SetRectangleMode(RectangleMode::Plane);

    if(_dropDownBorder)
        _dropDownBorder->SetRectangleMode(RectangleMode::Border);

    if(_dropDownItemHighlight)
        _dropDownItemHighlight->SetRectangleMode(RectangleMode::Plane);

    if(_dropDownArrow)
        _dropDownArrow->SetProjection(projection);

    setDropDownBackgroundColors();
    setDropDownBorderColors();

    if(_dropDownItemHighlight)
        _dropDownItemHighlight->SetColors(_highLightColor);
}

void RetroFuturaGUI::IDropDown::updateDropDownGeometry()
{
    if(!_dropDownBackground)
        return;

    if(!_dropDownBorder)
        return;

    const f32 panelHeight { _size.y * static_cast<f32>(visibleItemCount()) };
    const glm::vec3 center { dropDownCenter() };

    _dropDownBackground->SetSize(glm::vec2(_size.x, panelHeight));
    _dropDownBackground->SetPosition(center);
    _dropDownBorder->SetSize(_dropDownBackground->GetSize());
    _dropDownBorder->SetPosition(center + glm::vec3(0.0f, 0.0f, _zStep * 3.0f));

    if(_dropDownItemHighlight)
        _dropDownItemHighlight->SetSize(glm::vec2(_size.x, _size.y));
}

glm::vec3 RetroFuturaGUI::IDropDown::dropDownCenter() const
{
    const f32 panelHeight { _size.y * static_cast<f32>(visibleItemCount()) };
    glm::vec2 localOffset {};

    switch(_dropDownPosition)
    {
        case DropDownPosition::Right:
            localOffset = glm::vec2( _size.x, (_size.y - panelHeight) * 0.5f);
        break;
        case DropDownPosition::Left:
            localOffset = glm::vec2(-_size.x, (_size.y - panelHeight) * 0.5f);
        break;
        case DropDownPosition::Top:
            localOffset = glm::vec2(0.0f,  (_size.y + panelHeight) * 0.5f);
        break;
        default:
            localOffset = glm::vec2(0.0f, -(_size.y + panelHeight) * 0.5f);
    }

    const glm::vec2 offset { rotateLocalOffset(localOffset) };
    return glm::vec3(_position.x + offset.x, _position.y + offset.y, _position.z + _zOffset);
}

void RetroFuturaGUI::IDropDown::updateDropDownArrowGeometry()
{
    if(!_dropDownArrow)
        return;

    const bool isQuarterTurned { _dropDownPosition == DropDownPosition::Right || _dropDownPosition == DropDownPosition::Left };
    const f32
        extent { _size.y - _arrowPadding * 2.0f > 0.0f ? _size.y - _arrowPadding * 2.0f : 0.0f },
        aspectRatio { _dropDownArrow->GetAspectRatio() > 0.0f ? _dropDownArrow->GetAspectRatio() : 1.0f },
        width { aspectRatio > 1.0f ? extent : extent * aspectRatio },
        height { aspectRatio > 1.0f ? extent / aspectRatio : extent },
        drawnWidth { isQuarterTurned ? height : width },
        drawnHeight { isQuarterTurned ? width : height },
        rightMargin { (_size.y - drawnHeight) * 0.5f };
    const glm::vec2 offset { rotateLocalOffset(glm::vec2(_size.x * 0.5f - rightMargin - drawnWidth * 0.5f, 0.0f)) };

    _dropDownArrow->SetSize(glm::vec2(width, height));
    _dropDownArrow->SetPosition(glm::vec3(_position.x + offset.x, _position.y + offset.y, _position.z + _zStep * 2.0f));

    switch(_dropDownPosition)
    {
        case DropDownPosition::Right:
            _dropDownArrow->SetRotation(_rotation + glm::vec3(0.0f, 0.0f, 90.0f));
        break;
        case DropDownPosition::Left:
            _dropDownArrow->SetRotation(_rotation + glm::vec3(0.0f, 0.0f, 270.0f));
        break;
        case DropDownPosition::Top:
            _dropDownArrow->SetRotation(_rotation + glm::vec3(0.0f, 0.0f, 180.0f));
        break;
        default:
            _dropDownArrow->SetRotation(_rotation);
    }
}

void RetroFuturaGUI::IDropDown::setDropDownArrowColors()
{
    if(!_dropDownArrow)
        return;

    switch(_dropDownArrowColorState)
    {
        case ColorState::Enabled:
            _dropDownArrowPathFill.colors = _dropDownArrowColorsEnabled;
        break;
        case ColorState::Clicked:
            _dropDownArrowPathFill.colors = _dropDownArrowColorsClicked;
        break;
        case ColorState::Hover:
            _dropDownArrowPathFill.colors = _dropDownArrowColorsHover;
        break;
        default: //Disabled
            _dropDownArrowPathFill.colors = _dropDownArrowColorsDisabled;
    }
    
    if(_dropDownArrow)
        _dropDownArrow->SetPathFill("DropDownArrow", _dropDownArrowPathFill);
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

glm::vec2 RetroFuturaGUI::IDropDown::rotateLocalOffset(const glm::vec2& localOffset) const
{
    const f32 radians { glm::radians(_rotation.z) };

    return glm::vec2
    (
        localOffset.x * cos(radians) - localOffset.y * sin(radians),
        localOffset.x * sin(radians) + localOffset.y * cos(radians)
    );
}

uSize RetroFuturaGUI::IDropDown::visibleItemCount() const
{
    return _itemCount < _maxItemsToDraw ? _itemCount : _maxItemsToDraw;
}

glm::vec3 RetroFuturaGUI::IDropDown::itemRowCenter(const uSize visibleIndex) const
{
    const glm::vec3 panelCenter { dropDownCenter() };
    const f32 panelHeight { _size.y * static_cast<f32>(visibleItemCount()) };
    const glm::vec2 offset { rotateLocalOffset(glm::vec2(0.0f, panelHeight * 0.5f - _size.y * (static_cast<f32>(visibleIndex) + 0.5f))) };

    return glm::vec3(panelCenter.x + offset.x, panelCenter.y + offset.y, panelCenter.z + _zStep * 2.0f);
}

void RetroFuturaGUI::IDropDown::setColors(const ColorState state)
{
    _dropDownArrowColorState = state;
    _dropDownBackgroundColorSate = state;
    _dropDownBorderColorSate = state;
    setDropDownArrowColors();
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

uSize RetroFuturaGUI::IDropDown::GetSelectedIndex() const
{
    return _currentIndex;
}

void RetroFuturaGUI::IDropDown::SetIndex(const uSize index)
{
    _currentIndex = index;
}

void RetroFuturaGUI::IDropDown::SetDropDownArrowColors(std::span<glm::vec4> colors, const ColorState state)
{
    _dropDownArrowColorState = state;

    switch(state)
    {
        case ColorState::Enabled:
            _dropDownArrowColorsEnabled.assign(colors.begin(), colors.end());
        break;
        case ColorState::Clicked:
            _dropDownArrowColorsClicked.assign(colors.begin(), colors.end());
        break;
        case ColorState::Hover:
            _dropDownArrowColorsHover.assign(colors.begin(), colors.end());
        break;
        default: //disabled
            _dropDownArrowColorsDisabled.assign(colors.begin(), colors.end());
    }

    setDropDownArrowColors();
}

void RetroFuturaGUI::IDropDown::SetDropDownBackgroundColors(std::span<glm::vec4> colors, const ColorState state)
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

void RetroFuturaGUI::IDropDown::SetDropDownBorderColors(std::span<glm::vec4> colors, const ColorState state)
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
    if(_dropDownBackground)
        _dropDownBackground->SetCornerRadii(radii);

    if(_dropDownBorder)
        _dropDownBorder->SetCornerRadii(radii);
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

    return isPointInsideRect(point, glm::vec3(_dropDownBackground->GetSize(), 0.0f), _dropDownBackground->GetPosition(), _rotation);
}

bool RetroFuturaGUI::IDropDown::hoveredItemIndex(const glm::vec2& point, uSize& outIndex) const
{
    const glm::vec3 rowSize { _size.x, _size.y, 0.0f };

    for(uSize index { 0 }; index < visibleItemCount(); ++index)
    {
        if(!isPointInsideRect(point, rowSize, itemRowCenter(index), _rotation))
            continue;

        outIndex = index;
        return true;
    }

    return false;
}