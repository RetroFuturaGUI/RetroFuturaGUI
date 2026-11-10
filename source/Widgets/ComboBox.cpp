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
    useSubPartFlavors(_itemText, StylePart::ItemText);
    setColors(ColorState::Enabled);
}

void RetroFuturaGUI::ComboBox::Draw()
{
    updateItemColors(); //read every draw, so a changed color or a new hover shows at once

    if(!isLabelHidden())
    {
        _background.Draw();
        _border.Draw();
        drawDropDownArrow();

        if(_hasCaption && _previewText)
        {
            placeItemText(*_previewText, getLabelCenter(), getLabelWidth());
            _previewText->Draw();
        }
        else if(_currentIndex < _itemTexts.size())
        {
            auto iterator { _itemTexts.begin() };
            std::advance(iterator, _currentIndex);

            if(*iterator)
            {
                placeItemText(**iterator, getLabelCenter(), getLabelWidth());
                iterator->get()->Draw();
            }
        }
    }

    showDropDownLater(); //drawn at the end of the frame, or the widgets after this box would cover the list; also while it fades out
}

void RetroFuturaGUI::ComboBox::drawDropDown()
{
    _list.GetBackground().Draw();
    _list.GetBorder().Draw();

    updateItemGeometry(); //the selected item was just drawn over the closed box, so every row is placed again before the list is drawn

    if(_dropDownItemHovered)
        _list.GetHighlight().Draw();

    uSize
        itemIndex { 0 },
        row { 0 };

    for(const auto& item : _itemTexts)
    {
        if(getItemRow(itemIndex++, row) && item)
            item->Draw();
    }
}

void RetroFuturaGUI::ComboBox::applySize(const glm::vec3& size)
{
    IWidget::applySize(size);
    _background.SetSize(size);
    _border.SetSize(size);
    updateDropDownGeometry();
}



void RetroFuturaGUI::ComboBox::placeItemText(Text& item, const glm::vec3& center, const f32 width) const
{
    const f32
        boxBorder { _border.GetWidth() },
        panelBorder { _list.GetBorder().GetWidth() },
        inset { (boxBorder > panelBorder ? boxBorder : panelBorder) * 2.0f },
        parentWidth { 0.0f < width - inset ? width - inset : 0.0f },
        parentHeight { 0.0f < _size.y - inset ? _size.y - inset : 0.0f };

    item.SetParentSize(glm::vec2(parentWidth, parentHeight));
    item.SetPosition(center);
    item.SetTextAlignment(_textAlignment);
    item.SetTextPadding(_textPadding);
}

void RetroFuturaGUI::ComboBox::applyPosition(const glm::vec3& position)
{
    IWidget::applyPosition(position);
    _background.SetPosition(position);
    _border.SetPosition(position + glm::vec3(0.0f, 0.0f, _zStep));
    updateDropDownGeometry();
}

void RetroFuturaGUI::ComboBox::AddItem(std::string_view name)
{
    _itemTexts.emplace_back(std::make_unique<Text>(&_projection));
    _itemEnabledFlags.push_back(true);

    if(!_fontFamily.empty())
        _itemTexts.back()->SetFontFamily(_fontFamily, _fontSize, FontSizeUnit::Pixels, _slant, _fontWeight);

    _itemTexts.back()->SetTextUTF8(name);
    _itemTexts.back()->SetColors(_itemText.GetShownColors(ColorState::Enabled));
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
        _previewText->SetFontFamily(_fontFamily, _fontSize, FontSizeUnit::Pixels, _slant, _fontWeight);

    _previewText->SetTextUTF8(caption);
    _previewText->SetColors(_itemText.GetShownColors(ColorState::Enabled));
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

void RetroFuturaGUI::ComboBox::updateItemColors()
{
    uSize index { 0 };

    for(const auto& item : _itemTexts)
    {
        if(item)
            item->SetColors(_itemText.GetShownColors(itemState(index)));

        ++index;
    }

    if(!_previewText)
        return;

    _previewText->SetColors(_itemText.GetShownColors(_isEnabledFlag ? ColorState::Enabled : ColorState::Disabled)); //the caption
}

RetroFuturaGUI::ColorState RetroFuturaGUI::ComboBox::itemState(const uSize index) const
{
    if(!_isEnabledFlag || !IsItemEnabled(index))
        return ColorState::Disabled;

    if(!_showDropDown && _hasPickedItem && _pickedItemIndex == index)
        return ColorState::Clicked;

    if(_dropDownItemHovered && _nthItemHovered == index)
        return ColorState::Hover;

    return ColorState::Enabled;
}

RetroFuturaGUI::ItemTextStyle& RetroFuturaGUI::ComboBox::GetItemText()
{
    return _itemText;
}

const RetroFuturaGUI::ItemTextStyle& RetroFuturaGUI::ComboBox::GetItemText() const
{
    return _itemText;
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

    if(!_isEnabledFlag)
        _showDropDown = false;

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
    scrollListByWheel(pointer, isInsideDropDown); //before the hovered row is found, so it is the one now under the pointer

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
        _hasPickedItem = true;
        _pickedItemIndex = _nthItemHovered;
        selectItem(_nthItemHovered);
        _showDropDown = false;
        _dropDownItemHovered = false;
        _holdsPickPress = true; //a widget drawn after this box would take the same press for its own
        return;
    }

    if(isInsideBox)
        setListOpen(!_showDropDown);
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
    _background.SetColorState(state);
    _border.SetColorState(state);
}

void RetroFuturaGUI::ComboBox::updateItemGeometry()
{
    uSize row { 0 };

    if(getItemRow(_nthItemHovered, row))
        _list.GetHighlight().SetPosition(itemRowCenter(row) - glm::vec3(0.0f, 0.0f, _zStep));

    uSize itemIndex { 0 };

    for(const auto& item : _itemTexts)
    {
        if(getItemRow(itemIndex++, row) && item)
            placeItemText(*item, itemRowCenter(row), getDropDownWidth());
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

    //The items after it move up one
    if(_hasPickedItem && index <= _pickedItemIndex)
    {
        _hasPickedItem = index < _pickedItemIndex;
        _pickedItemIndex -= _hasPickedItem ? 1 : 0;
    }

    if(_dropDownItemHovered && index <= _nthItemHovered)
    {
        _dropDownItemHovered = index < _nthItemHovered;
        _nthItemHovered -= _dropDownItemHovered ? 1 : 0;
    }

    clampListScroll();
    updateDropDownGeometry();

    if(index > _currentIndex)
        return;

    if(index < _currentIndex || _currentIndex == _itemCount)
        _currentIndex = 0 < _currentIndex ? _currentIndex - 1 : 0;

    _onIndexChangedAsync.EmitAsync();
    _onIndexChanged.Emit();
}

void RetroFuturaGUI::ComboBox::SetFontFamily(std::string_view fontFamily, const f32 fontSize, const FontSizeUnit sizeUnit, const PlatformBridge::Fonts::Slant slant, const PlatformBridge::Fonts::Weight fontWeight)
{
    _fontFamily = fontFamily;
    _fontSize = FontManager::ToPixels(fontSize, sizeUnit);
    _slant = slant;
    _fontWeight = fontWeight;

    for(const auto& item : _itemTexts)
    {
        item->SetFontFamily(_fontFamily, _fontSize, FontSizeUnit::Pixels, _slant, _fontWeight);
        item->SetTextUTF8(std::string(item->GetTextUTF8())); //a font swap leaves the glyph mesh behind, so the text is laid out again
    }

    if(!_previewText)
        return;

    _previewText->SetFontFamily(_fontFamily, _fontSize, FontSizeUnit::Pixels, _slant, _fontWeight);
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

RetroFuturaGUI::Border& RetroFuturaGUI::ComboBox::GetBorder()
{
    return _border;
}

const RetroFuturaGUI::Border& RetroFuturaGUI::ComboBox::GetBorder() const
{
    return _border;
}