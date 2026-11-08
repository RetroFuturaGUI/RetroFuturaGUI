#include "WindowTab.hpp"
#include "Window.hpp"
#include "PlatformBridge.hpp"
#include "ResourceManager.hpp"
#include <algorithm>
#include <cmath>
#include <filesystem>
#include <span>

RetroFuturaGUI::WindowTab::WindowTab(std::string_view name, Projection* projection, IHierarchyNode* parentWidget, const WidgetTypeID parentWidgetTypeID, GLFWwindow* parentWindow)
    : IWidget(name, projection, parentWidget, parentWidgetTypeID, parentWindow)
{
    _widgetTypeID = WidgetTypeID::WindowTab;
    _closeBackground = std::make_unique<Plane>(projection);
    _previewTitle = std::make_unique<Text>(projection);
    //The field elements draw once per field, in its colors, so they switch at once
    _background.SetTransitionSettings(&_transitionSettings);
    _border.SetTransitionSettings(&_transitionSettings);

    if(!_closeBackground)
        return;

    _border.SetWidth(0.0f); //no frame until one is asked for, since a Frame starts with a 5 px border
    _background.SetColors(std::array { glm::vec4(0.055f, 0.055f, 0.075f, 1.0f) }, ColorState::Enabled);
    _background.SetColors(std::array { glm::vec4(0.055f, 0.055f, 0.075f, 1.0f) }, ColorState::Disabled);
    //_border.SetColors(std::span<const glm::vec4> colors, const ColorState state)
    applyTitleFont(_previewTitle.get());
    _closeIcon.SetColorState(ColorState::Enabled); //the cross takes its color at once, not the SVG's own.
    setFieldStyleDefaults();

    Window* host { GetHostWindow() };

    if(!host)
        return;

    host->_windowTabs.push_back(this);
}

RetroFuturaGUI::WindowTab::~WindowTab()
{
    //Every window becomes a free one again, where the content area was. No signals: whoever connected them may be gone already.
    const glm::ivec4 content { getContentScreenRect() };
    std::vector<Field> fields { std::move(_fields) };
    _fields.clear();
    _selectedWindow = nullptr;
    _pressedWindow = nullptr;

    for(Field& field : fields)
    {
        if(!field._Window)
            continue;

        field._Window->leaveTab(glm::ivec2(content.x, content.y));
    }

    Window* host { GetHostWindow() };

    if(!host)
        return;

    std::erase(host->_windowTabs, this);
}

void RetroFuturaGUI::WindowTab::Draw()
{
    syncTitles(); //again, since a widget after this WindowTab may have renamed a window
    syncFieldRects(); //again, since the pointer may have moved, closed or added a field since
    _background.Draw();
    _border.Draw();
    drawFields();
    drawDropPreview();
    drawSelectedWindow();
}

void RetroFuturaGUI::WindowTab::applyPosition(const glm::vec3& position)
{
    IWidget::applyPosition(position);
    _background.SetPosition(position);
    _border.SetPosition(position + glm::vec3(0.0f, 0.0f, _zStep * 8.0f)); //over the fields
}

void RetroFuturaGUI::WindowTab::applySize(const glm::vec3& size)
{
    IWidget::applySize(size);

    _background.SetSize(glm::vec2(size));
    _border.SetSize(glm::vec2(size));
}

bool RetroFuturaGUI::WindowTab::AddWindow(Window* window)
{
    return InsertWindow(window, _fields.size());
}

bool RetroFuturaGUI::WindowTab::InsertWindow(Window* window, const uSize index)
{
    if(!window)
        return false;

    if(_kNoField != indexOf(window))
        return false;

    if(!window->enterTab(this))
        return false;

    std::unique_ptr<Text> title { std::make_unique<Text>(&_projection) };
    applyTitleFont(title.get());

    //A window dropped on the strip takes its field from where the preview showed, and moves into its slot from there
    TransitionValue<glm::vec2> shown {};
    shown.Jump(_previewShown.Get());

    _fields.insert(std::next(_fields.begin(), static_cast<i64>(std::min(index, _fields.size()))),
                   Field { ._Window = window, ._Title = std::move(title), ._ShownTitle = {}, ._Shown = shown, ._HasShown = _isPreviewShown });
    syncTitles();

    _onWindowAdded.Emit(window);
    _onWindowAddedAsync.EmitAsync(window);
    SelectWindow(window, true);
    return true;
}

bool RetroFuturaGUI::WindowTab::RemoveWindow(Window* window)
{
    if(!window)
        return false;

    const uSize index { indexOf(window) };

    if(_kNoField == index)
        return false;

    //It stays where its content was
    const glm::ivec4 content { getContentScreenRect() };
    removeField(index);
    window->leaveTab(glm::ivec2(content.x, content.y));
    _onWindowRemoved.Emit(window);
    _onWindowRemovedAsync.EmitAsync(window);
    return true;
}

void RetroFuturaGUI::WindowTab::SelectWindow(Window* window, const bool emitSignal)
{
    if(!window)
        return;

    if(_kNoField == indexOf(window))
        return;

    if(_selectedWindow == window)
        return;

    _selectedWindow = window;
    window->_ignoreHeldPress = true; //the press that selected it may still be down, and isn't a click on what lies under it
    Window::markLayoutChanged(); //the layout file keeps which tab shows

    if(!emitSignal)
        return;

    _onSelectionChanged.Emit(window);
    _onSelectionChangedAsync.EmitAsync(window);
}

RetroFuturaGUI::Window* RetroFuturaGUI::WindowTab::GetSelectedWindow() const
{
    return _selectedWindow;
}

RetroFuturaGUI::Window* RetroFuturaGUI::WindowTab::GetWindow(const uSize index) const
{
    if(index >= _fields.size())
        return nullptr;

    return _fields[index]._Window;
}

uSize RetroFuturaGUI::WindowTab::GetWindowCount() const
{
    return _fields.size();
}

RetroFuturaGUI::Window* RetroFuturaGUI::WindowTab::GetHostWindow() const
{
    return Window::FromGlfwWindow(_parentWindow);
}

void RetroFuturaGUI::WindowTab::Connect_OnSelectionChanged(const typename Signal<Window*>::Slot& slot, const bool async)
{
    if(async)
        _onSelectionChangedAsync.Connect(slot);
    else
        _onSelectionChanged.Connect(slot);
}

void RetroFuturaGUI::WindowTab::Disconnect_OnSelectionChanged(const typename Signal<Window*>::Slot& slot)
{
    _onSelectionChanged.Disconnect(slot);
    _onSelectionChangedAsync.Disconnect(slot);
}

void RetroFuturaGUI::WindowTab::Connect_OnWindowAdded(const typename Signal<Window*>::Slot& slot, const bool async)
{
    if(async)
        _onWindowAddedAsync.Connect(slot);
    else
        _onWindowAdded.Connect(slot);
}

void RetroFuturaGUI::WindowTab::Disconnect_OnWindowAdded(const typename Signal<Window*>::Slot& slot)
{
    _onWindowAdded.Disconnect(slot);
    _onWindowAddedAsync.Disconnect(slot);
}

void RetroFuturaGUI::WindowTab::Connect_OnWindowRemoved(const typename Signal<Window*>::Slot& slot, const bool async)
{
    if(async)
        _onWindowRemovedAsync.Connect(slot);
    else
        _onWindowRemoved.Connect(slot);
}

void RetroFuturaGUI::WindowTab::Disconnect_OnWindowRemoved(const typename Signal<Window*>::Slot& slot)
{
    _onWindowRemoved.Disconnect(slot);
    _onWindowRemovedAsync.Disconnect(slot);
}

void RetroFuturaGUI::WindowTab::SetStripHeight(const f32 height)
{
    _stripHeight = std::max(height, 0.0f);
}

void RetroFuturaGUI::WindowTab::SetFieldWidthRange(const f32 minimumWidth, const f32 maximumWidth)
{
    _minimumFieldWidth = std::max(minimumWidth, 1.0f);
    _maximumFieldWidth = std::max(maximumWidth, _minimumFieldWidth);
}

void RetroFuturaGUI::WindowTab::SetFieldSpacing(const f32 spacing)
{
    _fieldSpacing = std::max(spacing, 0.0f);
}

void RetroFuturaGUI::WindowTab::SetTitleFontFamily(std::string_view fontFamily, const f32 fontSize, const FontSizeUnit sizeUnit, const PlatformBridge::Fonts::Slant slant, const PlatformBridge::Fonts::Weight weight)
{
    _titleFontFamily = fontFamily;
    _titleFontSize = FontManager::ToPixels(fontSize, sizeUnit);
    _titleFontSlant = slant;
    _titleFontWeight = weight;

    for(Field& field : _fields)
        applyTitleFont(field._Title.get());

    applyTitleFont(_previewTitle.get());
}

f32 RetroFuturaGUI::WindowTab::GetTitleFontSize(const FontSizeUnit sizeUnit) const
{
    return FontManager::FromPixels(_titleFontSize, sizeUnit);
}

void RetroFuturaGUI::WindowTab::SetTitleColors(std::span<const glm::vec4> colors, const ColorState state)
{
    colorOf(_titleColors, state).assign(colors.begin(), colors.end());
}

void RetroFuturaGUI::WindowTab::SetSelectedTitleColors(std::span<const glm::vec4> colors)
{
    _titleColors._Selected.assign(colors.begin(), colors.end());
}

void RetroFuturaGUI::WindowTab::SetTitlePadding(const f32 padding)
{
    _titlePadding = std::max(padding, 0.0f);
}

void RetroFuturaGUI::WindowTab::SetCloseButtonSize(const f32 size)
{
    _closeButtonSize = std::max(size, 0.0f);

    //Rasterized at twice the size the cross draws at, so it stays smooth when it is scaled down
    const i32 resolution { std::max(static_cast<i32>(std::lround(_closeButtonSize * _closeIconScale * 2.0f)), 8) };
    _closeIcon.Rasterize(glm::i32vec2(resolution));
}

void RetroFuturaGUI::WindowTab::SetCloseButtonBackgroundColors(std::span<const glm::vec4> colors, const ColorState state)
{
    colorOf(_closeBackgroundColors, state).assign(colors.begin(), colors.end());
}

void RetroFuturaGUI::WindowTab::SetContentCornerRadii(const glm::vec4& radii)
{
    _contentCornerRadii = radii;
}

const glm::vec4& RetroFuturaGUI::WindowTab::GetContentCornerRadii() const
{
    return _contentCornerRadii;
}

void RetroFuturaGUI::WindowTab::interact()
{
    _isShown = true;
    syncTitles();
    syncFieldRects(); //the pointer meets the fields where they are drawn
    followPointer();
    interactSelectedWindow();
}

void RetroFuturaGUI::WindowTab::followPointer()
{
    const PointerState pointer { readPointer() };
    const bool
        isPressed { pointer._LeftButtonPressed },
        pressedThisFrame { isPressed && !_wasPressed };
    _wasPressed = isPressed;

    if(!isPressed)
    {
        _pressedWindow = nullptr;
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
    _hoveredCloseButton = _kNoField != _hoveredField && contains(closeButtonRect(fieldRect(_hoveredField)), point) ? _hoveredField : _kNoField;

    if(!_isEnabledFlag)
        return;

    if(pressedThisFrame)
    {
        //Like the window bar's close button, the cross closes on-press
        if(_kNoField != _hoveredCloseButton)
        {
            _pressedCloseButton = _hoveredCloseButton;
            closeWindow(_hoveredCloseButton);
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

void RetroFuturaGUI::WindowTab::pressField(const uSize index, const glm::vec2& point)
{
    if(index >= _fields.size())
        return;

    Window* window { _fields[index]._Window };

    if(!window)
        return;

    const glm::vec4 field { slotRect(index) };
    _pressedWindow = window;
    _pressPoint = point;
    _grabOffset = glm::vec2(point.x - field.x, field.y + field.w - point.y); //from the top-left corner, y running downward
    _dragLeft = field.x;
    _sidewaysTravel = 0.0f;
    _isDraggingField = false;
    SelectWindow(window, true);
}

void RetroFuturaGUI::WindowTab::dragField(const glm::vec2& point)
{
    if(!_pressedWindow)
        return;

    const uSize index { indexOf(_pressedWindow) };

    if(_kNoField == index)
    {
        _pressedWindow = nullptr;
        _isDraggingField = false;
        return;
    }

    const glm::vec2 travel { point - _pressPoint };
    _sidewaysTravel = std::max(_sidewaysTravel, std::abs(travel.x));

    if(!_isDraggingField)
    {
        if(_dragThreshold >= glm::length(travel))
            return;

        _isDraggingField = true;
    }

    if(isPullingOut(index, point))
    {
        pullOut(index, point);
        return;
    }

    //The field follows the cursor along the strip and takes the slot its middle is nearest to
    const glm::vec4
        first { slotRect(0) },
        last { slotRect(_fields.size() - 1) };
    _dragLeft = std::clamp(point.x - _grabOffset.x, first.x, std::max(first.x, last.x));
    const uSize slot { slotAt(_dragLeft + first.z * 0.5f) };

    if(slot == index)
        return;

    //The fields in between shift by one toward where it came from
    if(slot < index)
        std::rotate(std::next(_fields.begin(), static_cast<i64>(slot)), std::next(_fields.begin(), static_cast<i64>(index)), std::next(_fields.begin(), static_cast<i64>(index + 1)));
    else
        std::rotate(std::next(_fields.begin(), static_cast<i64>(index)), std::next(_fields.begin(), static_cast<i64>(index + 1)), std::next(_fields.begin(), static_cast<i64>(slot + 1)));

    Window::markLayoutChanged(); //the layout file keeps the tabs' order
}

bool RetroFuturaGUI::WindowTab::isPullingOut(const uSize index, const glm::vec2& point) const
{
    const glm::vec4 strip { getStripRect() };
    const f32 fontSize { _titleFontSize };

    //Above or below the strip. A long sideways drag raises the threshold, so reordering doesn't pull a field out by accident.
    const f32
        verticalThreshold { _pullOutDistance * fontSize + std::clamp((_sidewaysTravel - 2.0f * fontSize) * 0.2f, 0.0f, 4.0f * fontSize) }, //ImGui's numbers
        outsideStrip { std::max(strip.y - point.y, point.y - (strip.y + strip.w)) };

    if(outsideStrip >= verticalThreshold)
        return true;

    //Past the first or the last field
    const f32 pastEndThreshold { _pullOutPastEndDistance * fontSize };

    if(0 == index && slotRect(0).x - point.x > pastEndThreshold)
        return true;

    const glm::vec4 last { slotRect(_fields.size() - 1) };
    return _fields.size() - 1 == index && point.x - (last.x + last.z) > pastEndThreshold;
}

void RetroFuturaGUI::WindowTab::pullOut(const uSize index, const glm::vec2& point)
{
    if(index >= _fields.size())
        return;

    Window* window { _fields[index]._Window };

    if(!window)
        return;

    //The cursor holds the window where it held the field, so the window's top-left lands where the field's was. Never past its
    //right edge, though, when the field was wider than the window.
    const i32 widest { std::max(window->_untabbedSize.x - 1, 0) };
    const glm::ivec2
        grabPoint { std::min(static_cast<i32>(std::lround(_grabOffset.x)), widest), static_cast<i32>(std::lround(_grabOffset.y)) },
        position { toScreen(point) - grabPoint };

    _pressedWindow = nullptr;
    _isDraggingField = false;
    removeField(index);
    window->leaveTab(position);
    window->beginCursorDrag(grabPoint);
    _onWindowRemoved.Emit(window);
    _onWindowRemovedAsync.EmitAsync(window);
}

void RetroFuturaGUI::WindowTab::closeWindow(const uSize index) const
{
    if(index >= _fields.size())
        return;

    const Window* window { _fields[index]._Window };

    if(!window)
        return;

    GLFWwindow* glfwWindow { window->GetGlfwWindow() };

    if(!glfwWindow)
        return;

    glfwSetWindowShouldClose(glfwWindow, GLFW_TRUE);
}

uSize RetroFuturaGUI::WindowTab::fieldAt(const glm::vec2& point) const
{
    if(!contains(getStripRect(), point))
        return _kNoField;

    //The dragged field lies over the others
    const uSize dragged { _isDraggingField ? indexOf(_pressedWindow) : _kNoField };

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

RetroFuturaGUI::ColorState RetroFuturaGUI::WindowTab::fieldState(const uSize index) const
{
    if(!_isEnabledFlag)
        return ColorState::Disabled;

    if(index < _fields.size() && _pressedWindow == _fields[index]._Window)
        return ColorState::Clicked;

    if(index == _hoveredField)
        return ColorState::Hover;

    return ColorState::Enabled;
}

glm::vec4 RetroFuturaGUI::WindowTab::getStripRect() const
{
    const f32
        height { std::min(std::max(_stripHeight, 0.0f), std::max(_size.y, 0.0f)) },
        left { _position.x - _size.x * 0.5f },
        top { _position.y + _size.y * 0.5f };

    return glm::vec4(left, top - height, _size.x, height);
}

RetroFuturaGUI::ScreenRect RetroFuturaGUI::WindowTab::getContentArea() const
{
    const glm::vec4 strip { getStripRect() };
    const f32 bottom { _position.y - _size.y * 0.5f };
    return ScreenRect { ._Origin = glm::vec2(strip.x, bottom), ._Extent = glm::vec2(strip.z, std::max(strip.y - bottom, 0.0f)) };
}

f32 RetroFuturaGUI::WindowTab::getFieldWidth() const
{
    const uSize count { _showsDropPreview ? _fields.size() + 1 : _fields.size() }; //while a window is dragged over the strip, its preview field counts too, so the fields narrow to make room for it

    if(0 == count)
        return _maximumFieldWidth;

    const f32 available { getStripRect().z - _fieldSpacing * static_cast<f32>(count + 1) };
    return std::clamp(available / static_cast<f32>(count), _minimumFieldWidth, _maximumFieldWidth);
}

glm::vec4 RetroFuturaGUI::WindowTab::slotRect(const uSize slot) const
{
    const glm::vec4 strip { getStripRect() };
    const f32 width { getFieldWidth() };

    //The fields stand on the strip's bottom edge, where the content begins, with the spacing above them
    return glm::vec4(strip.x + _fieldSpacing + static_cast<f32>(slot) * (width + _fieldSpacing), strip.y, width, std::max(strip.w - _fieldSpacing, 0.0f));
}

glm::vec4 RetroFuturaGUI::WindowTab::fieldRect(const uSize index) const
{
    glm::vec4 field { slotRect(index) };

    if(index >= _fields.size())
        return field;

    const glm::vec2 shown { _fields[index]._HasShown ? _fields[index]._Shown.Get() : getFieldTarget(index) };
    field.x = shown.x;
    field.z = shown.y;
    return field;
}

glm::vec2 RetroFuturaGUI::WindowTab::getFieldTarget(const uSize index) const
{
    const glm::vec4 slot { slotRect(index) };

    if(!_isDraggingField)
        return glm::vec2(slot.x, slot.z);

    if(index >= _fields.size())
        return glm::vec2(slot.x, slot.z);

    if(_pressedWindow != _fields[index]._Window)
        return glm::vec2(slot.x, slot.z);

    return glm::vec2(_dragLeft, slot.z);
}

void RetroFuturaGUI::WindowTab::syncFieldRects()
{
    const Transition transition { Transitions::Resolve(_transitionSettings, TransitionProperty::Position) };

    for(uSize index = 0; index < _fields.size(); ++index)
    {
        Field& field { _fields[index] };
        const glm::vec2 target { getFieldTarget(index) };
        const bool isDragged { _isDraggingField && _pressedWindow == field._Window };

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

    if(!_showsDropPreview)
    {
        _isPreviewShown = false;
        return;
    }

    const glm::vec4 slot { slotRect(_fields.size()) };
    const glm::vec2 target { slot.x, slot.z };

    //The preview field opens from nothing where it will stand, while the others narrow to make room
    if(!_isPreviewShown)
    {
        _previewShown.Jump(glm::vec2(slot.x, 0.0f));
        _isPreviewShown = true;
    }

    if(target == _previewShown.GetTarget())
        return;

    _previewShown.Set(target, transition);
}

uSize RetroFuturaGUI::WindowTab::slotAt(const f32 x) const
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

bool RetroFuturaGUI::WindowTab::fitsInStrip(const glm::vec4& field) const
{
    const glm::vec4 strip { getStripRect() };
    return field.x + field.z <= strip.x + strip.z + 0.5f; //half a pixel for rounding
}

glm::vec4 RetroFuturaGUI::WindowTab::closeButtonRect(const glm::vec4& field) const
{
    const f32
        size { std::min(_closeButtonSize, field.w) },
        margin { (field.w - size) * 0.5f }; //as far from the field's right edge as from its top and bottom

    return glm::vec4(field.x + field.z - margin - size, field.y + margin, size, size);
}

glm::vec4 RetroFuturaGUI::WindowTab::titleRect(const glm::vec4& field) const
{
    const glm::vec4 closeButton { closeButtonRect(field) };
    const f32 left { field.x + _titlePadding };
    return glm::vec4(left, field.y, std::max(closeButton.x - _fieldSpacing - left, 0.0f), field.w);
}

glm::ivec2 RetroFuturaGUI::WindowTab::toScreen(const glm::vec2& point) const
{
    //consider OPenGL-style window coordinates (bottom-left is 0,0)
    const glm::ivec2 local { static_cast<i32>(std::lround(point.x)), static_cast<i32>(std::lround(_projection.GetResolution().y - point.y)) };

    if(!_parentWindow)
        return local;

    glm::ivec2 windowPosition { 0 };
    glfwGetWindowPos(_parentWindow, &windowPosition.x, &windowPosition.y);
    return windowPosition + local;
}

void RetroFuturaGUI::WindowTab::drawFields()
{
    const uSize dragged { _isDraggingField ? indexOf(_pressedWindow) : _kNoField };

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

void RetroFuturaGUI::WindowTab::drawField(const uSize index, const f32 z)
{
    if(index >= _fields.size())
        return;

    const Field& field { _fields[index] };

    if(!field._Window)
        return;

    const glm::vec4 rect { fieldRect(index) };

    if(!fitsInStrip(rect))
        return;

    const bool selected { field._Window == _selectedWindow };
    const ColorState state { fieldState(index) };
    drawFieldShape(rect, z, _fieldStyle, fieldStyleState(state, selected));
    drawTitle(field._Title.get(), rect, z + _zStep, colorFor(_titleColors, state, selected));

    ColorState closeState { _isEnabledFlag ? ColorState::Enabled : ColorState::Disabled };

    if(index == _hoveredCloseButton)
        closeState = ColorState::Hover;

    if(index == _pressedCloseButton)
        closeState = ColorState::Clicked;

    drawCloseButton(rect, z + _zStep, closeState);
}

void RetroFuturaGUI::WindowTab::drawFieldShape(const glm::vec4& rect, const f32 z, Box& style, const ColorState state)
{
    const glm::vec3 center { rect.x + rect.z * 0.5f, rect.y + rect.w * 0.5f, z };
    const glm::vec2 size { rect.z, rect.w };
    Background& background { style.GetBackground() };
    Border& border { style.GetBorder() };

    if(isVisible(background.GetColors(state)))
    {
        background.SetColorState(state);
        background.SetSize(size);
        background.SetPosition(center);
        background.Draw();
    }

    if(0.0f >= border.GetWidth() || !isVisible(border.GetColors(state)))
        return;

    border.SetColorState(state);
    border.SetSize(size);
    border.SetPosition(center + glm::vec3(0.0f, 0.0f, _zStep * 0.5f));
    border.Draw();
}

RetroFuturaGUI::ColorState RetroFuturaGUI::WindowTab::fieldStyleState(const ColorState state, const bool selected)
{
    if(ColorState::Disabled == state)
        return ColorState::Disabled;

    return selected ? ColorState::Selected : state;
}

void RetroFuturaGUI::WindowTab::drawCloseButton(const glm::vec4& field, const f32 z, const ColorState state)
{
    if(!_closeBackground)
        return;

    const glm::vec4 button { closeButtonRect(field) };

    if(0.0f >= button.z)
        return;

    const glm::vec3 center { button.x + button.z * 0.5f, button.y + button.w * 0.5f, z };
    const std::vector<glm::vec4>& background { colorFor(_closeBackgroundColors, state, false) };

    if(isVisible(background))
    {
        _closeBackground->SetColors(background);
        _closeBackground->SetCornerRadii(glm::vec4(button.z * 0.25f));
        _closeBackground->SetSize(glm::vec2(button.z, button.w));
        _closeBackground->SetPosition(center);
        _closeBackground->Draw();
    }

    _closeIcon.SetSize(glm::vec2(button.z * _closeIconScale)); //the cross keeps clear of the button's edges
    _closeIcon.SetPosition(center + glm::vec3(0.0f, 0.0f, _zStep));
    _closeIcon.Draw();
}

void RetroFuturaGUI::WindowTab::drawTitle(Text* title, const glm::vec4& field, const f32 z, const std::vector<glm::vec4>& colors)
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

void RetroFuturaGUI::WindowTab::interactSelectedWindow()
{
    if(!_selectedWindow)
        return;

    const ScreenRect area { getContentArea() };

    if(1.0f > area._Extent.x || 1.0f > area._Extent.y)
        return;

    const glm::ivec4 screen { getContentScreenRect() };
    _selectedWindow->interactInHost(area, glm::ivec2(screen.x, screen.y), _contentCornerRadii); //nothing of this WindowTab is used after it: the window's widgets may take it out
}

void RetroFuturaGUI::WindowTab::drawSelectedWindow()
{
    if(!_selectedWindow)
        return;

    const ScreenRect area { getContentArea() };

    if(1.0f > area._Extent.x || 1.0f > area._Extent.y)
        return;

    const glm::ivec4 screen { getContentScreenRect() };
    _selectedWindow->drawInHost(area, glm::ivec2(screen.x, screen.y), _contentCornerRadii); //nothing of this WindowTab is used after it: the window's widgets may take it out
}

void RetroFuturaGUI::WindowTab::drawDropPreview()
{
    if(!_showsDropPreview)
        return;

    glm::vec4 rect { slotRect(_fields.size()) };

    if(!fitsInStrip(rect))
        return;

    const glm::vec2 shown { _previewShown.Get() };
    rect.x = shown.x;
    rect.z = shown.y;

    //See-through, since the window isn't in yet: the selected field's look at half its opacity. Copies, since the title only points at the colors it draws
    std::vector<glm::vec4>
        background { _fieldStyle.GetBackground().GetColors(ColorState::Selected) },
        titleColors { _titleColors._Selected };

    for(glm::vec4& color : background)
        color.a *= 0.5f;

    for(glm::vec4& color : titleColors)
        color.a *= 0.6f;

    const f32 z { _position.z + _zStep * 2.0f };
    _previewFieldStyle.GetBackground().SetColors(background, ColorState::Enabled);
    _previewFieldStyle.GetBackground().SetCornerRadii(_fieldStyle.GetBackground().GetCornerRadii());
    _previewFieldStyle.GetBorder().SetColors(_fieldStyle.GetBorder().GetColors(ColorState::Selected), ColorState::Enabled);
    _previewFieldStyle.GetBorder().SetCornerRadii(_fieldStyle.GetBorder().GetCornerRadii());
    _previewFieldStyle.GetBorder().SetWidth(_fieldStyle.GetBorder().GetWidth());
    drawFieldShape(rect, z, _previewFieldStyle, ColorState::Enabled);
    drawTitle(_previewTitle.get(), rect, z + _zStep, titleColors);
}

void RetroFuturaGUI::WindowTab::syncTitles()
{
    for(Field& field : _fields)
    {
        if(!field._Window)
            continue;

        if(!field._Title)
            continue;

        const std::string& title { field._Window->GetWindowTitle() };

        if(title == field._ShownTitle)
            continue;

        field._ShownTitle = title;
        field._Title->SetTextUTF8(title);
    }
}

void RetroFuturaGUI::WindowTab::applyTitleFont(Text* title) const
{
    if(!title)
        return;

    title->SetTextAlignment(TextAlignment::Left);

    if(_titleFontFamily.empty())
        return;

    title->SetFontFamily(_titleFontFamily, _titleFontSize, FontSizeUnit::Pixels, _titleFontSlant, _titleFontWeight);
}

bool RetroFuturaGUI::WindowTab::isStripUnder(const glm::ivec2& screenPoint) const
{
    if(!_isShown)
        return false;

    if(!_isEnabledFlag)
        return false;

    if(!_parentWindow)
        return false;

    glm::ivec2 windowPosition { 0 };
    glfwGetWindowPos(_parentWindow, &windowPosition.x, &windowPosition.y);
    const glm::ivec2 local { screenPoint - windowPosition };
    const glm::vec2 point { static_cast<f32>(local.x), _projection.GetResolution().y - static_cast<f32>(local.y) };
    return contains(getStripRect(), point);
}

glm::ivec4 RetroFuturaGUI::WindowTab::getContentScreenRect() const
{
    const ScreenRect area { getContentArea() };
    const glm::ivec2 topLeft { toScreen(glm::vec2(area._Origin.x, area._Origin.y + area._Extent.y)) };
    return glm::ivec4
    (
        topLeft.x,
        topLeft.y,
        std::max(static_cast<i32>(std::lround(area._Extent.x)), 1),
        std::max(static_cast<i32>(std::lround(area._Extent.y)), 1)
    );
}

void RetroFuturaGUI::WindowTab::showDropPreview(const std::string& title)
{
    _showsDropPreview = true;

    if(!_previewTitle)
        return;

    if(_previewTitle->GetTextUTF8() == title)
        return;

    _previewTitle->SetTextUTF8(title);
}

void RetroFuturaGUI::WindowTab::hideDropPreview()
{
    _showsDropPreview = false;
}

uSize RetroFuturaGUI::WindowTab::indexOf(const Window* window) const
{
    if(!window)
        return _kNoField;

    for(uSize index = 0; index < _fields.size(); ++index)
    {
        if(_fields[index]._Window == window)
            return index;
    }

    return _kNoField;
}

void RetroFuturaGUI::WindowTab::removeField(const uSize index)
{
    if(index >= _fields.size())
        return;

    Window* window { _fields[index]._Window };
    _fields.erase(std::next(_fields.begin(), static_cast<i64>(index)));
    _hoveredField = _kNoField;
    _hoveredCloseButton = _kNoField;
    _pressedCloseButton = _kNoField;

    if(_pressedWindow == window)
    {
        _pressedWindow = nullptr;
        _isDraggingField = false;
    }

    if(_selectedWindow != window)
        return;

    _selectedWindow = nullptr;

    if(_fields.empty())
    {
        _onSelectionChanged.Emit(nullptr);
        _onSelectionChangedAsync.EmitAsync(nullptr);
        return;
    }

    //The field now in its place takes over, or the one before it when it was the last
    SelectWindow(_fields[std::min(index, _fields.size() - 1)]._Window, true);
}

const std::vector<glm::vec4>& RetroFuturaGUI::WindowTab::colorFor(const FieldColors& colors, const ColorState state, const bool selected)
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

std::vector<glm::vec4>& RetroFuturaGUI::WindowTab::colorOf(FieldColors& colors, const ColorState state)
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

bool RetroFuturaGUI::WindowTab::isVisible(std::span<const glm::vec4> colors)
{
    return std::any_of(colors.begin(), colors.end(), [](const glm::vec4& color) { return 0.0f < color.a; });
}

bool RetroFuturaGUI::WindowTab::contains(const glm::vec4& rect, const glm::vec2& point)
{
    return point.x >= rect.x && point.x <= rect.x + rect.z && point.y >= rect.y && point.y <= rect.y + rect.w;
}

std::string RetroFuturaGUI::WindowTab::getCloseIconPath()
{
    std::filesystem::path path { PlatformBridge::Paths::GetExecutablePathFSPath().parent_path() };
    path /= "Resources";
    path /= "img";
    path /= "CloseIcon.svg";

    //Not path.string(): on Windows that goes through the ANSI code page and turns names outside it into '?'
    const std::u8string text { path.u8string() };
    return std::string(text.begin(), text.end());
}

RetroFuturaGUI::Box& RetroFuturaGUI::WindowTab::GetFieldStyle()
{
    return _fieldStyle;
}

const RetroFuturaGUI::Box& RetroFuturaGUI::WindowTab::GetFieldStyle() const
{
    return _fieldStyle;
}

void RetroFuturaGUI::WindowTab::setFieldStyleDefaults()
{
    Background& background { _fieldStyle.GetBackground() };
    background.SetColors(std::array { glm::vec4(0.0f) }, ColorState::Enabled);
    background.SetColors(std::array { glm::vec4(1.0f, 1.0f, 1.0f, 0.06f) }, ColorState::Hover);
    background.SetColors(std::array { glm::vec4(1.0f, 1.0f, 1.0f, 0.1f) }, ColorState::Clicked);
    background.SetColors(std::array { glm::vec4(0.0f) }, ColorState::Disabled);
    background.SetColors(std::array { glm::vec4(0.086f, 0.086f, 0.1137f, 1.0f) }, ColorState::Selected); //a window's default background, so the selected field joins its content.
    background.SetCornerRadii(glm::vec4(8.0f, 8.0f, 0.0f, 0.0f));

    Border& border { _fieldStyle.GetBorder() };
    border.SetColors(std::array { glm::vec4(0.35f, 0.35f, 0.45f, 1.0f) }, ColorState::Enabled);
    border.SetColors(std::array { glm::vec4(0.45f, 0.45f, 0.55f, 1.0f) }, ColorState::Hover);
    border.SetColors(std::array { glm::vec4(0.5f, 0.5f, 0.6f, 1.0f) }, ColorState::Clicked);
    border.SetColors(std::array { glm::vec4(0.25f, 0.25f, 0.3f, 1.0f) }, ColorState::Disabled);
    border.SetColors(std::array { glm::vec4(0.35f, 0.35f, 0.45f, 1.0f) }, ColorState::Selected);
    border.SetCornerRadii(glm::vec4(8.0f, 8.0f, 0.0f, 0.0f));
    border.SetWidth(0.0f); //no border until one is asked for.
}

RetroFuturaGUI::Icon& RetroFuturaGUI::WindowTab::GetCloseIcon()
{
    return _closeIcon;
}

const RetroFuturaGUI::Icon& RetroFuturaGUI::WindowTab::GetCloseIcon() const
{
    return _closeIcon;
}

RetroFuturaGUI::Background& RetroFuturaGUI::WindowTab::GetBackground()
{
    return _background;
}

const RetroFuturaGUI::Background& RetroFuturaGUI::WindowTab::GetBackground() const
{
    return _background;
}

RetroFuturaGUI::Border& RetroFuturaGUI::WindowTab::GetBorder()
{
    return _border;
}

const RetroFuturaGUI::Border& RetroFuturaGUI::WindowTab::GetBorder() const
{
    return _border;
}