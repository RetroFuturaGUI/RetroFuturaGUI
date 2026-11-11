#include "ContextMenu.hpp"
#include "Background.hpp"
#include "ExtendedComboBox.hpp"
#include "FontManager.hpp"
#include "WidgetText.hpp"
#include "IWidget.hpp"
#include "IncludeHelper.hpp"
#include "Label.hpp"
#include "PlatformBridge.hpp"
#include "EnvironmentView.hpp"
#include "Window.hpp"
#include "Prefab.hpp"
#include <algorithm>
#include <memory>
#include <print>
#include <string_view>

RetroFuturaGUI::ContextMenu::ContextMenu(Projection* projection, IWidget* widget)
    : _hostProjection(*projection)
{
    if(!projection)
        return;

    if(!widget)
        return;

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

    _hostWindow = _widget->GetParentWindow();
    buildItems(_hostWindow);
    useWidgetTransitions();

    _border.SetWidth(_defaultBorderWidth);
    _text.SetPadding(_defaultTextPadding);
    _text.SetColorState(ColorState::Enabled);
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

    if(!_items)
        return;

    const Shader::FrameScope flat { glm::mat4(1.0f), Shader::FrameComposition::Instead }; //the menu stays flat over a tilted widget

    if(_hasPendingItemClose)
    {
        _hasPendingItemClose = false;
        closeLists(*_items);
    }

    interact();

    //Fades in once shown and out once dismissed, over the widget's Opacity transition, and draws until it has faded out
    const f32 target { _isShown ? 1.0f : 0.0f };

    if(target != _opacity.GetTarget())
        _opacity.Set(target, Transitions::Resolve(_widget->_transitionSettings, TransitionProperty::Opacity));

    if(0.0f >= _opacity.Get())
    {
        std::erase_if(_shownContextMenus, [this](const auto& shown) { return this == shown.second; });
        return;
    }

    _shownContextMenus[_hostWindow] = this; //only where it opened

    if(!_isShown)
        return; //fading out, it takes no more input

    //Placed before the items take the input, so they test it where they are drawn, and at once, whatever transitions they have
    const Transitions::InstantScope instant { true };
    followHostWindow();
    applyDefaultFont();
    setPosition();
    _items->SetSize(glm::vec3(_background.GetSize(), 0.01f));
    _items->SetPosition(_background.GetPosition() + glm::vec3(0.0f, 0.0f, _zStep * 2.0f));
    if(_items->GetPreview().GetString() != _text.GetString())
        _items->GetPreview().SetString(_text.GetString(), false); //laid out only when it changed
    _items->Interact();
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
    const Shader::OpacityScope opacity { contextMenu->_opacity.Get() };
    contextMenu->draw();
}

void RetroFuturaGUI::ContextMenu::SetDefaultFontFamily(std::string_view fontFamily, const f32 fontSize, const FontSizeUnit sizeUnit, const PlatformBridge::Fonts::Slant slant, const PlatformBridge::Fonts::Weight fontWeight)
{
    _defaultFontFamily = fontFamily;
    _defaultFontSize = FontManager::ToPixels(fontSize, sizeUnit);
    _defaultFontSlant = slant;
    _defaultFontWeight = fontWeight;
    ++_defaultFontVersion;

    FontManager::GetFontInfo(fontFamily, fontSize, sizeUnit, slant, fontWeight);
}

void RetroFuturaGUI::ContextMenu::draw()
{
    if(!_items)
        return;

    followHostWindow();
    _items->Draw();
    IDropDown::DrawShown();
}

void RetroFuturaGUI::ContextMenu::AddItem(std::string_view itemName, std::string_view node, std::string_view imagePath, std::string_view text, std::string_view note)
{
    _entries.push_back(MenuEntry { ._IsNode = false, ._Name = std::string(itemName), ._Node = std::string(node), ._ImagePath = std::string(imagePath), ._Text = std::string(text), ._Note = std::string(note) });
    addEntry(_entries.back());
}

void RetroFuturaGUI::ContextMenu::AddNode(std::string_view newNodeName, std::string_view node)
{
    _entries.push_back(MenuEntry { ._IsNode = true, ._Name = std::string(newNodeName), ._Node = std::string(node), ._ImagePath = {}, ._Text = {}, ._Note = {} });
    addEntry(_entries.back());
}

void RetroFuturaGUI::ContextMenu::addEntry(const MenuEntry& entry)
{
    if(!_items)
        return;

    ExtendedComboBox* parentNode { iterateNode(_items.get(), entry._Node) };

    if(!parentNode)
        return;

    if(entry._IsNode)
    {
        ExtendedComboBox* node { parentNode->AddItem<ExtendedComboBox>(entry._Name) };

        if(!node)
            return;

        node->GetPreview().SetString(entry._Name, false);
        followNode(*node);
        return;
    }

    const std::string_view
        itemName { entry._Name },
        imagePath { entry._ImagePath },
        text { entry._Text },
        note { entry._Note };
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
        textLabel->GetText().SetString(text, false);

        if(!_fontFamily.empty())
            textLabel->GetText().SetFontFamily(_fontFamily, _fontSize, FontSizeUnit::Pixels, _fontSlant, _fontWeight);
    }

    Label* noteLabel { item->AddWidget<Label>("Note", { ._Column = 2 }) };

    if(noteLabel)
    {
        noteLabel->GetText().SetString(note, false);

        if(!_fontFamily.empty())
            noteLabel->GetText().SetFontFamily(_fontFamily, _fontSize, FontSizeUnit::Pixels, _fontSlant, _fontWeight);
    }
}

void RetroFuturaGUI::ContextMenu::Connect_OnItemClicked(const typename Signal<std::vector<uSize>>::Slot& slot, const bool async)
{
    if(async)
        _onItemClickedAsync.Connect(slot);
    else
        _onItemClicked.Connect(slot);
}

void RetroFuturaGUI::ContextMenu::Disconnect_OnItemClicked(const typename Signal<std::vector<uSize>>::Slot& slot)
{
    _onItemClicked.Disconnect(slot);
    _onItemClickedAsync.Disconnect(slot);
}

const std::vector<uSize>& RetroFuturaGUI::ContextMenu::GetClickedPath() const
{
    return _clickedPath;
}

void RetroFuturaGUI::ContextMenu::followNode(ExtendedComboBox& node)
{
    //The node lives as long as the menu, which owns it through the root list
    node.Connect_OnItemClicked([this, &node]() { takeItemClick(node); }, false);
}

void RetroFuturaGUI::ContextMenu::takeItemClick(ExtendedComboBox& node)
{
    if(!_items)
        return;

    const uSize index { node.GetSelectedIndex() };
    const std::list<std::unique_ptr<IWidget>>& rows { node.GetItems() };

    if(index >= rows.size())
        return;

    const IWidget* row { std::next(rows.begin(), static_cast<i64>(index))->get() };

    if(!row)
        return; //a row removed this frame.

    if(WidgetTypeID::ExtendedComboBox == row->GetWidgetTypeID())
        return; //a submenu's row only opens its list.

    std::vector<uSize> path {};

    if(!findPath(*_items, node, path))
        return;

    path.push_back(index);
    _clickedPath = path;

    //Its lists close at the next Update, since the node that reported is still taking the input
    _isShown = false;
    _hasPendingItemClose = true;
    _onItemClickedAsync.EmitAsync(path);
    _onItemClicked.Emit(std::move(path));
}

bool RetroFuturaGUI::ContextMenu::findPath(const ExtendedComboBox& list, const ExtendedComboBox& node, std::vector<uSize>& path)
{
    if(&list == &node)
        return true;

    uSize index { 0 };

    for(const auto& row : list.GetItems())
    {
        const ExtendedComboBox* childNode { dynamic_cast<const ExtendedComboBox*>(row.get()) };

        if(childNode)
        {
            path.push_back(index);

            if(findPath(*childNode, node, path))
                return true;

            path.pop_back();
        }

        ++index;
    }

    return false;
}

void RetroFuturaGUI::ContextMenu::closeLists(ExtendedComboBox& node)
{
    node.SetDropDownOpen(false);

    for(const auto& row : node.GetItems())
    {
        ExtendedComboBox* childNode { dynamic_cast<ExtendedComboBox*>(row.get()) };

        if(!childNode)
            continue;

        closeLists(*childNode);
    }
}

void RetroFuturaGUI::ContextMenu::open(const PointerState& pointer)
{
    //At the cursor in the window it is in: through a view, the view's window, not the world's
    const std::optional<EnvironmentView::ViewPointer>& viewPointer { EnvironmentView::getViewPointer() };
    const bool isThroughView { viewPointer && _widget->isInside(viewPointer->_World) };
    _hostWindow = isThroughView ? viewPointer->_Window : _widget->GetParentWindow();
    _pointerPosition = isThroughView ? viewPointer->_ViewWindowPoint : pointer._WindowPosition;
    _depth = isThroughView ? 0.0f : _widget->GetPosition().z;

    if(_hostWindow != _itemsWindow)
        buildItems(_hostWindow); //its rows read the mouse of the window they belong to

    followHostWindow();
    _isShown = true;

    if(!_items)
        return;

    _items->SetDropDownOpen(true);
}

void RetroFuturaGUI::ContextMenu::buildItems(GLFWwindow* window)
{
    //Under the window itself rather than the widget, so only that window's pointer filter applies: not those of the
    //containers around the widget, nor a world's the widget is seen in through a view
    Window* host { Window::FromGlfwWindow(window) };
    _items = std::make_unique<ExtendedComboBox>(_rootNodeName, &_hostProjection, host, WidgetTypeID::Window, window);
    _itemsWindow = window;

    if(!_items)
        return;

    _items->SetLabelHiddenWhenOpen(true);
    _items->SetTransition(TransitionProperty::Opacity, Transition {}); //its list fades with the menu (not on its own as well)
    followNode(*_items);

    for(const MenuEntry& entry : _entries)
        addEntry(entry);

    if(_fontFamily.empty())
        return;

    _items->SetFontFamily(_fontFamily, _fontSize, FontSizeUnit::Pixels, _fontSlant, _fontWeight);
    setItemFont(_items.get());
}

void RetroFuturaGUI::ContextMenu::followHostWindow()
{
    const Window* host { Window::FromGlfwWindow(_hostWindow) };

    if(!host)
        return;

    const Projection* projection { host->GetProjection() };

    if(!projection)
        return;

    _hostProjection = *projection;
}

bool RetroFuturaGUI::ContextMenu::isPointerOverMenu() const
{
    if(!_items)
        return false;

    const PointerState menuPointer { _items->readPointer() }; //its rows' window's

    if(!menuPointer._HasPosition)
        return false;

    return _items->isPointInside(menuPointer._WindowPosition) || _items->IsPointInsideOpenDropDown(menuPointer._WindowPosition);
}

void RetroFuturaGUI::ContextMenu::hide()
{
    _isShown = false;

    if(!_items)
        return;

    closeLists(*_items);
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

void RetroFuturaGUI::ContextMenu::SetFontFamily(std::string_view fontFamily, const f32 fontSize, const FontSizeUnit sizeUnit, const PlatformBridge::Fonts::Slant slant, const PlatformBridge::Fonts::Weight fontWeight)
{
    _hasOwnFont = true;
    _fontFamily = fontFamily;
    _fontSize = FontManager::ToPixels(fontSize, sizeUnit);
    _fontSlant = slant;
    _fontWeight = fontWeight;
    _text.SetFontFamily(fontFamily, fontSize, sizeUnit, slant, fontWeight);

    if(_items)
        _items->SetFontFamily(fontFamily, fontSize, sizeUnit, slant, fontWeight);

    setItemFont(_items.get());
    setSize();
}

void RetroFuturaGUI::ContextMenu::useWidgetTransitions()
{
    if(!_widget)
        return;

    _background.SetTransitionSettings(&_widget->_transitionSettings);
    _border.SetTransitionSettings(&_widget->_transitionSettings);
    _text.SetTransitionSettings(&_widget->_transitionSettings);
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
    _text.SetFontFamily(_fontFamily, _fontSize, FontSizeUnit::Pixels, _fontSlant, _fontWeight);

    if(_items)
        _items->SetFontFamily(_fontFamily, _fontSize, FontSizeUnit::Pixels, _fontSlant, _fontWeight);

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
                textLabel->GetText().SetFontFamily(_fontFamily, _fontSize, FontSizeUnit::Pixels, _fontSlant, _fontWeight);

            Label* noteLabel { prefab->GetChildWidget<Label>("Note") };

            if(noteLabel)
                noteLabel->GetText().SetFontFamily(_fontFamily, _fontSize, FontSizeUnit::Pixels, _fontSlant, _fontWeight);
        }
        else if(WidgetTypeID::ExtendedComboBox == item->GetWidgetTypeID())
        {
            ExtendedComboBox* childComboBox { dynamic_cast<ExtendedComboBox*>(item.get()) };

            if(!childComboBox)
                continue;

            childComboBox->SetFontFamily(_fontFamily, _fontSize, FontSizeUnit::Pixels, _fontSlant, _fontWeight);
            setItemFont(childComboBox);
        }
    }
}

void RetroFuturaGUI::ContextMenu::setSize()
{
    const f32
        padding { _text.GetPadding() },
        borderWidth { _border.GetWidth() };
    const glm::vec2
        textArea { _text.GetWidth() + padding * 2.0f, _text.GetElement().GetHeight() + padding * 2.0f },
        contentSize { std::max(textArea.x, 180.0f - borderWidth * 2.0f), std::max(textArea.y, 28.0f - borderWidth * 2.0f) },
        size { contentSize + glm::vec2(borderWidth * 2.0f) };
    _text.SetSize(contentSize);
    _background.SetSize(glm::vec3(size, 0.01f));
    _border.SetSize(size);
}

void RetroFuturaGUI::ContextMenu::setPosition()
{
    if(!_widget)
        return;

    const glm::vec2
        halfSize { _background.GetSize() * 0.5f },
        resolution { _hostProjection.GetResolution() };

    // Centered above the cursor's tip and kept inside the window: it slides sideways at the left and right edges
    // and down at the top one. World space runs bottom-up, so above is +Y
    // Not std::clamp: a ContextMenu larger than the window would make it undefined
    const glm::vec3 center {
        std::max(halfSize.x, std::min(_pointerPosition.x, resolution.x - halfSize.x)),
        std::max(halfSize.y, std::min(_pointerPosition.y + _cursorDistance + halfSize.y, resolution.y - halfSize.y)),
        _depth };

    _background.SetPosition(center);
    _border.SetPosition(center + glm::vec3(0.0f, 0.0f, _zStep));
    _text.SetPosition(center + glm::vec3(0.0f, 0.0f, _zStep * 2.0f));
}

void RetroFuturaGUI::ContextMenu::interact()
{
    if(!_widget)
        return;

    const PointerState pointer { _widget->readPointer() }; //through a view the widget is seen in, if the cursor is over one
    const bool rightButtonPressedThisFrame { pointer._RightButtonPressed && !_wasRightButtonPressed };
    _wasRightButtonPressed = pointer._RightButtonPressed;
    const bool
        isOverWidget { pointer._HasPosition && !pointer._IsCaptured && _widget->isPointInside(pointer._Position) },
        isOverMenu { _isShown && isPointerOverMenu() }; //in the menu's own window, which may be a view's

    if(rightButtonPressedThisFrame && isOverWidget)
    {
        if(_isShown)
        {
            hide(); //a right-click on the widget again puts it away.
            return;
        }

        open(pointer);
        return;
    }

    if(!_isShown || isOverMenu)
        return;

    if(pointer._LeftButtonPressed || rightButtonPressedThisFrame)
        hide(); //a press anywhere else puts it away.
}

RetroFuturaGUI::Background& RetroFuturaGUI::ContextMenu::GetBackground()
{
    return _background;
}

const RetroFuturaGUI::Background& RetroFuturaGUI::ContextMenu::GetBackground() const
{
    return _background;
}

RetroFuturaGUI::Border& RetroFuturaGUI::ContextMenu::GetBorder()
{
    return _border;
}

const RetroFuturaGUI::Border& RetroFuturaGUI::ContextMenu::GetBorder() const
{
    return _border;
}

RetroFuturaGUI::WidgetText& RetroFuturaGUI::ContextMenu::GetText()
{
    return _text;
}

const RetroFuturaGUI::WidgetText& RetroFuturaGUI::ContextMenu::GetText() const
{
    return _text;
}