#include "Tab.hpp"
#include <algorithm>

RetroFuturaGUI::Tab::Tab(std::string_view name, Projection* projection, IHierarchyNode* parentWidget, const WidgetTypeID parentWidgetTypeID, GLFWwindow* parentWindow)
    : ITab(name, projection, parentWidget, parentWidgetTypeID, parentWindow)
{
    _widgetTypeID = WidgetTypeID::Tab;
}

void RetroFuturaGUI::Tab::Draw()
{
    syncFieldRects(); //again, since the pointer may have moved, removed or added a field since
    _background.Draw();
    _border.Draw();
    drawFields();
    drawSelectedPage();
}

bool RetroFuturaGUI::Tab::RemovePage(std::string_view name)
{
    const uSize index { indexOf(name) };

    if(_kNoField == index)
        return false;

    removePage(index);
    return true;
}

uSize RetroFuturaGUI::Tab::GetPageCount() const
{
    return _fields.size();
}

bool RetroFuturaGUI::Tab::SelectPage(std::string_view name, const bool emitSignal)
{
    const uSize index { indexOf(name) };

    if(_kNoField == index)
        return false;

    selectField(index, emitSignal);
    return true;
}

RetroFuturaGUI::IWidget* RetroFuturaGUI::Tab::GetSelectedPage() const
{
    return static_cast<IWidget*>(_selectedItem);
}

bool RetroFuturaGUI::Tab::SetPageTitle(std::string_view name, std::string_view title)
{
    const uSize index { indexOf(name) };

    if(_kNoField == index)
        return false;

    setFieldTitle(index, title);
    return true;
}

const std::string& RetroFuturaGUI::Tab::GetPageTitle(std::string_view name) const
{
    static const std::string none {};
    const uSize index { indexOf(name) };

    if(_kNoField == index)
        return none;

    return _fields[index]._Title;
}

void RetroFuturaGUI::Tab::Connect_OnSelectionChanged(const typename Signal<IWidget*>::Slot& slot, const bool async)
{
    if(async)
        _onSelectionChangedAsync.Connect(slot);
    else
        _onSelectionChanged.Connect(slot);
}

void RetroFuturaGUI::Tab::Disconnect_OnSelectionChanged(const typename Signal<IWidget*>::Slot& slot)
{
    _onSelectionChanged.Disconnect(slot);
    _onSelectionChangedAsync.Disconnect(slot);
}

void RetroFuturaGUI::Tab::Connect_OnPageAdded(const typename Signal<IWidget*>::Slot& slot, const bool async)
{
    if(async)
        _onPageAddedAsync.Connect(slot);
    else
        _onPageAdded.Connect(slot);
}

void RetroFuturaGUI::Tab::Disconnect_OnPageAdded(const typename Signal<IWidget*>::Slot& slot)
{
    _onPageAdded.Disconnect(slot);
    _onPageAddedAsync.Disconnect(slot);
}

void RetroFuturaGUI::Tab::Connect_OnPageRemoved(const typename Signal<IWidget*>::Slot& slot, const bool async)
{
    if(async)
        _onPageRemovedAsync.Connect(slot);
    else
        _onPageRemoved.Connect(slot);
}

void RetroFuturaGUI::Tab::Disconnect_OnPageRemoved(const typename Signal<IWidget*>::Slot& slot)
{
    _onPageRemoved.Disconnect(slot);
    _onPageRemovedAsync.Disconnect(slot);
}

void RetroFuturaGUI::Tab::ShowCloseButtons(const bool show)
{
    _areCloseButtonsShown = show;
    _hoveredCloseButton = _kNoField;
    _pressedCloseButton = _kNoField;
}

bool RetroFuturaGUI::Tab::AreCloseButtonsShown() const
{
    return _areCloseButtonsShown;
}

void RetroFuturaGUI::Tab::addPage(std::unique_ptr<IWidget> page, std::string_view title)
{
    if(!page)
        return;

    IWidget* addedPage { page.get() };
    _pages.push_back(std::move(page));
    insertField(addedPage, title, _fields.size());
    layoutPage(*addedPage);
    _onPageAdded.Emit(addedPage);
    _onPageAddedAsync.EmitAsync(addedPage);

    if(_selectedItem)
        return;

    selectField(_fields.size() - 1, true); //the first page shows at once.
}

void RetroFuturaGUI::Tab::removePage(const uSize index)
{
    IWidget* removed { pageOf(index) };

    if(!removed)
        return;

    //Kept alive until the next Interact, since one of its own widgets' slots may be running right now
    const auto owned { std::find_if(_pages.begin(), _pages.end(), [removed](const std::unique_ptr<IWidget>& page) { return page.get() == removed; }) };

    if(_pages.end() != owned)
    {
        _removedPages.push_back(std::move(*owned));
        _pages.erase(owned);
    }

    const bool wasSelected { eraseField(index) };
    _onPageRemoved.Emit(removed);
    _onPageRemovedAsync.EmitAsync(removed);

    if(!wasSelected)
        return;

    reselectAfterErase(index);
}

void RetroFuturaGUI::Tab::layoutPages()
{
    for(const std::unique_ptr<IWidget>& page : _pages)
    {
        if(!page)
            continue;

        layoutPage(*page);
    }
}

void RetroFuturaGUI::Tab::layoutPage(IWidget& page) const
{
    const Transitions::InstantScope instant { true }; //layout moves at once, as in every container
    const glm::vec4 content { getContentRect() };
    page.SetSize(glm::vec3(content.z, content.w, _size.z));
    page.SetPosition(glm::vec3(content.x + content.z * 0.5f, content.y + content.w * 0.5f, _position.z + _zStep));
    page.SetRotation(glm::vec3(0.0f)); //the Tab's frame turns it with the Tab.
}

RetroFuturaGUI::IWidget* RetroFuturaGUI::Tab::pageOf(const uSize index) const
{
    if(index >= _fields.size())
        return nullptr;

    return static_cast<IWidget*>(_fields[index]._Item);
}

uSize RetroFuturaGUI::Tab::indexOf(std::string_view name) const
{
    for(uSize index = 0; index < _fields.size(); ++index)
    {
        const IWidget* page { pageOf(index) };

        if(!page)
            continue;

        if(page->GetName() == name)
            return index;
    }

    return _kNoField;
}

void RetroFuturaGUI::Tab::interact()
{
    _removedPages.clear();
    syncFieldRects(); //the pointer meets the fields where they are drawn
    followPointer();
    IWidget* selectedPage { GetSelectedPage() };

    if(!selectedPage)
        return;

    selectedPage->Interact(); //nothing of this Tab is used after it. The page's widgets may remove it
}

void RetroFuturaGUI::Tab::update()
{
    //By index, since an page's slot may add another one, which can move the fields
    for(uSize index = 0; index < _fields.size(); ++index)
    {
        IWidget* page { pageOf(index) };

        if(!page)
            continue;

        page->Update();
    }
}

void RetroFuturaGUI::Tab::drawSelectedPage()
{
    IWidget* selectedPage { GetSelectedPage() };

    if(!selectedPage)
        return;

    selectedPage->DrawAtOpacity();
}

void RetroFuturaGUI::Tab::fieldSelected(IHierarchyNode* item, const bool emitSignal)
{
    if(!emitSignal)
        return;

    IWidget* page { static_cast<IWidget*>(item) };
    _onSelectionChanged.Emit(page);
    _onSelectionChangedAsync.EmitAsync(page);
}

void RetroFuturaGUI::Tab::selectionCleared()
{
    _onSelectionChanged.Emit(nullptr);
    _onSelectionChangedAsync.EmitAsync(nullptr);
}

void RetroFuturaGUI::Tab::closeField(const uSize index)
{
    removePage(index);
}

bool RetroFuturaGUI::Tab::showsCloseButtons() const
{
    return _areCloseButtonsShown;
}

void RetroFuturaGUI::Tab::contentAreaChanged()
{
    layoutPages();
}

void RetroFuturaGUI::Tab::contentCornerRadiiChanged()
{
    //Clockwise from the top-left, so z and w are the bottom corners; the top ones meet the strip.
    const glm::vec4 radii { 0.0f, 0.0f, _contentCornerRadii.z, _contentCornerRadii.w };
    _background.SetBuiltInCornerRadii(styleKey(), radii);
    _border.SetBuiltInCornerRadii(styleKey(), radii);
}
