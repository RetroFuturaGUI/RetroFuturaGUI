#include "Tab.hpp"
#include "PlatformBridge.hpp"
#include "ResourceManager.hpp"
#include <algorithm>
#include <cmath>
#include <filesystem>
#include <span>

RetroFuturaGUI::Tab::Tab(std::string_view name, Projection* projection, IHierarchyNode* parentWidget, const WidgetTypeID parentWidgetTypeID, GLFWwindow* parentWindow)
    : IWidget(name, projection, parentWidget, parentWidgetTypeID, parentWindow)
{
    _widgetTypeID = WidgetTypeID::Tab;
    //The field elements draw once per field, in its colors, so they switch at once
    _background.SetTransitionSettings(&_transitionSettings);
    _border.SetTransitionSettings(&_transitionSettings);
    useBorderGap(&_background, &_border);
    useFlavors(&_background, &_border, nullptr);
    useSubPartFlavors(_fieldStyle.GetBackground(), StylePart::FieldStyleBackground);
    useSubPartFlavors(_fieldStyle.GetBorder(), StylePart::FieldStyleBorder);
    useSubPartFlavors(_closeButton.GetBackground(), StylePart::CloseButtonBackground);
    useSubPartFlavors(_closeButton.GetIcon(), StylePart::CloseButtonIcon);

    _border.SetBuiltInWidth(styleKey(), 0.0f); //no frame until one is asked for, since a Frame starts with a 5 px border
    _background.SetBuiltInColors(styleKey(), std::array { glm::vec4(0.055f, 0.055f, 0.075f, 1.0f) }, ColorState::Enabled);
    _background.SetBuiltInColors(styleKey(), std::array { glm::vec4(0.055f, 0.055f, 0.075f, 1.0f) }, ColorState::Disabled);
    _closeButton.GetBackground().SetBuiltInCornerRadii(styleKey(), glm::vec4(_closeButtonSize * 0.25f));
    setFieldStyleDefaults();
}

void RetroFuturaGUI::Tab::Draw()
{
    syncFieldRects(); //again, since the pointer may have moved, removed or added a field since
    _background.Draw();
    _border.Draw();
    drawFields();
    drawSelectedPage();
}

void RetroFuturaGUI::Tab::applyPosition(const glm::vec3& position)
{
    IWidget::applyPosition(position);
    _background.SetPosition(position);
    _border.SetPosition(position + glm::vec3(0.0f, 0.0f, _zStep * 8.0f)); //over the fields
    layoutPages();
}

void RetroFuturaGUI::Tab::applySize(const glm::vec3& size)
{
    IWidget::applySize(size);

    _background.SetSize(glm::vec2(size));
    _border.SetSize(glm::vec2(size));
    layoutPages();
}

bool RetroFuturaGUI::Tab::RemovePage(std::string_view name)
{
    const uSize index { indexOf(name) };

    if(_kNoField == index)
        return false;

    removeField(index);
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
    return _selectedPage;
}

bool RetroFuturaGUI::Tab::SetPageTitle(std::string_view name, std::string_view title)
{
    const uSize index { indexOf(name) };

    if(_kNoField == index)
        return false;

    Field& field { _fields[index] };
    field._Title = title;

    if(!field._TitleText)
        return true;

    field._TitleText->SetTextUTF8(title);
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

void RetroFuturaGUI::Tab::SetStripHeight(const f32 height)
{
    _stripHeight = std::max(height, 0.0f);
    layoutPages(); //the content area changed with it.
}

void RetroFuturaGUI::Tab::SetFieldWidthRange(const f32 minimumWidth, const f32 maximumWidth)
{
    _minimumFieldWidth = std::max(minimumWidth, 1.0f);
    _maximumFieldWidth = std::max(maximumWidth, _minimumFieldWidth);
}

void RetroFuturaGUI::Tab::SetFieldSpacing(const f32 spacing)
{
    _fieldSpacing = std::max(spacing, 0.0f);
}

void RetroFuturaGUI::Tab::SetTitleFontFamily(std::string_view fontFamily, const f32 fontSize, const FontSizeUnit sizeUnit, const PlatformBridge::Fonts::Slant slant, const PlatformBridge::Fonts::Weight weight)
{
    _titleFontFamily = fontFamily;
    _titleFontSize = FontManager::ToPixels(fontSize, sizeUnit);
    _titleFontSlant = slant;
    _titleFontWeight = weight;

    for(Field& field : _fields)
        applyTitleFont(field._TitleText.get());
}

f32 RetroFuturaGUI::Tab::GetTitleFontSize(const FontSizeUnit sizeUnit) const
{
    return FontManager::FromPixels(_titleFontSize, sizeUnit);
}

void RetroFuturaGUI::Tab::SetTitleColors(std::span<const glm::vec4> colors, const ColorState state)
{
    if(ColorState::Selected == state)
        return; //the selected page's title shows the same colors as the others

    colorOf(_titleColors, state).assign(colors.begin(), colors.end());
}

void RetroFuturaGUI::Tab::SetTitlePadding(const f32 padding)
{
    _titlePadding = std::max(padding, 0.0f);
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

void RetroFuturaGUI::Tab::SetCloseButtonSize(const f32 size)
{
    _closeButtonSize = std::max(size, 0.0f);

    //Rasterized at twice the size the cross draws at, so it stays smooth when it is scaled down
    const i32 resolution { std::max(static_cast<i32>(std::lround(_closeButtonSize * _closeIconScale * 2.0f)), 8) };
    _closeButton.GetIcon().Rasterize(glm::i32vec2(resolution));
    _closeButton.GetBackground().SetBuiltInCornerRadii(styleKey(), glm::vec4(_closeButtonSize * 0.25f));
}

void RetroFuturaGUI::Tab::addField(std::unique_ptr<IWidget> page, std::string_view title)
{
    if(!page)
        return;

    IWidget* addedPage { page.get() };
    std::unique_ptr<Text> titleText { std::make_unique<Text>(&_projection) };
    applyTitleFont(titleText.get());
    titleText->SetTextUTF8(title);
    _fields.push_back(Field { ._Page = std::move(page), ._Title = std::string(title), ._TitleText = std::move(titleText), ._Shown = {}, ._HasShown = false });
    layoutPage(*addedPage);
    _onPageAdded.Emit(addedPage);
    _onPageAddedAsync.EmitAsync(addedPage);

    if(_selectedPage)
        return;

    selectField(_fields.size() - 1, true); //the first page shows at once.
}

void RetroFuturaGUI::Tab::selectField(const uSize index, const bool emitSignal)
{
    if(index >= _fields.size())
        return;

    IWidget* page { _fields[index]._Page.get() };

    if(page == _selectedPage)
        return;

    _selectedPage = page;

    if(!emitSignal)
        return;

    _onSelectionChanged.Emit(page);
    _onSelectionChangedAsync.EmitAsync(page);
}

void RetroFuturaGUI::Tab::layoutPages()
{
    for(Field& field : _fields)
    {
        if(!field._Page)
            continue;

        layoutPage(*field._Page);
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

void RetroFuturaGUI::Tab::interact()
{
    _removedPages.clear();
    syncFieldRects(); //the pointer meets the fields where they are drawn
    followPointer();

    if(!_selectedPage)
        return;

    _selectedPage->Interact(); //nothing of this Tab is used after it. The page's widgets may remove it
}

void RetroFuturaGUI::Tab::update()
{
    //By index, since an page's slot may add another one, which can move the fields
    for(uSize index = 0; index < _fields.size(); ++index)
    {
        IWidget* page { _fields[index]._Page.get() };

        if(!page)
            continue;

        page->Update();
    }
}

void RetroFuturaGUI::Tab::followPointer()
{
    const PointerState pointer { readPointer() };
    const bool
        isPressed { pointer._LeftButtonPressed },
        pressedThisFrame { isPressed && !_wasPressed };
    _wasPressed = isPressed;

    if(!isPressed)
    {
        _pressedPage = nullptr;
        _isDraggingField = false;
        _pressedCloseButton = _kNoField;
    }

    if(pointer._IsCaptured || !pointer._HasPosition)
    {
        _hoveredField = _kNoField;
        _hoveredCloseButton = _kNoField;
        return; //a ScrollView above has taken the pointer, or a popup lies over the window
    }

    const glm::vec2 point { pointer._Position };
    _hoveredField = fieldAt(point);
    _hoveredCloseButton = _areCloseButtonsShown && _kNoField != _hoveredField && contains(closeButtonRect(fieldRect(_hoveredField)), point) ? _hoveredField : _kNoField;

    if(!_isEnabledFlag)
        return;

    if(pressedThisFrame)
    {
        //Like the page bar's close button, the cross removes on-press
        if(_kNoField != _hoveredCloseButton)
        {
            removeField(_hoveredCloseButton);
            return;
        }

        if(_kNoField != _hoveredField)
            pressField(_hoveredField, point);

        return;
    }

    if(!isPressed)
        return;

    dragField(point);
}

void RetroFuturaGUI::Tab::pressField(const uSize index, const glm::vec2& point)
{
    if(index >= _fields.size())
        return;

    const glm::vec4 field { slotRect(index) };
    _pressedPage = _fields[index]._Page.get();
    _pressPoint = point;
    _grabOffset = point.x - field.x;
    _dragLeft = field.x;
    _isDraggingField = false;
    selectField(index, true);
}

void RetroFuturaGUI::Tab::dragField(const glm::vec2& point)
{
    if(!_pressedPage)
        return;

    const uSize index { indexOf(_pressedPage) };

    if(_kNoField == index)
    {
        _pressedPage = nullptr;
        _isDraggingField = false;
        return;
    }

    if(!_isDraggingField)
    {
        if(_dragThreshold >= glm::length(point - _pressPoint))
            return;

        _isDraggingField = true;
    }

    //The field follows the cursor along the strip and takes the slot its middle is nearest to
    const glm::vec4
        first { slotRect(0) },
        last { slotRect(_fields.size() - 1) };
    _dragLeft = std::clamp(point.x - _grabOffset, first.x, std::max(first.x, last.x));
    const uSize slot { slotAt(_dragLeft + first.z * 0.5f) };

    if(slot == index)
        return;

    //The fields in between shift by one toward where it came from
    if(slot < index)
        std::rotate(std::next(_fields.begin(), static_cast<i64>(slot)), std::next(_fields.begin(), static_cast<i64>(index)), std::next(_fields.begin(), static_cast<i64>(index + 1)));
    else
        std::rotate(std::next(_fields.begin(), static_cast<i64>(index)), std::next(_fields.begin(), static_cast<i64>(index + 1)), std::next(_fields.begin(), static_cast<i64>(slot + 1)));
}

uSize RetroFuturaGUI::Tab::fieldAt(const glm::vec2& point) const
{
    if(!contains(getStripRect(), point))
        return _kNoField;

    //The dragged field lies over the others
    const uSize dragged { _isDraggingField ? indexOf(_pressedPage) : _kNoField };

    if(_kNoField != dragged && contains(fieldRect(dragged), point))
        return dragged;

    for(uSize index = 0; index < _fields.size(); ++index)
    {
        const glm::vec4 field { fieldRect(index) };

        if(!fitsInStrip(field))
            continue;

        if(contains(field, point))
            return index;
    }

    return _kNoField;
}

RetroFuturaGUI::ColorState RetroFuturaGUI::Tab::fieldState(const uSize index) const
{
    if(!_isEnabledFlag)
        return ColorState::Disabled;

    if(index < _fields.size() && _pressedPage == _fields[index]._Page.get())
        return ColorState::Clicked;

    if(index == _hoveredField)
        return ColorState::Hover;

    return ColorState::Enabled;
}

glm::vec4 RetroFuturaGUI::Tab::getStripRect() const
{
    const f32
        height { std::min(std::max(_stripHeight, 0.0f), std::max(_size.y, 0.0f)) },
        left { _position.x - _size.x * 0.5f },
        top { _position.y + _size.y * 0.5f };

    return glm::vec4(left, top - height, _size.x, height);
}

glm::vec4 RetroFuturaGUI::Tab::getContentRect() const
{
    const glm::vec4 strip { getStripRect() };
    const f32 bottom { _position.y - _size.y * 0.5f };
    return glm::vec4(strip.x, bottom, strip.z, std::max(strip.y - bottom, 0.0f));
}

f32 RetroFuturaGUI::Tab::getFieldWidth() const
{
    const uSize count { _fields.size() };

    if(0 == count)
        return _maximumFieldWidth;

    const f32 available { getStripRect().z - _fieldSpacing * static_cast<f32>(count + 1) };
    return std::clamp(available / static_cast<f32>(count), _minimumFieldWidth, _maximumFieldWidth);
}

glm::vec4 RetroFuturaGUI::Tab::slotRect(const uSize slot) const
{
    const glm::vec4 strip { getStripRect() };
    const f32 width { getFieldWidth() };

    //The fields stand on the strip's bottom edge, where the content begins, with the spacing above them
    return glm::vec4(strip.x + _fieldSpacing + static_cast<f32>(slot) * (width + _fieldSpacing), strip.y, width, std::max(strip.w - _fieldSpacing, 0.0f));
}

glm::vec4 RetroFuturaGUI::Tab::fieldRect(const uSize index) const
{
    glm::vec4 field { slotRect(index) };

    if(index >= _fields.size())
        return field;

    const glm::vec2 shown { _fields[index]._HasShown ? _fields[index]._Shown.Get() : getFieldTarget(index) };
    field.x = shown.x;
    field.z = shown.y;
    return field;
}

glm::vec2 RetroFuturaGUI::Tab::getFieldTarget(const uSize index) const
{
    const glm::vec4 slot { slotRect(index) };

    if(!_isDraggingField)
        return glm::vec2(slot.x, slot.z);

    if(index >= _fields.size())
        return glm::vec2(slot.x, slot.z);

    if(_pressedPage != _fields[index]._Page.get())
        return glm::vec2(slot.x, slot.z);

    return glm::vec2(_dragLeft, slot.z);
}

void RetroFuturaGUI::Tab::syncFieldRects()
{
    const Transition transition { Transitions::Resolve(_transitionSettings, TransitionProperty::Position) };

    for(uSize index = 0; index < _fields.size(); ++index)
    {
        Field& field { _fields[index] };
        const glm::vec2 target { getFieldTarget(index) };
        const bool isDragged { _isDraggingField && _pressedPage == field._Page.get() };

        if(!field._HasShown || isDragged)
        {
            field._Shown.Jump(target);
            field._HasShown = true;
            continue;
        }

        if(target == field._Shown.GetTarget())
            continue;

        field._Shown.Set(target, transition);
    }
}

uSize RetroFuturaGUI::Tab::slotAt(const f32 x) const
{
    if(_fields.empty())
        return 0;

    const glm::vec4 strip { getStripRect() };
    const f32
        width { getFieldWidth() },
        step { width + _fieldSpacing };

    if(0.0f >= step)
        return 0;

    const f32 slot { std::round((x - strip.x - _fieldSpacing - width * 0.5f) / step) };
    return static_cast<uSize>(std::clamp(slot, 0.0f, static_cast<f32>(_fields.size() - 1)));
}

bool RetroFuturaGUI::Tab::fitsInStrip(const glm::vec4& field) const
{
    const glm::vec4 strip { getStripRect() };
    return field.x + field.z <= strip.x + strip.z + 0.5f; //half a pixel for rounding
}

glm::vec4 RetroFuturaGUI::Tab::closeButtonRect(const glm::vec4& field) const
{
    const f32
        size { std::min(_closeButtonSize, field.w) },
        margin { (field.w - size) * 0.5f }; //as far from the field's right edge as from its top and bottom

    return glm::vec4(field.x + field.z - margin - size, field.y + margin, size, size);
}

glm::vec4 RetroFuturaGUI::Tab::titleRect(const glm::vec4& field) const
{
    const f32 left { field.x + _titlePadding };

    if(!_areCloseButtonsShown)
        return glm::vec4(left, field.y, std::max(field.x + field.z - _titlePadding - left, 0.0f), field.w); //as far from the right edge as from the left.

    const glm::vec4 closeButton { closeButtonRect(field) };
    return glm::vec4(left, field.y, std::max(closeButton.x - _fieldSpacing - left, 0.0f), field.w);
}

void RetroFuturaGUI::Tab::drawFields()
{
    const uSize dragged { _isDraggingField ? indexOf(_pressedPage) : _kNoField };

    for(uSize index = 0; index < _fields.size(); ++index)
    {
        if(index == dragged)
            continue;

        drawField(index, _position.z + _zStep * 2.0f);
    }

    if(_kNoField == dragged)
        return;

    drawField(dragged, _position.z + _zStep * 5.0f); //over the others
}

void RetroFuturaGUI::Tab::drawField(const uSize index, const f32 z)
{
    if(index >= _fields.size())
        return;

    const Field& field { _fields[index] };
    const glm::vec4 rect { fieldRect(index) };

    if(!fitsInStrip(rect))
        return;

    const bool selected { field._Page.get() == _selectedPage };
    const ColorState state { fieldState(index) };
    drawFieldShape(rect, z, _fieldStyle, fieldStyleState(state, selected));
    drawTitle(field._TitleText.get(), rect, z + _zStep, colorFor(_titleColors, state, false));

    if(!_areCloseButtonsShown)
        return;

    ColorState closeState { _isEnabledFlag ? ColorState::Enabled : ColorState::Disabled };

    if(index == _hoveredCloseButton)
        closeState = ColorState::Hover;

    if(index == _pressedCloseButton)
        closeState = ColorState::Clicked;

    drawCloseButton(rect, z + _zStep, closeState);
}

void RetroFuturaGUI::Tab::drawFieldShape(const glm::vec4& rect, const f32 z, Box& style, const ColorState state)
{
    const glm::vec3 center { rect.x + rect.z * 0.5f, rect.y + rect.w * 0.5f, z };
    const glm::vec2 size { rect.z, rect.w };
    Background& background { style.GetBackground() };
    Border& border { style.GetBorder() };

    if(isVisible(background.GetShownColors(state)))
    {
        background.SetColorState(state);
        background.SetSize(size);
        background.SetPosition(center);
        background.Draw();
    }

    if(0.0f >= border.GetWidth() || !isVisible(border.GetShownColors(state)))
        return;

    border.SetColorState(state);
    border.SetSize(size);
    border.SetPosition(center + glm::vec3(0.0f, 0.0f, _zStep * 0.5f));
    border.Draw();
}

RetroFuturaGUI::ColorState RetroFuturaGUI::Tab::fieldStyleState(const ColorState state, const bool selected)
{
    if(ColorState::Disabled == state)
        return ColorState::Disabled;

    return selected ? ColorState::Selected : state;
}

void RetroFuturaGUI::Tab::drawCloseButton(const glm::vec4& field, const f32 z, const ColorState state)
{
    const glm::vec4 button { closeButtonRect(field) };

    if(0.0f >= button.z)
        return;

    const glm::vec3 center { button.x + button.z * 0.5f, button.y + button.w * 0.5f, z };
    Background& background { _closeButton.GetBackground() };

    if(isVisible(background.GetShownColors(state)))
    {
        background.SetColorState(state);
        background.SetSize(glm::vec2(button.z, button.w));
        background.SetPosition(center);
        background.Draw();
    }

    Icon& icon { _closeButton.GetIcon() };
    icon.SetColorState(state); //a state without colors shows the Enabled ones.
    icon.SetSize(glm::vec2(button.z * _closeIconScale)); //the cross keeps clear of the button's edges
    icon.SetPosition(center + glm::vec3(0.0f, 0.0f, _zStep));
    icon.Draw();
}

void RetroFuturaGUI::Tab::drawTitle(Text* title, const glm::vec4& field, const f32 z, const std::vector<glm::vec4>& colors)
{
    if(!title)
        return;

    const glm::vec4 area { titleRect(field) };

    if(0.0f >= area.z)
        return;

    //The text is clipped to the area, so a long title ends at the close button
    title->SetParentSize(glm::vec2(area.z, area.w));
    title->SetPosition(glm::vec3(area.x + area.z * 0.5f, area.y + area.w * 0.5f, z));
    title->SetColors(colors);
    title->Draw();
}

void RetroFuturaGUI::Tab::drawSelectedPage()
{
    if(!_selectedPage)
        return;

    _selectedPage->DrawAtOpacity();
}

void RetroFuturaGUI::Tab::applyTitleFont(Text* title) const
{
    if(!title)
        return;

    title->SetTextAlignment(TextAlignment::Left);

    if(_titleFontFamily.empty())
        return;

    title->SetFontFamily(_titleFontFamily, _titleFontSize, FontSizeUnit::Pixels, _titleFontSlant, _titleFontWeight);
}

uSize RetroFuturaGUI::Tab::indexOf(const IWidget* page) const
{
    if(!page)
        return _kNoField;

    for(uSize index = 0; index < _fields.size(); ++index)
    {
        if(_fields[index]._Page.get() == page)
            return index;
    }

    return _kNoField;
}

uSize RetroFuturaGUI::Tab::indexOf(std::string_view name) const
{
    for(uSize index = 0; index < _fields.size(); ++index)
    {
        const IWidget* page { _fields[index]._Page.get() };

        if(!page)
            continue;

        if(page->GetName() == name)
            return index;
    }

    return _kNoField;
}

void RetroFuturaGUI::Tab::removeField(const uSize index)
{
    if(index >= _fields.size())
        return;

    //Kept alive until the next Interact, since one of its own widgets' slots may be running right now
    std::unique_ptr<IWidget> page { std::move(_fields[index]._Page) };
    IWidget* removed { page.get() };
    _removedPages.push_back(std::move(page));
    _fields.erase(std::next(_fields.begin(), static_cast<i64>(index)));
    _hoveredField = _kNoField;
    _hoveredCloseButton = _kNoField;
    _pressedCloseButton = _kNoField;

    if(_pressedPage == removed)
    {
        _pressedPage = nullptr;
        _isDraggingField = false;
    }

    _onPageRemoved.Emit(removed);
    _onPageRemovedAsync.EmitAsync(removed);

    if(_selectedPage != removed)
        return;

    _selectedPage = nullptr;

    if(_fields.empty())
    {
        _onSelectionChanged.Emit(nullptr);
        _onSelectionChangedAsync.EmitAsync(nullptr);
        return;
    }

    //The field now in its place takes over, or the one before it when it was the last
    selectField(std::min(index, _fields.size() - 1), true);
}

const std::vector<glm::vec4>& RetroFuturaGUI::Tab::colorFor(const FieldColors& colors, const ColorState state, const bool selected)
{
    if(ColorState::Disabled == state)
        return colors._Disabled;

    if(selected)
        return colors._Selected;

    switch(state)
    {
        case ColorState::Hover:
            return colors._Hover;
        case ColorState::Clicked:
            return colors._Clicked;
        default:
            return colors._Enabled;
    }
}

std::vector<glm::vec4>& RetroFuturaGUI::Tab::colorOf(FieldColors& colors, const ColorState state)
{
    switch(state)
    {
        case ColorState::Hover:
            return colors._Hover;
        case ColorState::Clicked:
            return colors._Clicked;
        case ColorState::Disabled:
            return colors._Disabled;
        default:
            return colors._Enabled;
    }
}

bool RetroFuturaGUI::Tab::isVisible(std::span<const glm::vec4> colors)
{
    return std::any_of(colors.begin(), colors.end(), [](const glm::vec4& color) { return 0.0f < color.a; });
}

bool RetroFuturaGUI::Tab::contains(const glm::vec4& rect, const glm::vec2& point)
{
    return point.x >= rect.x && point.x <= rect.x + rect.z && point.y >= rect.y && point.y <= rect.y + rect.w;
}

std::string RetroFuturaGUI::Tab::getCloseIconPath()
{
    std::filesystem::path path { PlatformBridge::Paths::GetExecutablePathFSPath().parent_path() };
    path /= "Resources";
    path /= "img";
    path /= "CloseIcon.svg";

    //Not path.string(): on Windows that goes through the ANSI code page and turns names outside it into '?'
    const std::u8string text { path.u8string() };
    return std::string(text.begin(), text.end());
}

RetroFuturaGUI::Box& RetroFuturaGUI::Tab::GetFieldStyle()
{
    return _fieldStyle;
}

const RetroFuturaGUI::Box& RetroFuturaGUI::Tab::GetFieldStyle() const
{
    return _fieldStyle;
}

void RetroFuturaGUI::Tab::setFieldStyleDefaults()
{
    Background& background { _fieldStyle.GetBackground() };
    background.SetBuiltInColors(styleKey(), std::array { glm::vec4(0.0f) }, ColorState::Enabled);
    background.SetBuiltInColors(styleKey(), std::array { glm::vec4(1.0f, 1.0f, 1.0f, 0.06f) }, ColorState::Hover);
    background.SetBuiltInColors(styleKey(), std::array { glm::vec4(1.0f, 1.0f, 1.0f, 0.1f) }, ColorState::Clicked);
    background.SetBuiltInColors(styleKey(), std::array { glm::vec4(0.0f) }, ColorState::Disabled);
    background.SetBuiltInColors(styleKey(), std::array { glm::vec4(0.086f, 0.086f, 0.1137f, 1.0f) }, ColorState::Selected); //a window's default background, so the selected field joins its content.
    background.SetBuiltInCornerRadii(styleKey(), glm::vec4(8.0f, 8.0f, 0.0f, 0.0f));

    Border& border { _fieldStyle.GetBorder() };
    border.SetBuiltInColors(styleKey(), std::array { glm::vec4(0.35f, 0.35f, 0.45f, 1.0f) }, ColorState::Enabled);
    border.SetBuiltInColors(styleKey(), std::array { glm::vec4(0.45f, 0.45f, 0.55f, 1.0f) }, ColorState::Hover);
    border.SetBuiltInColors(styleKey(), std::array { glm::vec4(0.5f, 0.5f, 0.6f, 1.0f) }, ColorState::Clicked);
    border.SetBuiltInColors(styleKey(), std::array { glm::vec4(0.25f, 0.25f, 0.3f, 1.0f) }, ColorState::Disabled);
    border.SetBuiltInColors(styleKey(), std::array { glm::vec4(0.35f, 0.35f, 0.45f, 1.0f) }, ColorState::Selected);
    border.SetBuiltInCornerRadii(styleKey(), glm::vec4(8.0f, 8.0f, 0.0f, 0.0f));
    border.SetBuiltInWidth(styleKey(), 0.0f); //no border until one is asked for.
}

RetroFuturaGUI::CloseButton& RetroFuturaGUI::Tab::GetCloseButton()
{
    return _closeButton;
}

const RetroFuturaGUI::CloseButton& RetroFuturaGUI::Tab::GetCloseButton() const
{
    return _closeButton;
}

RetroFuturaGUI::Background& RetroFuturaGUI::Tab::GetBackground()
{
    return _background;
}

const RetroFuturaGUI::Background& RetroFuturaGUI::Tab::GetBackground() const
{
    return _background;
}

RetroFuturaGUI::Border& RetroFuturaGUI::Tab::GetBorder()
{
    return _border;
}

const RetroFuturaGUI::Border& RetroFuturaGUI::Tab::GetBorder() const
{
    return _border;
}
