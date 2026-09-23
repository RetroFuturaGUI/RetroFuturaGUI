#include "ComboBox.hpp"
#include "IWidget.hpp"
#include "IncludeHelper.hpp"
#include "Interfaces/IDropDown.hpp"
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
    : IDropDown(name, projection, parentWidget, parentWidgetTypeID, parentWindow)
{
    _widgetTypeID = WidgetTypeID::ComboBox;
    _background = std::make_unique<Rectangle>(projection);
    _border = std::make_unique<Rectangle>(projection);
    
    if(_background)
        _background->SetRectangleMode(RectangleMode::Plane);

    if(_border)
        _border->SetRectangleMode(RectangleMode::Border);

    setColors(ColorState::Enabled);
}

void RetroFuturaGUI::ComboBox::Draw()
{
    interact();

    if(_background)
        _background->Draw();

    if(_border)
        _border->Draw();

    if(_dropDownArrow)
    {
        updateDropDownArrowGeometry();
        _dropDownArrow->Draw();
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



void RetroFuturaGUI::ComboBox::placeItemText(Text& item, const glm::vec3& center) const
{
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
    _itemCount = _itemTexts.size();
    updateDropDownGeometry();
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

void RetroFuturaGUI::ComboBox::setColors(const ColorState state)
{
    IDropDown::setColors(state);
    _backgroundColorState = state;
    _borderColorState = state;
    setBackgroundColors();
    setBorderColors();
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

void RetroFuturaGUI::ComboBox::RemoveItem(const uSize index)
{
    auto iterator = _itemTexts.begin();
    std::advance(iterator, index);
    _itemTexts.erase(iterator);
    _itemCount = _itemTexts.size();
    updateDropDownGeometry();
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

void RetroFuturaGUI::ComboBox::SetDropDownPosition(const DropDownPosition position)
{
    _dropDownPosition = position;
    updateDropDownGeometry();
    updateItemGeometry();
}