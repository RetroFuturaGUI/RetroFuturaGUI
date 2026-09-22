#include "ComboBox.hpp"
#include "IWidget.hpp"
#include "IncludeHelper.hpp"
#include "Rectangle.hpp"
#include "ResourceManager.hpp"
#include "SvgTexture.hpp"
#include <glm/ext/vector_float3.hpp>
#include <memory>
#include <iterator>
#if defined(TARGET_PLATFORM_LINUX)
    #define GLFW_EXPOSE_NATIVE_X11
#elif defined(TARGET_PLATFORM_WINDOWS)
    #define GLFW_EXPOSE_NATIVE_WIN32
#endif
#include <GLFW/glfw3native.h>

RetroFuturaGUI::ComboBox::ComboBox(std::string_view name, Projection* projection, IHierarchyNode* parentWidget, const WidgetTypeID parentWidgetTypeID, GLFWwindow* parentWindow)
    : IWidget(name, projection, parentWidget, parentWidgetTypeID, parentWindow)
{
    _widgetTypeID = WidgetTypeID::ComboBox;
    _background = std::make_unique<Rectangle>(projection);
    _border = std::make_unique<Rectangle>(projection);
    _dropDownBackground = std::make_unique<Rectangle>(projection);
    _dropDownBorder = std::make_unique<Rectangle>(projection);
    _dropDownItemHighlight = std::make_unique<Rectangle>(projection);
    _arrowDown = ResourceManager::GetArrowDownIcon();

    if(_background)
        _background->SetRectangleMode(RectangleMode::Plane);

    if(_border)
        _border->SetRectangleMode(RectangleMode::Border);

    if(_dropDownBackground)
        _dropDownBackground->SetRectangleMode(RectangleMode::Plane);

    if(_dropDownBorder)
        _dropDownBorder->SetRectangleMode(RectangleMode::Border);

    if(_dropDownItemHighlight)
        _dropDownItemHighlight->SetRectangleMode(RectangleMode::Plane);

    if(_arrowDown)
        _arrowDown->SetProjection(projection);

    setColors(ColorState::Enabled);
    setDropDownBackgroundColors();
    setDropDownBorderColors();

    if(_dropDownItemHighlight)
        _dropDownItemHighlight->SetColors(_highLightColor);
}

void RetroFuturaGUI::ComboBox::Draw()
{
    interact();

    if(_background)
        _background->Draw();

    if(_border)
        _border->Draw();

    if(_arrowDown)
    {
        updateArrowDownGeometry();
        _arrowDown->Draw();
    }

    if(_currentIndex < _itemTexts.size())
    {
        auto iterator { _itemTexts.begin() };
        std::advance(iterator, _currentIndex);

        if(*iterator)
        {
            placeItemText(**iterator, _position + glm::vec3(0.0f, 0.0f, _zStep * 2.0f));
            iterator->get()->Draw();
        }
    }

    if(!_showDropDown)
        return;

    if(_dropDownBackground)
        _dropDownBackground->Draw();

    if(_dropDownBorder)
        _dropDownBorder->Draw();

    updateItemGeometry(); //the selected item was just drawn over the closed box, so every row is placed again before the list is drawn

    if(_dropDownItemHighlight && _dropDownItemHovered)
        _dropDownItemHighlight->Draw();

    uSize visibleIndex { 0 };

    for(const auto& item : _itemTexts)
    {
        if(visibleIndex >= visibleItemCount())
            return;

        if(item)
            item->Draw();

        ++visibleIndex;
    }
}

void RetroFuturaGUI::ComboBox::SetSize(const glm::vec3& size)
{
    IWidget::SetSize(size);

    if(_background)
        _background->SetSize(size);

    if(_border)
        _border->SetSize(size);

    updateDropDownGeometry();
}

void RetroFuturaGUI::ComboBox::updateDropDownGeometry()
{
    if(!_dropDownBackground)
        return;

    if(!_dropDownBorder)
        return;

    //dropDown hangs off the bottom edge of the closed box and shows at most _maxItemsToDraw rows
    const f32 panelHeight { _size.y * static_cast<f32>(visibleItemCount()) };
    const glm::vec2 offset { rotateLocalOffset(glm::vec2(0.0f, - (_size.y + panelHeight) * 0.5f)) };
    const glm::vec3 center { _position.x + offset.x, _position.y + offset.y, _position.z + _zOffset };

    _dropDownBackground->SetSize(glm::vec2(_size.x, panelHeight));
    _dropDownBackground->SetPosition(center);
    _dropDownBorder->SetSize(_dropDownBackground->GetSize());
    _dropDownBorder->SetPosition(center + glm::vec3(0.0f, 0.0f, _zStep * 3.0f));

    if(_dropDownItemHighlight)
        _dropDownItemHighlight->SetSize(glm::vec2(_size.x, _size.y));
}

void RetroFuturaGUI::ComboBox::updateItemGeometry()
{
    if(_dropDownItemHighlight && _nthItemHovered < visibleItemCount())
        _dropDownItemHighlight->SetPosition(itemRowCenter(_nthItemHovered) - glm::vec3(0.0f, 0.0f, _zStep));

    uSize visibleIndex { 0 };

    for(const auto& item : _itemTexts)
    {
        if(visibleIndex >= visibleItemCount())
            return;

        if(item)
            placeItemText(*item, itemRowCenter(visibleIndex));

        ++visibleIndex;
    }
}

void RetroFuturaGUI::ComboBox::placeItemText(Text& item, const glm::vec3& center) const
{
    /* Text lays itself out inside the parent size, so that size loses the border on both sides to keep
       the glyphs off the frame. An item is drawn against the box when it is the selected one and against
       the panel when the list is open, so the wider of the two borders is used for both: the selected
       item then does not shift sideways as the list opens, and the parent size stays constant, which
       matters because Text rebuilds its glyph mesh whenever that size changes. */
    const f32
        boxBorder { _border ? _border->GetBorderWidth() : 0.0f },
        panelBorder { _dropDownBorder ? _dropDownBorder->GetBorderWidth() : 0.0f },
        inset { (boxBorder > panelBorder ? boxBorder : panelBorder) * 2.0f },
        parentWidth { _size.x - inset > 0.0f ? _size.x - inset : 0.0f },
        parentHeight { _size.y - inset > 0.0f ? _size.y - inset : 0.0f };

    item.SetParentSize(glm::vec2(parentWidth, parentHeight));
    item.SetPosition(center);
    item.SetRotation(_rotation);
    item.SetTextAlignment(_textAlignment);
    item.SetTextPadding(_textPadding);
}

uSize RetroFuturaGUI::ComboBox::visibleItemCount() const
{
    return _itemTexts.size() < _maxItemsToDraw ? _itemTexts.size() : _maxItemsToDraw;
}

glm::vec3 RetroFuturaGUI::ComboBox::itemRowCenter(const uSize visibleIndex) const
{
    const glm::vec2 offset { rotateLocalOffset(glm::vec2(0.0f, - _size.y * (static_cast<f32>(visibleIndex) + 1.0f))) };
    return glm::vec3(_position.x + offset.x, _position.y + offset.y, _position.z + _zOffset + _zStep * 2.0f);
}

glm::vec2 RetroFuturaGUI::ComboBox::rotateLocalOffset(const glm::vec2& localOffset) const
{
    const f32 radians { glm::radians(_rotation.z) };

    return glm::vec2
    (
        localOffset.x * cos(radians) - localOffset.y * sin(radians),
        localOffset.x * sin(radians) + localOffset.y * cos(radians)
    );
}

void RetroFuturaGUI::ComboBox::updateArrowDownGeometry()
{
    if(!_arrowDown)
        return;

    const f32
        extent { _size.y - _arrowPadding * 2.0f > 0.0f ? _size.y - _arrowPadding * 2.0f : 0.0f },
        aspectRatio { _arrowDown->GetAspectRatio() > 0.0f ? _arrowDown->GetAspectRatio() : 1.0f },
        width { aspectRatio > 1.0f ? extent : extent * aspectRatio },
        height { aspectRatio > 1.0f ? extent / aspectRatio : extent },
        rightMargin { (_size.y - height) * 0.5f };
    const glm::vec2 offset { rotateLocalOffset(glm::vec2(_size.x * 0.5f - rightMargin - width * 0.5f, 0.0f)) };

    _arrowDown->SetSize(glm::vec2(width, height));
    _arrowDown->SetPosition(glm::vec3(_position.x + offset.x, _position.y + offset.y, _position.z + _zStep * 2.0f));
    _arrowDown->SetRotation(_rotation);
}

void RetroFuturaGUI::ComboBox::SetPosition(const glm::vec3& position)
{
    IWidget::SetPosition(position);

    if(_background)
        _background->SetPosition(position);

    if(_border)
        _border->SetPosition(position + glm::vec3(0.0f, 0.0f, _zStep));

    updateDropDownGeometry();
}

void RetroFuturaGUI::ComboBox::SetRotation(const glm::vec3& rotation)
{
    IWidget::SetRotation(rotation);

    if(_background)
        _background->SetRotation(rotation);

    if(_border)
        _border->SetRotation(rotation);

    if(_dropDownBackground)
        _dropDownBackground->SetRotation(rotation);

    if(_dropDownBorder)
        _dropDownBorder->SetRotation(rotation);

    if(_dropDownItemHighlight)
        _dropDownItemHighlight->SetRotation(rotation);

    updateDropDownGeometry();
}

void RetroFuturaGUI::ComboBox::AddItem(std::string_view name)
{
    _itemTexts.emplace_back(std::make_unique<Text>(&_projection));

    if(!_fontFamily.empty())
        _itemTexts.back()->SetFontFamily(_fontFamily, _fontSize, _slant, _fontWeight);

    _itemTexts.back()->SetTextUTF8(name);
    updateDropDownGeometry();
}

void RetroFuturaGUI::ComboBox::RemoveItem(const uSize index)
{
    auto iterator = _itemTexts.begin();
    std::advance(iterator, index);
    _itemTexts.erase(iterator);
    updateDropDownGeometry();
}

void RetroFuturaGUI::ComboBox::Connect_OnItemClicked(const typename Signal<>::Slot& slot, const bool async)
{
    if(async)
        _onItemClickedAsync.Connect(slot);
    else
        _onItemClicked.Connect(slot);
}

void RetroFuturaGUI::ComboBox::Connect_OnIndexChanged(const typename Signal<>::Slot& slot, const bool async)
{
    if(async)
        _onIndexChangedAsync.Connect(slot);
    else
        _onIndexChanged.Connect(slot);
}

void RetroFuturaGUI::ComboBox::Disconnect_OnItemClicked(const typename Signal<>::Slot& slot)
{
    _onItemClickedAsync.Disconnect(slot);
    _onItemClicked.Disconnect(slot);
}

void RetroFuturaGUI::ComboBox::Disconnect_OnIndexChanged(const typename Signal<>::Slot& slot)
{
    _onIndexChangedAsync.Disconnect(slot);
    _onIndexChanged.Disconnect(slot);
}

uSize RetroFuturaGUI::ComboBox::GetItemCount() const
{
    return _itemTexts.size();
}

uSize RetroFuturaGUI::ComboBox::GetSelectedIndex() const
{
    return _currentIndex;
}

void RetroFuturaGUI::ComboBox::SetIndex(const uSize index)
{
    _currentIndex = index;
}

const std::string& RetroFuturaGUI::ComboBox::GetSelectedText() const
{
    if(_currentIndex >= _itemTexts.size())
       return _invalidIndex;

    auto iterator { _itemTexts.begin() };
    std::advance(iterator, _currentIndex);
    return iterator->get()->GetTextUTF8();
}

void RetroFuturaGUI::ComboBox::interact()
{
     i32
        mouseX { 0 },
        mouseY { 0 };
    bool hasMousePosition { false };

#if defined(TARGET_PLATFORM_LINUX)
    hasMousePosition = PlatformBridge::Input::GetMouseWindowPosition(glfwGetX11Window(_parentWindow), mouseX, mouseY);
#elif defined(TARGET_PLATFORM_WINDOWS)
    hasMousePosition = PlatformBridge::Input::GetMouseWindowPosition(glfwGetWin32Window(_parentWindow), mouseX, mouseY);
#endif

    //PlatformBridge reports native (top-down) window coordinates; flip to this library's bottom-up world space here
    const glm::vec2 mousePos { static_cast<f32>(mouseX), _projection.GetResolution().y - static_cast<f32>(mouseY) };
    const bool
        isMousePressed { PlatformBridge::Input::IsMouseButtonDown(PlatformBridge::MouseButton::Left) },
        pressedThisFrame { isMousePressed && !_wasClicked },
        releasedThisFrame { !isMousePressed && _wasClicked },
        isInsideBox { hasMousePosition && isPointInside(mousePos) },
        isInsideDropDown { hasMousePosition && _showDropDown && isPointInsideDropDown(mousePos) };
    _wasClicked = isMousePressed;

    //the open dropDown belongs to the widget, so hovering a row still counts as hovering the ComboBox
    const bool isHovering { _isEnabledFlag && (isInsideBox || isInsideDropDown) };
    _dropDownItemHovered = isHovering && isInsideDropDown && hoveredItemIndex(mousePos, _nthItemHovered);

    if(isHovering) // hover
    {
        _whileHoverAsync.EmitAsync();
        _whileHover.Emit();
    }

    if(isHovering && !_mouseEnteredFlag) //enter
    {
        _mouseEnteredFlag = true;
        _onMouseEnterAsync.EmitAsync();
        _onMouseEnter.Emit();
        setColors(ColorState::Hover);
    }
    else if(!isHovering && _mouseEnteredFlag) //leave
    {
        _mouseEnteredFlag = false;
        _onMouseLeaveAsync.EmitAsync();
        _onMouseLeave.Emit();
        setColors(ColorState::Enabled);
    }

    if(releasedThisFrame) //release, wherever the button came up, because the press belonged to this widget
    {
        _onReleaseAsync.EmitAsync();
        _onRelease.Emit();

        if(isHovering)
            setColors(ColorState::Hover);
        else
            setColors(ColorState::Enabled);
    }

    if(!isHovering)
    {
        if(pressedThisFrame) //a press anywhere else puts the drop down away
        {
            _showDropDown = false;
            _dropDownItemHovered = false;
        }

        return;
    }

    if(!pressedThisFrame)
        return;

    //click
    _onClickAsync.EmitAsync();
    _onClick.Emit();
    setColors(ColorState::Clicked);

    if(_dropDownItemHovered) //picking a row selects it and closes the list
    {
        selectItem(_nthItemHovered);
        _showDropDown = false;
        _dropDownItemHovered = false;
        return;
    }

    if(isInsideBox)
        _showDropDown = !_showDropDown;
}

bool RetroFuturaGUI::ComboBox::isPointInsideDropDown(const glm::vec2& point) const
{
    if(!_dropDownBackground)
        return false;

    return isPointInsideRect(point, glm::vec3(_dropDownBackground->GetSize(), 0.0f), _dropDownBackground->GetPosition(), _rotation);
}

bool RetroFuturaGUI::ComboBox::hoveredItemIndex(const glm::vec2& point, uSize& outIndex) const
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

void RetroFuturaGUI::ComboBox::selectItem(const uSize index)
{
    if(index >= _itemTexts.size())
        return;

    const bool indexChanged { index != _currentIndex };
    _currentIndex = index;
    _onItemClickedAsync.EmitAsync();
    _onItemClicked.Emit();

    if(!indexChanged)
        return;

    _onIndexChangedAsync.EmitAsync();
    _onIndexChanged.Emit();
}

void RetroFuturaGUI::ComboBox::SetArrowDownColors(std::span<glm::vec4> colors, const ColorState state)
{
    _arrowDownColorState = state;

    switch(state)
    {
        case ColorState::Enabled:
            _arrowDownColorsEnabled.assign(colors.begin(), colors.end());
        break;
        case ColorState::Clicked:
            _arrowDownColorsClicked.assign(colors.begin(), colors.end());
        break;
        case ColorState::Hover:
            _arrowDownColorsHover.assign(colors.begin(), colors.end());
        break;
        default: //disabled
            _arrowDownColorsDisabled.assign(colors.begin(), colors.end());
    }

    setArrowDownColors();
}

void RetroFuturaGUI::ComboBox::SetDropDownBackgroundColors(std::span<glm::vec4> colors, const ColorState state)
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

void RetroFuturaGUI::ComboBox::SetDropDownBorderColors(std::span<glm::vec4> colors, const ColorState state)
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

void RetroFuturaGUI::ComboBox::setArrowDownColors()
{
    if(!_arrowDown)
        return;

    switch(_arrowDownColorState)
    {
        case ColorState::Enabled:
            _arrowDownPathFill.colors = _arrowDownColorsEnabled;
        break;
        case ColorState::Clicked:
            _arrowDownPathFill.colors = _arrowDownColorsClicked;
        break;
        case ColorState::Hover:
            _arrowDownPathFill.colors = _arrowDownColorsHover;
        break;
        default: //Disabled
            _arrowDownPathFill.colors = _arrowDownColorsDisabled;
    }
    
    if(_arrowDown)
        _arrowDown->SetPathFill("arrowDown", _arrowDownPathFill);
}

void RetroFuturaGUI::ComboBox::setDropDownBackgroundColors()
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

void RetroFuturaGUI::ComboBox::setDropDownBorderColors()
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

void RetroFuturaGUI::ComboBox::setColors(const ColorState state)
{
    _backgroundColorState = state;
    _borderColorState = state;
    _arrowDownColorState = state;
    setBackgroundColors();
    setBorderColors();
    setArrowDownColors();
}

void RetroFuturaGUI::ComboBox::SetDropDownCornerRadii(const glm::vec4& radii)
{
    if(_dropDownBackground)
        _dropDownBackground->SetCornerRadii(radii);

    if(_dropDownBorder)
        _dropDownBorder->SetCornerRadii(radii);
}

void RetroFuturaGUI::ComboBox::SetFontFamily(std::string_view fontFamily, const f32 fontSize, const PlatformBridge::Fonts::Slant slant, const PlatformBridge::Fonts::Weight fontWeight)
{
    _fontFamily = fontFamily;
    _fontSize = fontSize;
    _slant = slant;
    _fontWeight = fontWeight;

    for(const auto& item : _itemTexts)
    {
        item->SetFontFamily(_fontFamily, _fontSize, _slant, _fontWeight);
        item->SetTextUTF8(std::string(item->GetTextUTF8())); //a font swap leaves the glyph mesh behind, so the text is laid out again
    }
}

void RetroFuturaGUI::ComboBox::SetTextAlignment(const TextAlignment alignment)
{
    _textAlignment = alignment;
    updateItemGeometry();
}

void RetroFuturaGUI::ComboBox::SetTextPadding(const f32 padding)
{
    _textPadding = padding;
    updateItemGeometry();
}