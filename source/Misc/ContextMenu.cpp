#include "ContextMenu.hpp"
#include "ExtendedComboBox.hpp"
#include "FontManager.hpp"
#include "ITextProperties.hpp"
#include "IWidget.hpp"
#include "IncludeHelper.hpp"
#include "Label.hpp"
#include "PlatformBridge.hpp"
#include "Prefab.hpp"
#include <algorithm>
#include <memory>
#include <print>
#include <string_view>

RetroFuturaGUI::ContextMenu::ContextMenu(Projection* projection, IWidget* widget)
{
    if(!projection)
        return;

    if(!widget)
        return;

    _projection = projection;
    _widget = widget;

    if(_defaultFontFamily.empty())
    {
        const auto& availableFonts { PlatformBridge::Fonts::GetFontProperties() };

        if(availableFonts.empty())
            std::println("ERROR::CONTEXTMENU::NO_FONTS_AVAILABLE");
        else
        {
            _defaultFontFamily = availableFonts.front()._Name;
            _defaultFontSlant = availableFonts.front()._Slant;
            _defaultFontWeight = availableFonts.front()._Weight;
            ++_defaultFontVersion;
        }
    }

    _items = std::make_unique<ExtendedComboBox>(_rootNodeName, _projection, _widget, _widget->GetWidgetTypeID(), _widget->GetParentWindow());

    if(_items)
    {
        _items->SetLabelHiddenWhenOpen(true);
        _items->Connect_OnItemClicked([this]()
        {
            if(!_items)
                return;

            _isShown = false;
            _hasPendingItemClose = true;
        }, false);
    }

    _background = std::make_unique<Plane>(_projection);
    _border = std::make_unique<Frame>(_projection);
    _text = std::make_unique<Text>(projection);

    if(!_background)
        return;

    if(!_border)
        return;

    if(!_text)
        return;

    _border->SetBorderWidth(_defaultBorderWidth);
    _text->SetTextPadding(_defaultTextPadding);
    setBackgroundColors();
    setBorderColors();
    setTextColors();
    applyDefaultFont();
    setSize();
}

RetroFuturaGUI::ContextMenu::~ContextMenu()
{
    std::erase_if(_shownContextMenus, [this](const auto& shown) { return this == shown.second; });
}

void RetroFuturaGUI::ContextMenu::Update()
{
    if(!_widget)
        return;

    if(!_background)
        return;

    if(!_text)
        return;

    if(!_items)
        return;

    if(_hasPendingItemClose)
    {
        _hasPendingItemClose = false;
        _items->SetDropDownOpen(false);
    }

    interact();

    if(!_isShown)
    {
        std::erase_if(_shownContextMenus, [this](const auto& shown) { return this == shown.second; });
        return;
    }

    //Placed before the items take the input, so they test it where they are drawn
    applyDefaultFont();
    setPosition();
    _items->SetSize(glm::vec3(_background->GetSize(), 0.01f));
    _items->SetPosition(_background->GetPosition() + glm::vec3(0.0f, 0.0f, _zStep * 2.0f));
    _items->SetPreviewText(_text->GetTextUTF8());
    _items->Interact();

    _shownContextMenus[_widget->GetParentWindow()] = this;
}

void RetroFuturaGUI::ContextMenu::DrawShown(GLFWwindow* window)
{
    const auto shown { _shownContextMenus.find(window) };

    if(_shownContextMenus.end() == shown)
        return;

    ContextMenu* contextMenu { shown->second };
    _shownContextMenus.erase(shown);

    if(!contextMenu)
        return;

    glClear(GL_DEPTH_BUFFER_BIT);
    contextMenu->draw();
}

void RetroFuturaGUI::ContextMenu::SetDefaultFontFamily(std::string_view fontFamily, const f32 fontSize, const PlatformBridge::Fonts::Slant slant, const PlatformBridge::Fonts::Weight fontWeight)
{
    _defaultFontFamily = fontFamily;
    _defaultFontSize = fontSize;
    _defaultFontSlant = slant;
    _defaultFontWeight = fontWeight;
    ++_defaultFontVersion;

    FontManager::GetFontInfo(fontFamily, fontSize, slant, fontWeight);
}

void RetroFuturaGUI::ContextMenu::draw()
{
    if(!_items)
        return;

    _items->Draw();
    IDropDown::DrawShown();
}

void RetroFuturaGUI::ContextMenu::AddItem(std::string_view itemName, std::string_view node, std::string_view imagePath, std::string_view text, std::string_view note)
{
    if(!_items)
        return;

    ExtendedComboBox* parentNode { iterateNode(_items.get(), node) };

    if(!parentNode)
        return;

    const AxisDefinition itemLayout {
        ._RowDefinition = { 1.0f },
        ._ColumnDefinition = { 0.15f, 0.6f, 0.25f },
        ._LayerDefinition = { 1.0f }
    };
    Prefab* item { parentNode->AddItem<Prefab>(itemName, itemLayout) };

    if(!item)
        return;

    if(!imagePath.empty())
        item->AddWidget<SvgImage>("Image", { ._Column = 0 }, imagePath);

    Label* textLabel { item->AddWidget<Label>("Text", { ._Column = 1 }) };

    if(textLabel)
    {
        textLabel->SetText(text, false);

        if(!_fontFamily.empty())
            textLabel->SetFontFamily(_fontFamily, _fontSize, _fontSlant, _fontWeight);
    }

    Label* noteLabel { item->AddWidget<Label>("Note", { ._Column = 2 }) };

    if(noteLabel)
    {
        noteLabel->SetText(note, false);

        if(!_fontFamily.empty())
            noteLabel->SetFontFamily(_fontFamily, _fontSize, _fontSlant, _fontWeight);
    }
}

void RetroFuturaGUI::ContextMenu::AddNode(std::string_view newNodeName, std::string_view node)
{
    if(!_items)
        return;

    ExtendedComboBox* parentNode { iterateNode(_items.get(), node) };

    if(!parentNode)
        return;

    ExtendedComboBox* item { parentNode->AddItem<ExtendedComboBox>(newNodeName) };

    if(!item)
        return;

    item->SetPreviewText(newNodeName);
}

void RetroFuturaGUI::ContextMenu::Connect_OnItemClicked(const typename Signal<>::Slot& slot, const bool async)
{
    if(!_items)
        return;

    _items->Connect_OnItemClicked(slot, async);
}

uSize RetroFuturaGUI::ContextMenu::GetSelectedIndex() const
{
    if(!_items)
        return 0;

    return _items->GetSelectedIndex();
}

RetroFuturaGUI::ExtendedComboBox* RetroFuturaGUI::ContextMenu::iterateNode(ExtendedComboBox* comboBox, std::string_view node)
{
    if(!comboBox)
        return nullptr;

    if(comboBox->GetName() == node)
        return comboBox;

    if(comboBox->GetItems().empty())
        return nullptr;

    for(const auto& item : comboBox->GetItems())
    {
        if(!item)
            continue;

        if(WidgetTypeID::ExtendedComboBox != item->GetWidgetTypeID())
            continue;

        ExtendedComboBox* childNode { dynamic_cast<ExtendedComboBox*>(item.get()) };

        if(!childNode)
            continue;

        ExtendedComboBox* matchingNode { iterateNode(childNode, node) };

        if(matchingNode)
            return matchingNode;
    }

    return nullptr;
}

void RetroFuturaGUI::ContextMenu::SetBackgroundColors(std::span<const glm::vec4> colors)
{
    if(!_background)
        return;

    _backgroundColors.assign(colors.begin(), colors.end());
    _background->SetColors(_backgroundColors);
}

void RetroFuturaGUI::ContextMenu::SetBorderColors(std::span<const glm::vec4> colors)
{
    if(!_border)
        return;
    
    _borderColors.assign(colors.begin(), colors.end());
    _border->SetColors(_borderColors);
}

void RetroFuturaGUI::ContextMenu::SetCornerRadii(const glm::vec4& radii)
{
    if(!_background)
        return;
    
    if(!_border)
        return;
    
    _background->SetCornerRadii(radii);
    _border->SetCornerRadii(radii);
}

void RetroFuturaGUI::ContextMenu::SetText(std::string_view text, const bool emitSignal)
{
    ITextProperties::SetText(text, emitSignal);
    setSize();
}

void RetroFuturaGUI::ContextMenu::SetFontFamily(std::string_view fontFamily, const f32 fontSize, const PlatformBridge::Fonts::Slant slant, const PlatformBridge::Fonts::Weight fontWeight)
{
    _hasOwnFont = true;
    _fontFamily = fontFamily;
    _fontSize = fontSize;
    _fontSlant = slant;
    _fontWeight = fontWeight;
    ITextProperties::SetFontFamily(fontFamily, fontSize, slant, fontWeight);

    if(_items)
        _items->SetFontFamily(fontFamily, fontSize, slant, fontWeight);

    setItemFont(_items.get());
    setSize();
}

void RetroFuturaGUI::ContextMenu::applyDefaultFont()
{
    if(_hasOwnFont)
        return;

    if(_defaultFontVersion == _appliedDefaultFontVersion)
        return;

    _fontFamily = _defaultFontFamily;
    _fontSize = _defaultFontSize;
    _fontSlant = _defaultFontSlant;
    _fontWeight = _defaultFontWeight;
    ITextProperties::SetFontFamily(_fontFamily, _fontSize, _fontSlant, _fontWeight);

    if(_items)
        _items->SetFontFamily(_fontFamily, _fontSize, _fontSlant, _fontWeight);

    setItemFont(_items.get());
    setSize();
    _appliedDefaultFontVersion = _defaultFontVersion;
}

void RetroFuturaGUI::ContextMenu::setItemFont(ExtendedComboBox* comboBox) const
{
    if(!comboBox)
        return;

    if(_fontFamily.empty())
        return;

    for(const auto& item : comboBox->GetItems())
    {
        if(!item)
            continue;

        if(WidgetTypeID::Prefab == item->GetWidgetTypeID())
        {
            Prefab* prefab { dynamic_cast<Prefab*>(item.get()) };

            if(!prefab)
                continue;

            Label* textLabel { prefab->GetChildWidget<Label>("Text") };

            if(textLabel)
                textLabel->SetFontFamily(_fontFamily, _fontSize, _fontSlant, _fontWeight);

            Label* noteLabel { prefab->GetChildWidget<Label>("Note") };

            if(noteLabel)
                noteLabel->SetFontFamily(_fontFamily, _fontSize, _fontSlant, _fontWeight);
        }
        else if(WidgetTypeID::ExtendedComboBox == item->GetWidgetTypeID())
        {
            ExtendedComboBox* childComboBox { dynamic_cast<ExtendedComboBox*>(item.get()) };

            if(!childComboBox)
                continue;

            childComboBox->SetFontFamily(_fontFamily, _fontSize, _fontSlant, _fontWeight);
            setItemFont(childComboBox);
        }
    }
}

void RetroFuturaGUI::ContextMenu::SetTextPadding(const f32 padding)
{
    ITextProperties::SetTextPadding(padding);
    setSize(); //the padding is part of the box
}

void RetroFuturaGUI::ContextMenu::SetBorderWidth(const f32 borderWidth)
{
    IBorder::SetBorderWidth(borderWidth);
    setSize();
}

void RetroFuturaGUI::ContextMenu::setSize()
{
    if(!_text)
        return;

    if(!_background)
        return;

    if(!_border)
        return;

    const f32
        padding { _text->GetPadding() },
        borderWidth { _border->GetBorderWidth() };
    const glm::vec2
        textArea { _text->GetWidth() + padding * 2.0f, _text->GetHeight() + padding * 2.0f },
        contentSize { std::max(textArea.x, 180.0f - borderWidth * 2.0f), std::max(textArea.y, 28.0f - borderWidth * 2.0f) },
        size { contentSize + glm::vec2(borderWidth * 2.0f) };
    _text->SetParentSize(contentSize);
    _background->SetSize(size);
    _border->SetSize(size);
}

void RetroFuturaGUI::ContextMenu::setPosition()
{
    if(!_widget)
        return;

    if(!_projection)
        return;

    if(!_background)
        return;

    if(!_border)
        return;

    if(!_text)
        return;

    const glm::vec2
        halfSize { _background->GetSize() * 0.5f },
        resolution { _projection->GetResolution() };

    // Centered above the cursor's tip and kept inside the window: it slides sideways at the left and right edges
    // and down at the top one. World space runs bottom-up, so above is +Y.
    // Not std::clamp: a ContextMenu larger than the window would make it undefined.
    const glm::vec3 center {
        std::max(halfSize.x, std::min(_pointerPosition.x, resolution.x - halfSize.x)),
        std::max(halfSize.y, std::min(_pointerPosition.y + _cursorDistance + halfSize.y, resolution.y - halfSize.y)),
        _widget->GetPosition().z };

    _background->SetPosition(center);
    _border->SetPosition(center + glm::vec3(0.0f, 0.0f, _zStep));
    _text->SetPosition(center + glm::vec3(0.0f, 0.0f, _zStep * 2.0f));
}

void RetroFuturaGUI::ContextMenu::interact()
{
    if(!_widget)
        return;

    const PointerState pointer { _widget->readPointer() };
    const bool rightButtonPressedThisFrame { pointer._RightButtonPressed && !_wasRightButtonPressed };
    _wasRightButtonPressed = pointer._RightButtonPressed;

    if(!pointer._HasPosition)
        return;

    const bool isOverWidget { !pointer._IsCaptured && _widget->isPointInside(pointer._Position) };
    const bool isOverMenu {
        _isShown && _items
        && (_items->isPointInside(pointer._Position) || _items->IsPointInsideOpenDropDown(pointer._Position))
    };

    if(rightButtonPressedThisFrame && isOverWidget)
    {
        _pointerPosition = pointer._Position;
        _isShown = !_isShown;

        if(_items)
            _items->SetDropDownOpen(_isShown);
    }
    else if(_isShown
            && ((pointer._LeftButtonPressed && !isOverMenu)
                || (rightButtonPressedThisFrame && !isOverMenu)))
    {
        _isShown = false;

        if(_items)
            _items->SetDropDownOpen(false);
    }

}