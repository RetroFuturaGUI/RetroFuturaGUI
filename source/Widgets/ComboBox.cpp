#include "ComboBox.hpp"
#include "IWidget.hpp"
#include "IncludeHelper.hpp"
#include "Interfaces/IDropDown.hpp"
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
    _background = std::make_unique<Plane>(projection);
    _border = std::make_unique<Frame>(projection);

    if(!_background)
        return;

    if(!_border)
        return;


    setColors(ColorState::Enabled);
}

void RetroFuturaGUI::ComboBox::Draw()
{
    if(!_background)
        return;

    if(!_border)
        return;

    if(!isLabelHidden())
    {
        drawBackground();
        _border->Draw();
        drawDropDownArrow();

        if(_hasCaption && _previewText)
        {
            placeItemText(*_previewText, _position + glm::vec3(0.0f, 0.0f, _zStep * 2.0f), _size.x);
            _previewText->Draw();
        }
        else if(_currentIndex < _itemTexts.size())
        {
            auto iterator { _itemTexts.begin() };
            std::advance(iterator, _currentIndex);

            if(*iterator)
            {
                placeItemText(**iterator, _position + glm::vec3(0.0f, 0.0f, _zStep * 2.0f), _size.x);
                iterator->get()->Draw();
            }
        }
    }

    if(!_showDropDown)
        return;

    showDropDownLater(); //drawn at the end of the frame, or the widgets after this box would cover the list
}

void RetroFuturaGUI::ComboBox::drawDropDown()
{
    if(!_dropDownBackground)
        return;

    if(!_dropDownBorder)
        return;

    _dropDownBackground->Draw();
    _dropDownBorder->Draw();

    updateItemGeometry(); //the selected item was just drawn over the closed box, so every row is placed again before the list is drawn

    if(_dropDownItemHighlight && _dropDownItemHovered)
        _dropDownItemHighlight->Draw();

    uSize visibleIndex { 0 };

    for(const auto& item : _itemTexts)
    {
        if(visibleIndex >= getVisibleItemCount())
            return;

        if(item)
            item->Draw();

        ++visibleIndex;
    }
}

void RetroFuturaGUI::ComboBox::SetSize(const glm::vec3& size)
{
    if(!_background)
        return;

    if(!_border)
        return;

    IWidget::SetSize(size);

    _background->SetSize(size);
    _border->SetSize(size);

    updateDropDownGeometry();
}



void RetroFuturaGUI::ComboBox::placeItemText(Text& item, const glm::vec3& center, const f32 width) const
{
    const f32
        boxBorder { _border ? _border->GetBorderWidth() : 0.0f },
        panelBorder { _dropDownBorder ? _dropDownBorder->GetBorderWidth() : 0.0f },
        inset { (boxBorder > panelBorder ? boxBorder : panelBorder) * 2.0f },
        parentWidth { 0.0f < width - inset ? width - inset : 0.0f },
        parentHeight { 0.0f < _size.y - inset ? _size.y - inset : 0.0f };

    item.SetParentSize(glm::vec2(parentWidth, parentHeight));
    item.SetPosition(center);
    item.SetRotation(_rotation);
    item.SetTextAlignment(_textAlignment);
    item.SetTextPadding(_textPadding);
}

void RetroFuturaGUI::ComboBox::SetPosition(const glm::vec3& position)
{
    if(!_background)
        return;

    if(!_border)
        return;

    IWidget::SetPosition(position);

    _background->SetPosition(position);
    _border->SetPosition(position + glm::vec3(0.0f, 0.0f, _zStep));

    updateDropDownGeometry();
}

void RetroFuturaGUI::ComboBox::SetRotation(const glm::vec3& rotation)
{
    if(!_background)
        return;

    if(!_border)
        return;

    if(!_dropDownBackground)
        return;

    if(!_dropDownBorder)
        return;

    if(!_dropDownItemHighlight)
        return;

    IWidget::SetRotation(rotation);

    _background->SetRotation(rotation);
    _border->SetRotation(rotation);
    _dropDownBackground->SetRotation(rotation);
    _dropDownBorder->SetRotation(rotation);
    _dropDownItemHighlight->SetRotation(rotation);

    updateDropDownGeometry();
}

void RetroFuturaGUI::ComboBox::AddItem(std::string_view name)
{
    _itemTexts.emplace_back(std::make_unique<Text>(&_projection));
    _itemEnabledFlags.push_back(true);

    if(!_fontFamily.empty())
        _itemTexts.back()->SetFontFamily(_fontFamily, _fontSize, _slant, _fontWeight);

    _itemTexts.back()->SetTextUTF8(name);
    _itemTexts.back()->SetColors(_itemTextColorsEnabled);
    _itemCount = _itemTexts.size();
    updateDropDownGeometry();
}

void RetroFuturaGUI::ComboBox::SetCaption(std::string_view caption)
{
    _hasCaption = !caption.empty();

    if(!_previewText)
        _previewText = std::make_shared<Text>(&_projection);

    if(!_previewText)
        return;

    if(!_fontFamily.empty())
        _previewText->SetFontFamily(_fontFamily, _fontSize, _slant, _fontWeight);

    _previewText->SetTextUTF8(caption);
    _previewText->SetColors(_itemTextColorsEnabled);
}

void RetroFuturaGUI::ComboBox::SetItemEnabled(const uSize index, const bool enable)
{
    if(_itemEnabledFlags.size() <= index)
        return;

    _itemEnabledFlags[index] = enable;
    updateItemColors();
}

bool RetroFuturaGUI::ComboBox::IsItemEnabled(const uSize index) const
{
    return index < _itemEnabledFlags.size() && _itemEnabledFlags[index];
}

void RetroFuturaGUI::ComboBox::SetItemTextColors(std::span<const glm::vec4> colors, const ColorState state)
{
    if(ColorState::Disabled == state)
        _itemTextColorsDisabled.assign(colors.begin(), colors.end());
    else
        _itemTextColorsEnabled.assign(colors.begin(), colors.end());

    updateItemColors();

    if(!_previewText)
        return;

    _previewText->SetColors(_itemTextColorsEnabled);
}

void RetroFuturaGUI::ComboBox::updateItemColors()
{
    uSize index { 0 };

    for(const auto& item : _itemTexts)
    {
        if(item)
            item->SetColors(IsItemEnabled(index) ? _itemTextColorsEnabled : _itemTextColorsDisabled);

        ++index;
    }
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
    const PointerState pointer { readPointer() };

    if(!pointer._LeftButtonPressed)
        _holdsPickPress = false;

    if(pointer._IsCaptured)
        return; //ScrollView above has taken the pointer

    const glm::vec2 mousePos { pointer._Position };
    const bool
        isMousePressed { pointer._LeftButtonPressed },
        pressedThisFrame { isMousePressed && !_wasClicked },
        releasedThisFrame { !isMousePressed && _wasClicked },
        isInsideBox { pointer._HasPosition && isPointInside(mousePos) },
        isInsideDropDown { pointer._HasPosition && _showDropDown && isPointInsideDropDown(mousePos) };
    _wasClicked = isMousePressed;

    //the open dropDown belongs to the widget, so hovering a row still counts as hovering the ComboBox. A disabled row is
    //neither highlighted nor picked, and a press on it leaves the list open.
    const bool isHovering { _isEnabledFlag && (isInsideBox || isInsideDropDown) };
    _dropDownItemHovered = isHovering && isInsideDropDown && hoveredItemIndex(mousePos, _nthItemHovered) && IsItemEnabled(_nthItemHovered);

    if(isHovering) // hover
    {
        if(isInsideBox || _dropDownItemHovered)
            requestCursorIcon(PlatformBridge::CursorIcon::Hand); //a disabled row can't be picked, so it keeps the arrow

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
        _holdsPickPress = true; //a widget drawn after this box would take the same press for its own
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
    if(_dropDownItemHighlight && _nthItemHovered < getVisibleItemCount())
        _dropDownItemHighlight->SetPosition(itemRowCenter(_nthItemHovered) - glm::vec3(0.0f, 0.0f, _zStep));

    uSize visibleIndex { 0 };

    for(const auto& item : _itemTexts)
    {
        if(visibleIndex >= getVisibleItemCount())
            return;

        if(item)
            placeItemText(*item, itemRowCenter(visibleIndex), getDropDownWidth());

        ++visibleIndex;
    }
}

void RetroFuturaGUI::ComboBox::RemoveItem(const uSize index)
{
    if(_itemTexts.size() <= index)
        return;

    auto iterator = _itemTexts.begin();
    std::advance(iterator, index);
    _itemTexts.erase(iterator);

    if(index < _itemEnabledFlags.size())
        _itemEnabledFlags.erase(_itemEnabledFlags.begin() + static_cast<std::ptrdiff_t>(index));

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

    if(!_previewText)
        return;

    _previewText->SetFontFamily(_fontFamily, _fontSize, _slant, _fontWeight);
    _previewText->SetTextUTF8(std::string(_previewText->GetTextUTF8()));
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