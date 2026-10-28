#include "ExtendedComboBox.hpp"
#include "Image.hpp"

#if defined(TARGET_PLATFORM_LINUX)
    #define GLFW_EXPOSE_NATIVE_X11
#elif defined(TARGET_PLATFORM_WINDOWS)
    #define GLFW_EXPOSE_NATIVE_WIN32
#endif
#include <GLFW/glfw3native.h>

RetroFuturaGUI::ExtendedComboBox::ExtendedComboBox(std::string_view name, Projection* projection, IHierarchyNode* parentWidget, const WidgetTypeID parentWidgetTypeID, GLFWwindow* parentWindow)
    : IDropDown(name, projection, parentWidget, parentWidgetTypeID, parentWindow)
{
    _widgetTypeID = WidgetTypeID::ExtendedComboBox;
    _background = std::make_unique<Plane>(projection);
    _border = std::make_unique<Frame>(projection);
    _previewText = std::make_shared<Text>(projection);

    if(!_background)
        return;

    if(!_border)
        return;


    setColors(ColorState::Enabled);
}

void RetroFuturaGUI::ExtendedComboBox::SetDropDownPosition(const DropDownPosition position)
{
    _dropDownPosition = position;
    updateDropDownGeometry();
    updateItemGeometry();
}

void RetroFuturaGUI::ExtendedComboBox::Draw()
{
    performPendingRemoval(); //no slot of the removed items must run between frames

    if(!_background)
        return;

    if(!_border)
        return;

    if(!_previewText)
        return;

    interact();
    drawBackground();
    _border->Draw();
    drawDropDownArrow();

    //The rows are arbitrary widgets, so the closed box stands in for the selection with its own text
    updatePreviewTextGeometry();
    _previewText->Draw();

    if(!_showDropDown)
        return;

    showDropDownLater(); //drawn at the end of the frame, otherwise the widgets after this box would cover the list
}

void RetroFuturaGUI::ExtendedComboBox::drawDropDown()
{
    if(!_dropDownBackground)
        return;

    if(!_dropDownBorder)
        return;

    _dropDownBackground->Draw();
    _dropDownBorder->Draw();

    updateItemGeometry(); //a move or a resize only reaches the panel, so every row is placed again before the list is drawn

    if(_dropDownItemHighlight && _dropDownItemHovered)
        _dropDownItemHighlight->Draw();

    uSize visibleIndex { 0 };

    for(const auto& item : _itemWidgets)
    {
        if(visibleIndex >= getVisibleItemCount())
            return;

        if(item)
        {
            item->UpdateTooltip();
            item->Draw();
        }

        ++visibleIndex;
    }
}

void RetroFuturaGUI::ExtendedComboBox::placeItemWidget(IWidget* item, const glm::vec3& center) const
{
    const glm::vec2 available { contentSize(getDropDownWidth()) }; //the rows span the open list which can be wider than the box
    const f32
        parentWidth { available.x },
        parentHeight { available.y };

    switch(item->GetWidgetTypeID())
    {
        case WidgetTypeID::Label:
        {
            Label* label = dynamic_cast<Label*>(item);
            label->SetSize(glm::vec3(parentWidth, parentHeight, 0.01f));
            label->SetPosition(center);
            label->SetRotation(_rotation);
            label->SetTextAlignment(_textAlignment);
            label->SetTextPadding(_textPadding);
        } break;
        case WidgetTypeID::Button:
        {
            Button* button = dynamic_cast<Button*>(item);
            button->SetSize(glm::vec3(parentWidth, parentHeight, 0.01f));
            button->SetPosition(center);
            button->SetRotation(_rotation);
            button->SetTextAlignment(_textAlignment);
            button->SetTextPadding(_textPadding);
        } break;
        case WidgetTypeID::TextBox:
        {
            TextBox* textBox = dynamic_cast<TextBox*>(item);
            textBox->SetSize(glm::vec3(parentWidth, parentHeight, 0.01f));
            textBox->SetPosition(center);
            textBox->SetRotation(_rotation);
            textBox->SetTextAlignment(_textAlignment);
            textBox->SetTextPadding(_textPadding);
        } break;
        case WidgetTypeID::Image:
        {
            Image* image = dynamic_cast<Image*>(item);
            image->SetSize(glm::vec3(parentWidth, parentHeight, 0.01f));
            image->SetPosition(center);
            image->SetRotation(_rotation);
        } break;
        case WidgetTypeID::SvgImage:
        {
            SvgImage* svgImage = dynamic_cast<SvgImage*>(item);
            svgImage->SetSize(glm::vec3(parentWidth, parentHeight, 0.01f));
            svgImage->SetPosition(center);
            svgImage->SetRotation(_rotation);
        } break;
        case WidgetTypeID::CheckBox:
        {
            CheckBox* checkBox = dynamic_cast<CheckBox*>(item);
            checkBox->SetSize(glm::vec3(parentWidth, parentHeight, 0.01f));
            checkBox->SetPosition(center);
            checkBox->SetRotation(_rotation);
        } break;
        case WidgetTypeID::Slider:
        {
            Slider* slider = dynamic_cast<Slider*>(item);
            slider->SetSize(glm::vec3(parentWidth, parentHeight, 0.01f));
            slider->SetPosition(center);
            slider->SetRotation(_rotation);
        } break;
        case WidgetTypeID::ProgressBar:
        {
            ProgressBar* progressBar = dynamic_cast<ProgressBar*>(item);
            progressBar->SetSize(glm::vec3(parentWidth, parentHeight, 0.01f));
            progressBar->SetPosition(center);
            progressBar->SetRotation(_rotation);
        } break;
        case WidgetTypeID::Prefab:
        {
            Prefab* prefab = dynamic_cast<Prefab*>(item);
            prefab->SetSize(glm::vec3(parentWidth, parentHeight, 0.01f));
            prefab->SetPosition(center);
            prefab->SetRotation(_rotation);
        } break;
        case WidgetTypeID::ComboBox:
        {
            ComboBox* comboBox = dynamic_cast<ComboBox*>(item);
            comboBox->SetSize(glm::vec3(parentWidth, parentHeight, 0.01f));
            comboBox->SetPosition(center);
            comboBox->SetRotation(_rotation);
            comboBox->SetTextAlignment(_textAlignment);
            comboBox->SetTextPadding(_textPadding);
        } break;
        case WidgetTypeID::ExtendedComboBox:
        {
            ExtendedComboBox* extendedComboBox = dynamic_cast<ExtendedComboBox*>(item);
            extendedComboBox->SetSize(glm::vec3(parentWidth, parentHeight, 0.01f));
            extendedComboBox->SetPosition(center);
            extendedComboBox->SetRotation(_rotation);
            extendedComboBox->SetTextAlignment(_textAlignment);
            extendedComboBox->SetTextPadding(_textPadding);
        } break;
        default:
            [[unlikely]]
        break;
    }
}

glm::vec2 RetroFuturaGUI::ExtendedComboBox::contentSize(const f32 width) const
{
    const f32
        boxBorder { _border ? _border->GetBorderWidth() : 0.0f },
        panelBorder { _dropDownBorder ? _dropDownBorder->GetBorderWidth() : 0.0f },
        inset { (boxBorder > panelBorder ? boxBorder : panelBorder) * 2.0f };

    return glm::vec2
    (
        0.0f < width - inset ? width - inset : 0.0f,
        0.0f < _size.y - inset ? _size.y - inset : 0.0f
    );
}

bool RetroFuturaGUI::ExtendedComboBox::IsPointInsideOpenDropDown(const glm::vec2& point) const
{
    if(!_showDropDown)
        return false;

    return isPointInsideDropDown(point) || isPointInsideItemDropDown(point);
}

bool RetroFuturaGUI::ExtendedComboBox::isPointInsideItemDropDown(const glm::vec2& point) const
{
    uSize visibleIndex { 0 };

    for(const auto& item : _itemWidgets)
    {
        if(visibleIndex >= getVisibleItemCount())
            return false;

        ++visibleIndex;

        //only a row that is a drop down itself can have a list of its own standing outside this panel
        const IDropDown* dropDownItem { dynamic_cast<const IDropDown*>(item.get()) };

        if(dropDownItem && dropDownItem->IsPointInsideOpenDropDown(point))
            return true;
    }

    return false;
}

void RetroFuturaGUI::ExtendedComboBox::updatePreviewTextGeometry() const
{
    if(!_previewText)
        return;

    //the same inset the rows get, so the closed box and the open list line their text up the same way
    _previewText->SetParentSize(contentSize(_size.x));
    _previewText->SetPosition(_position + glm::vec3(0.0f, 0.0f, _zStep * 2.0f));
    _previewText->SetRotation(_rotation);
    _previewText->SetTextAlignment(_textAlignment);
    _previewText->SetTextPadding(_textPadding);
}

void RetroFuturaGUI::ExtendedComboBox::SetSize(const glm::vec3& size)
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

void RetroFuturaGUI::ExtendedComboBox::SetPosition(const glm::vec3& position)
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

void RetroFuturaGUI::ExtendedComboBox::SetRotation(const glm::vec3& rotation)
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

void RetroFuturaGUI::ExtendedComboBox::interact()
{
    const PointerState pointer { readPointer() };

    if(pointer._IsCaptured)
        return; //ScrollView above has taken the pointer

    const glm::vec2 mousePos { pointer._Position };
    const bool
        isMousePressed { pointer._LeftButtonPressed },
        pressedThisFrame { isMousePressed && !_wasClicked },
        releasedThisFrame { !isMousePressed && _wasClicked },
        isInsideBox { pointer._HasPosition && isPointInside(mousePos) },
        isInsideDropDown { pointer._HasPosition && _showDropDown && isPointInsideDropDown(mousePos) },
        //a row can be a drop down itself, and the list it opens stands outside this panel
        isInsideItemDropDown { pointer._HasPosition && _showDropDown && isPointInsideItemDropDown(mousePos) };
    _wasClicked = isMousePressed;

    //the open dropDown belongs to the widget, so hovering a row, or a list that row opened, still counts as hovering the ComboBox
    const bool isHovering { _isEnabledFlag && (isInsideBox || isInsideDropDown || isInsideItemDropDown) };
    //a row's own list stands on top, so the highlight must not follow the pointer through it
    _dropDownItemHovered = isHovering && isInsideDropDown && !isInsideItemDropDown && hoveredItemIndex(mousePos, _nthItemHovered);

    if(isHovering) // hover
    {
        if(isInsideBox || _dropDownItemHovered)
            requestCursorIcon(PlatformBridge::CursorIcon::Hand);

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

    /* The row's own list is what was pressed, so the row deals with it. This box stays out of the way: it neither
       selects nor toggles, and above all it does not read the press as one landing outside itself and close. */
    if(isInsideItemDropDown)
        return;

    //click
    _onClickAsync.EmitAsync();
    _onClick.Emit();
    setColors(ColorState::Clicked);

    if(_dropDownItemHovered) //picking a row selects it
    {
        selectItem(_nthItemHovered);
        //_showDropDown = false;
        _dropDownItemHovered = false;
        return;
    }

    _showDropDown = !_showDropDown;
}

void RetroFuturaGUI::ExtendedComboBox::selectItem(const uSize index)
{
    if(index >= _itemWidgets.size())
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

void RetroFuturaGUI::ExtendedComboBox::setColors(const ColorState state)
{
    IDropDown::setColors(state);
    _backgroundColorState = state;
    _borderColorState = state;
    _previewTextColorState = state;
    setBackgroundColors();
    setBorderColors();
    setPreviewTextColors();
}

void RetroFuturaGUI::ExtendedComboBox::setPreviewTextColors()
{
    if(!_previewText)
        return;

    switch(_previewTextColorState)
    {
        case ColorState::Clicked:
            _previewText->SetColors(_previewTextColorsClicked);
        break;
        case ColorState::Hover:
            _previewText->SetColors(_previewTextColorsHover);
        break;
        case ColorState::Disabled:
            _previewText->SetColors(_previewTextColorsDisabled);
        break;
        default: //Enabled
            _previewText->SetColors(_previewTextColorsEnabled);
    }
}

void RetroFuturaGUI::ExtendedComboBox::SetPreviewText(std::string_view text)
{
    if(!_previewText)
        return;

    _previewText->SetTextUTF8(text);
}

const std::string& RetroFuturaGUI::ExtendedComboBox::GetPreviewText() const
{
    if(!_previewText)
        return _emptyText;

    return _previewText->GetTextUTF8();
}

void RetroFuturaGUI::ExtendedComboBox::SetPreviewTextColors(std::span<const glm::vec4> colors, const ColorState state)
{
    switch(state)
    {
        case ColorState::Clicked:
            _previewTextColorsClicked.assign(colors.begin(), colors.end());
        break;
        case ColorState::Hover:
            _previewTextColorsHover.assign(colors.begin(), colors.end());
        break;
        case ColorState::Disabled:
            _previewTextColorsDisabled.assign(colors.begin(), colors.end());
        break;
        default: //Enabled
            _previewTextColorsEnabled.assign(colors.begin(), colors.end());
    }

    setPreviewTextColors();
}

const std::vector<glm::vec4>& RetroFuturaGUI::ExtendedComboBox::GetPreviewTextColors(const ColorState state) const
{
    switch(state)
    {
        case ColorState::Clicked:
            return _previewTextColorsClicked;
        case ColorState::Hover:
            return _previewTextColorsHover;
        case ColorState::Disabled:
            return _previewTextColorsDisabled;
        default: //Enabled
            return _previewTextColorsEnabled;
    }
}

void RetroFuturaGUI::ExtendedComboBox::updateItemGeometry()
{
    if(_dropDownItemHighlight && _nthItemHovered < getVisibleItemCount())
        _dropDownItemHighlight->SetPosition(itemRowCenter(_nthItemHovered) - glm::vec3(0.0f, 0.0f, _zStep));

    uSize visibleIndex { 0 };

    for(const auto& item : _itemWidgets)
    {
        if(visibleIndex >= getVisibleItemCount())
            return;

        if(item)
            placeItemWidget(item.get(), itemRowCenter(visibleIndex));

        ++visibleIndex;
    }
}

void RetroFuturaGUI::ExtendedComboBox::RemoveItem(const uSize index)
{
    if(index >= _itemWidgets.size())
        return;

    std::unique_ptr<IWidget>& item { *std::next(_itemWidgets.begin(), static_cast<i64>(index)) };

    if(!item)
        return;

    //Kept alive until the next Draw, since one of its own slots may be running right now!
    _removedWidgets.push_back(std::move(item));
}

bool RetroFuturaGUI::ExtendedComboBox::isItemNameTaken(std::string_view name) const
{
    for(const std::unique_ptr<IWidget>& item : _itemWidgets)
        if(item && name == item->GetName())
            return true;

    return false;
}

void RetroFuturaGUI::ExtendedComboBox::performPendingRemoval()
{
    if(_removedWidgets.empty())
        return;

    _removedWidgets.clear();
    std::erase_if(_itemWidgets, [](const std::unique_ptr<IWidget>& item) { return !item; });
    _itemCount = _itemWidgets.size();
    updateDropDownGeometry();
}

void RetroFuturaGUI::ExtendedComboBox::SetFontFamily(std::string_view fontFamily, const f32 fontSize, const PlatformBridge::Fonts::Slant slant, const PlatformBridge::Fonts::Weight fontWeight)
{
    if(!_previewText)
        return;

    _fontFamily = fontFamily;
    _fontSize = fontSize;
    _slant = slant;
    _fontWeight = fontWeight;

    _previewText->SetFontFamily(_fontFamily, _fontSize, _slant, _fontWeight);
    _previewText->SetTextUTF8(std::string(_previewText->GetTextUTF8())); //a font swap leaves the glyph mesh behind, so the text is laid out again

    for(const auto& item : _itemWidgets)
    {
        if(!item)
            continue;

        switch(item->GetWidgetTypeID())
        {
            case WidgetTypeID::Label:
            {
                Label* label = dynamic_cast<Label*>(item.get());
                label->SetFontFamily(_fontFamily, _fontSize, _slant, _fontWeight);
                label->SetText(label->GetText(), false);
            } break;
            case WidgetTypeID::Button:
            {
                Button* button = dynamic_cast<Button*>(item.get());
                button->SetFontFamily(_fontFamily, _fontSize, _slant, _fontWeight);
                button->SetText(button->GetText(), false);
            } break;
            case WidgetTypeID::TextBox:
            {
                TextBox* textBox = dynamic_cast<TextBox*>(item.get());
                textBox->SetFontFamily(_fontFamily, _fontSize, _slant, _fontWeight);
                textBox->SetValue<std::string_view>(textBox->GetText(), false);
            } break;
            /*case WidgetTypeID::Prefab:
            {
            
            } break;*/
            case WidgetTypeID::ComboBox:
            {
                ComboBox* comboBox = dynamic_cast<ComboBox*>(item.get());
                comboBox->SetFontFamily(_fontFamily, _fontSize, _slant, _fontWeight);
            } break;
            case WidgetTypeID::ExtendedComboBox:
            {
                ExtendedComboBox* extendedComboBox = dynamic_cast<ExtendedComboBox*>(item.get());
                extendedComboBox->SetFontFamily(_fontFamily, _fontSize, _slant, _fontWeight);
            } break;
            default:
                [[unlikely]]
            break;
        }
    }
}

void RetroFuturaGUI::ExtendedComboBox::SetTextAlignment(const TextAlignment alignment)
{
    _textAlignment = alignment;
    updateItemGeometry();
}

void RetroFuturaGUI::ExtendedComboBox::SetTextPadding(const f32 padding)
{
    _textPadding = padding;
    updateItemGeometry();
}