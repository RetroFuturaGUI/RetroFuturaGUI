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
    _background = std::make_unique<Rectangle>(projection);
    _border = std::make_unique<Rectangle>(projection);
    _previewText = std::make_shared<Text>(projection);

    if(_background)
        _background->SetRectangleMode(RectangleMode::Plane);

    if(_border)
        _border->SetRectangleMode(RectangleMode::Border);

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

    if(_previewText) //the rows are arbitrary widgets, so the closed box stands in for the selection with its own text
    {
        updatePreviewTextGeometry();
        _previewText->Draw();
    }

    if(!_showDropDown)
        return;

    if(_dropDownBackground)
        _dropDownBackground->Draw();

    if(_dropDownBorder)
        _dropDownBorder->Draw();

    updateItemGeometry(); //a move or a resize only reaches the panel, so every row is placed again before the list is drawn

    if(_dropDownItemHighlight && _dropDownItemHovered)
        _dropDownItemHighlight->Draw();

    uSize visibleIndex { 0 };

    for(const auto& item : _itemWidgets)
    {
        if(visibleIndex >= visibleItemCount())
            return;

        if(item)
            item->Draw();

        ++visibleIndex;
    }
}

void RetroFuturaGUI::ExtendedComboBox::placeItemWidget(IWidget* item, const glm::vec3& center) const
{
    const glm::vec2 available { contentSize() };
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

glm::vec2 RetroFuturaGUI::ExtendedComboBox::contentSize() const
{
    const f32
        boxBorder { _border ? _border->GetBorderWidth() : 0.0f },
        panelBorder { _dropDownBorder ? _dropDownBorder->GetBorderWidth() : 0.0f },
        inset { (boxBorder > panelBorder ? boxBorder : panelBorder) * 2.0f };

    return glm::vec2
    (
        _size.x - inset > 0.0f ? _size.x - inset : 0.0f,
        _size.y - inset > 0.0f ? _size.y - inset : 0.0f
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
        if(visibleIndex >= visibleItemCount())
            return false;

        ++visibleIndex;

        //only a row that is a drop down itself can have a list of its own standing outside this panel
        const IDropDown* _dropDownItem { dynamic_cast<const IDropDown*>(item.get()) };

        if(_dropDownItem && _dropDownItem->IsPointInsideOpenDropDown(point))
            return true;
    }

    return false;
}

void RetroFuturaGUI::ExtendedComboBox::updatePreviewTextGeometry() const
{
    if(!_previewText)
        return;

    //the same inset the rows get, so the closed box and the open list line their text up the same way
    _previewText->SetParentSize(contentSize());
    _previewText->SetPosition(_position + glm::vec3(0.0f, 0.0f, _zStep * 2.0f));
    _previewText->SetRotation(_rotation);
    _previewText->SetTextAlignment(_textAlignment);
    _previewText->SetTextPadding(_textPadding);
}

void RetroFuturaGUI::ExtendedComboBox::SetSize(const glm::vec3& size)
{
    IWidget::SetSize(size);

    if(_background)
        _background->SetSize(size);

    if(_border)
        _border->SetSize(size);

    updateDropDownGeometry();
}

void RetroFuturaGUI::ExtendedComboBox::SetPosition(const glm::vec3& position)
{
    IWidget::SetPosition(position);

    if(_background)
        _background->SetPosition(position);

    if(_border)
        _border->SetPosition(position + glm::vec3(0.0f, 0.0f, _zStep));

    updateDropDownGeometry();
}

void RetroFuturaGUI::ExtendedComboBox::SetRotation(const glm::vec3& rotation)
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

void RetroFuturaGUI::ExtendedComboBox::interact()
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
        isInsideDropDown { hasMousePosition && _showDropDown && isPointInsideDropDown(mousePos) },
        //a row can be a drop down itself, and the list it opens stands outside this panel
        isInsideItemDropDown { hasMousePosition && _showDropDown && isPointInsideItemDropDown(mousePos) };
    _wasClicked = isMousePressed;

    //the open dropDown belongs to the widget, so hovering a row, or a list that row opened, still counts as hovering the ComboBox
    const bool isHovering { _isEnabledFlag && (isInsideBox || isInsideDropDown || isInsideItemDropDown) };
    //a row's own list stands on top, so the highlight must not follow the pointer through it
    _dropDownItemHovered = isHovering && isInsideDropDown && !isInsideItemDropDown && hoveredItemIndex(mousePos, _nthItemHovered);

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
            _previewText->SetColor(_previewTextColorClicked);
        break;
        case ColorState::Hover:
            _previewText->SetColor(_previewTextColorHover);
        break;
        case ColorState::Disabled:
            _previewText->SetColor(_previewTextColorDisabled);
        break;
        default: //Enabled
            _previewText->SetColor(_previewTextColorEnabled);
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

void RetroFuturaGUI::ExtendedComboBox::SetPreviewTextColor(const glm::vec4& color, const ColorState state)
{
    switch(state)
    {
        case ColorState::Clicked:
            _previewTextColorClicked = color;
        break;
        case ColorState::Hover:
            _previewTextColorHover = color;
        break;
        case ColorState::Disabled:
            _previewTextColorDisabled = color;
        break;
        default: //Enabled
            _previewTextColorEnabled = color;
    }

    setPreviewTextColors();
}

glm::vec4 RetroFuturaGUI::ExtendedComboBox::GetPreviewTextColor(const ColorState state) const
{
    switch(state)
    {
        case ColorState::Clicked:
            return _previewTextColorClicked;
        case ColorState::Hover:
            return _previewTextColorHover;
        case ColorState::Disabled:
            return _previewTextColorDisabled;
        default: //Enabled
            return _previewTextColorEnabled;
    }
}

void RetroFuturaGUI::ExtendedComboBox::updateItemGeometry()
{
    if(_dropDownItemHighlight && _nthItemHovered < visibleItemCount())
        _dropDownItemHighlight->SetPosition(itemRowCenter(_nthItemHovered) - glm::vec3(0.0f, 0.0f, _zStep));

    uSize visibleIndex { 0 };

    for(const auto& item : _itemWidgets)
    {
        if(visibleIndex >= visibleItemCount())
            return;

        if(item)
            placeItemWidget(item.get(), itemRowCenter(visibleIndex));

        ++visibleIndex;
    }
}

void RetroFuturaGUI::ExtendedComboBox::RemoveItem(const uSize index)
{
    auto iterator = _itemWidgets.begin();
    std::advance(iterator, index);
    _itemWidgets.erase(iterator);
    _itemCount = _itemWidgets.size();
    updateDropDownGeometry();
}

void RetroFuturaGUI::ExtendedComboBox::SetFontFamily(std::string_view fontFamily, const f32 fontSize, const PlatformBridge::Fonts::Slant slant, const PlatformBridge::Fonts::Weight fontWeight)
{
    _fontFamily = fontFamily;
    _fontSize = fontSize;
    _slant = slant;
    _fontWeight = fontWeight;

    if(_previewText)
    {
        _previewText->SetFontFamily(_fontFamily, _fontSize, _slant, _fontWeight);
        _previewText->SetTextUTF8(std::string(_previewText->GetTextUTF8())); //a font swap leaves the glyph mesh behind, so the text is laid out again
    }

    for(const auto& item : _itemWidgets)
    {
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