#include "ITab.hpp"
#include "PlatformBridge.hpp"
#include "Shader.hpp"
#include "Window.hpp"
#include <algorithm>
#include <cmath>
#include <filesystem>
#include <optional>

RetroFuturaGUI::ITab::ITab(std::string_view name, Projection* projection, IHierarchyNode* parentWidget, const WidgetTypeID parentWidgetTypeID, GLFWwindow* parentWindow)
    : IWidget(name, projection, parentWidget, parentWidgetTypeID, parentWindow)
{
    //The field elements draw once per field, in its colors, so they switch at once
    _background.SetTransitionSettings(&_transitionSettings);
    _border.SetTransitionSettings(&_transitionSettings);
    useBorderGap(&_background, &_border);
    useFlavors(&_background, &_border, nullptr);
    useSubPartFlavors(_fieldStyle.GetBackground(), StylePart::FieldStyleBackground);
    useSubPartFlavors(_fieldStyle.GetBorder(), StylePart::FieldStyleBorder);
    useSubPartFlavors(_closeButton.GetBackground(), StylePart::CloseButtonBackground);
    useSubPartFlavors(_closeButton.GetIcon(), StylePart::CloseButtonIcon);
    useSubPartFlavors(_scrollButton.GetBackground(), StylePart::ScrollButtonBackground);
    useSubPartFlavors(_scrollButton.GetIcon(), StylePart::ScrollButtonIcon);

    _border.SetBuiltInWidth(styleKey(), 0.0f); //no frame until one is asked for, since a Frame starts with a 5 px border
    _background.SetBuiltInColors(styleKey(), std::array { glm::vec4(0.055f, 0.055f, 0.075f, 1.0f) }, ColorState::Enabled);
    _background.SetBuiltInColors(styleKey(), std::array { glm::vec4(0.055f, 0.055f, 0.075f, 1.0f) }, ColorState::Disabled);
    _closeButton.GetBackground().SetBuiltInCornerRadii(styleKey(), glm::vec4(_closeButtonSize * 0.25f));
    _scrollButton.GetBackground().SetBuiltInCornerRadii(styleKey(), glm::vec4(_closeButtonSize * 0.25f));
    setFieldStyleDefaults();
}

void RetroFuturaGUI::ITab::applyPosition(const glm::vec3& position)
{
    IWidget::applyPosition(position);
    _background.SetPosition(position);
    _border.SetPosition(position + glm::vec3(0.0f, 0.0f, _zStep * 8.0f)); //over the fields
    contentAreaChanged();
}

void RetroFuturaGUI::ITab::applySize(const glm::vec3& size)
{
    IWidget::applySize(size);
    _background.SetSize(glm::vec2(size));
    _border.SetSize(glm::vec2(size));
    contentAreaChanged();
}

void RetroFuturaGUI::ITab::SetStripHeight(const f32 height)
{
    _stripHeight = std::max(height, 0.0f);
    contentAreaChanged(); //the content area changed with it.
}

void RetroFuturaGUI::ITab::SetFieldWidthRange(const f32 minimumWidth, const f32 maximumWidth)
{
    _minimumFieldWidth = std::max(minimumWidth, 1.0f);
    _maximumFieldWidth = std::max(maximumWidth, _minimumFieldWidth);
}

void RetroFuturaGUI::ITab::SetFieldSpacing(const f32 spacing)
{
    _fieldSpacing = std::max(spacing, 0.0f);
}

void RetroFuturaGUI::ITab::SetTitleFontFamily(std::string_view fontFamily, const f32 fontSize, const FontSizeUnit sizeUnit, const PlatformBridge::Fonts::Slant slant, const PlatformBridge::Fonts::Weight weight)
{
    _titleFontFamily = fontFamily;
    _titleFontSize = FontManager::ToPixels(fontSize, sizeUnit);
    _titleFontSlant = slant;
    _titleFontWeight = weight;

    for(Field& field : _fields)
        applyTitleFont(field._TitleText.get());

    titleFontChanged();
}

f32 RetroFuturaGUI::ITab::GetTitleFontSize(const FontSizeUnit sizeUnit) const
{
    return FontManager::FromPixels(_titleFontSize, sizeUnit);
}

void RetroFuturaGUI::ITab::SetTitleColors(std::span<const glm::vec4> colors, const ColorState state)
{
    if(ColorState::Selected == state)
        return; //the selected field's title shows the same colors as the others

    colorOf(_titleColors, state).assign(colors.begin(), colors.end());
}

void RetroFuturaGUI::ITab::SetTitlePadding(const f32 padding)
{
    _titlePadding = std::max(padding, 0.0f);
}

void RetroFuturaGUI::ITab::SetCloseButtonSize(const f32 size)
{
    _closeButtonSize = std::max(size, 0.0f);

    //Rasterized at twice the size the cross draws at, so it stays smooth when it is scaled down
    const i32 resolution { std::max(static_cast<i32>(std::lround(_closeButtonSize * _closeIconScale * 2.0f)), 8) };
    _closeButton.GetIcon().Rasterize(glm::i32vec2(resolution));
    _closeButton.GetBackground().SetBuiltInCornerRadii(styleKey(), glm::vec4(_closeButtonSize * 0.25f));
    _scrollButton.GetIcon().Rasterize(glm::i32vec2(resolution)); //the arrows are as large as the close buttons.
    _scrollButton.GetBackground().SetBuiltInCornerRadii(styleKey(), glm::vec4(_closeButtonSize * 0.25f));
}

RetroFuturaGUI::IconButton& RetroFuturaGUI::ITab::GetCloseButton()
{
    return _closeButton;
}

const RetroFuturaGUI::IconButton& RetroFuturaGUI::ITab::GetCloseButton() const
{
    return _closeButton;
}

RetroFuturaGUI::IconButton& RetroFuturaGUI::ITab::GetScrollButton()
{
    return _scrollButton;
}

const RetroFuturaGUI::IconButton& RetroFuturaGUI::ITab::GetScrollButton() const
{
    return _scrollButton;
}

RetroFuturaGUI::Box& RetroFuturaGUI::ITab::GetFieldStyle()
{
    return _fieldStyle;
}

const RetroFuturaGUI::Box& RetroFuturaGUI::ITab::GetFieldStyle() const
{
    return _fieldStyle;
}

void RetroFuturaGUI::ITab::SetContentCornerRadii(const glm::vec4& radii)
{
    _contentCornerRadii = radii;
    contentCornerRadiiChanged();
}

const glm::vec4& RetroFuturaGUI::ITab::GetContentCornerRadii() const
{
    return _contentCornerRadii;
}

RetroFuturaGUI::Background& RetroFuturaGUI::ITab::GetBackground()
{
    return _background;
}

const RetroFuturaGUI::Background& RetroFuturaGUI::ITab::GetBackground() const
{
    return _background;
}

RetroFuturaGUI::Border& RetroFuturaGUI::ITab::GetBorder()
{
    return _border;
}

const RetroFuturaGUI::Border& RetroFuturaGUI::ITab::GetBorder() const
{
    return _border;
}

bool RetroFuturaGUI::ITab::showsCloseButtons() const
{
    return true;
}

bool RetroFuturaGUI::ITab::dragsOutOfStrip([[maybe_unused]] const uSize index, [[maybe_unused]] const glm::vec2& point)
{
    return false;
}

void RetroFuturaGUI::ITab::fieldsReordered()
{
}

uSize RetroFuturaGUI::ITab::getSlotCount() const
{
    return _fields.size();
}

uSize RetroFuturaGUI::ITab::getFieldSlot(const uSize index) const
{
    return index;
}

void RetroFuturaGUI::ITab::contentAreaChanged()
{
}

void RetroFuturaGUI::ITab::contentCornerRadiiChanged()
{
}

void RetroFuturaGUI::ITab::titleFontChanged()
{
}

uSize RetroFuturaGUI::ITab::insertField(IHierarchyNode* item, std::string_view title, const uSize index)
{
    std::unique_ptr<Text> titleText { std::make_unique<Text>(&_projection) };
    applyTitleFont(titleText.get());
    titleText->SetTextUTF8(title);
    const uSize place { std::min(index, _fields.size()) };
    _fields.insert(std::next(_fields.begin(), static_cast<i64>(place)),
                   Field { ._Item = item, ._Title = std::string(title), ._TitleText = std::move(titleText), ._Shown = {}, ._HasShown = false });
    return place;
}

void RetroFuturaGUI::ITab::selectField(const uSize index, const bool emitSignal)
{
    if(index >= _fields.size())
        return;

    IHierarchyNode* item { _fields[index]._Item };

    if(item == _selectedItem)
        return;

    _selectedItem = item;
    scrollSlotIntoView(index);
    fieldSelected(item, emitSignal);
}

void RetroFuturaGUI::ITab::setFieldTitle(const uSize index, std::string_view title)
{
    if(index >= _fields.size())
        return;

    Field& field { _fields[index] };

    if(field._Title == title)
        return;

    field._Title = title;

    if(!field._TitleText)
        return;

    field._TitleText->SetTextUTF8(title);
}

bool RetroFuturaGUI::ITab::eraseField(const uSize index)
{
    if(index >= _fields.size())
        return false;

    IHierarchyNode* item { _fields[index]._Item };
    _fields.erase(std::next(_fields.begin(), static_cast<i64>(index)));
    _hoveredField = _kNoField;
    _hoveredCloseButton = _kNoField;
    _pressedCloseButton = _kNoField;

    if(_pressedItem == item)
    {
        _pressedItem = nullptr;
        _isDraggingField = false;
    }

    if(_selectedItem != item)
        return false;

    _selectedItem = nullptr;
    return true;
}

void RetroFuturaGUI::ITab::reselectAfterErase(const uSize index)
{
    if(_fields.empty())
    {
        selectionCleared();
        return;
    }

    //The field now in its place takes over, or the one before it when it was the last
    selectField(std::min(index, _fields.size() - 1), true);
}

uSize RetroFuturaGUI::ITab::indexOf(const IHierarchyNode* item) const
{
    if(!item)
        return _kNoField;

    for(uSize index = 0; index < _fields.size(); ++index)
    {
        if(_fields[index]._Item == item)
            return index;
    }

    return _kNoField;
}

void RetroFuturaGUI::ITab::followPointer()
{
    const PointerState pointer { readPointer() };
    const bool
        isPressed { pointer._LeftButtonPressed },
        pressedThisFrame { isPressed && !_wasPressed };
    _wasPressed = isPressed;

    if(!isPressed)
    {
        _pressedItem = nullptr;
        _isDraggingField = false;
        _pressedCloseButton = _kNoField;
        _pressedScrollButton = ScrollSide::None;
    }

    if(pointer._IsCaptured || !pointer._HasPosition)
    {
        _hoveredField = _kNoField;
        _hoveredCloseButton = _kNoField;
        _hoveredScrollButton = ScrollSide::None;
        return; //a ScrollView above has taken the pointer, or a popup lies over the window
    }

    const glm::vec2 point { pointer._Position };
    _hoveredField = fieldAt(point);
    _hoveredCloseButton = showsCloseButtons() && _kNoField != _hoveredField && contains(closeButtonRect(fieldRect(_hoveredField)), point) ? _hoveredField : _kNoField;
    _hoveredScrollButton = ScrollSide::None;

    if(hasHiddenFields(false) && contains(scrollButtonRect(false), point))
        _hoveredScrollButton = ScrollSide::Left;
    else if(hasHiddenFields(true) && contains(scrollButtonRect(true), point))
        _hoveredScrollButton = ScrollSide::Right;

    if(!_isEnabledFlag)
        return;

    scrollByWheel(pointer);

    //An arrow scrolls the strip by one field, and held a moment, on until it is let go. It pauses while the pointer is off it
    if(pressedThisFrame && ScrollSide::None != _hoveredScrollButton)
    {
        _pressedScrollButton = _hoveredScrollButton;
        _scrollHoldSeconds = 0.0f;
        _dragScrollClock = StepClock {};
        _dragScrollClock.TakeSeconds(); //starts the clock
        scrollStrip((ScrollSide::Right == _hoveredScrollButton ? 1.0f : -1.0f) * (getFieldWidth() + _fieldSpacing));
        return;
    }

    if(isPressed && ScrollSide::None != _pressedScrollButton)
    {
        const f32 seconds { _dragScrollClock.TakeSeconds() };
        _scrollHoldSeconds += seconds;

        if(_hoveredScrollButton != _pressedScrollButton || _scrollRepeatDelay > _scrollHoldSeconds)
            return;

        scrollStrip((ScrollSide::Right == _pressedScrollButton ? 1.0f : -1.0f) * _dragScrollSpeed * seconds);
        return;
    }

    if(pressedThisFrame)
    {
        //Like a window bar's close button, the cross acts on-press.
        if(_kNoField != _hoveredCloseButton)
        {
            _pressedCloseButton = _hoveredCloseButton;
            closeField(_hoveredCloseButton);
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

void RetroFuturaGUI::ITab::pressField(const uSize index, const glm::vec2& point)
{
    if(index >= _fields.size())
        return;

    IHierarchyNode* item { _fields[index]._Item };

    if(!item)
        return;

    const glm::vec4 field { slotRect(getFieldSlot(index)) };
    _pressedItem = item;
    _pressPoint = point;
    _grabOffset = glm::vec2(point.x - field.x, field.y + field.w - point.y); //from the top-left corner, y running downward
    _dragLeft = field.x;
    _sidewaysTravel = 0.0f;
    _isDraggingField = false;
    _dragScrollClock = StepClock {};
    selectField(index, true);
}

void RetroFuturaGUI::ITab::dragField(const glm::vec2& point)
{
    if(!_pressedItem)
        return;

    const uSize index { indexOf(_pressedItem) };

    if(_kNoField == index)
    {
        _pressedItem = nullptr;
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

    if(dragsOutOfStrip(index, point))
        return;

    //Held past an end of the field area, it scrolls the strip along.
    const glm::vec4 area { getFieldArea() };
    const f32 seconds { _dragScrollClock.TakeSeconds() };

    if(point.x < area.x)
        scrollStrip(-_dragScrollSpeed * seconds);
    else if(point.x > area.x + area.z)
        scrollStrip(_dragScrollSpeed * seconds);

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

    fieldsReordered();
}

uSize RetroFuturaGUI::ITab::fieldAt(const glm::vec2& point) const
{
    if(!contains(getFieldArea(), point))
        return _kNoField;

    //The dragged field lies over the others
    const uSize dragged { _isDraggingField ? indexOf(_pressedItem) : _kNoField };

    if(_kNoField != dragged && contains(fieldRect(dragged), point))
        return dragged;

    for(uSize index = 0; index < _fields.size(); ++index)
    {
        const glm::vec4 field { fieldRect(index) };

        if(!isInFieldArea(field))
            continue;

        if(contains(field, point))
            return index;
    }

    return _kNoField;
}

RetroFuturaGUI::ColorState RetroFuturaGUI::ITab::fieldState(const uSize index) const
{
    if(!_isEnabledFlag)
        return ColorState::Disabled;

    if(index < _fields.size() && _pressedItem == _fields[index]._Item)
        return ColorState::Clicked;

    if(index == _hoveredField)
        return ColorState::Hover;

    return ColorState::Enabled;
}

glm::vec4 RetroFuturaGUI::ITab::getStripRect() const
{
    const f32
        height { std::min(std::max(_stripHeight, 0.0f), std::max(_size.y, 0.0f)) },
        left { _position.x - _size.x * 0.5f },
        top { _position.y + _size.y * 0.5f };

    return glm::vec4(left, top - height, _size.x, height);
}

glm::vec4 RetroFuturaGUI::ITab::getContentRect() const
{
    const glm::vec4 strip { getStripRect() };
    const f32 bottom { _position.y - _size.y * 0.5f };
    return glm::vec4(strip.x, bottom, strip.z, std::max(strip.y - bottom, 0.0f));
}

f32 RetroFuturaGUI::ITab::getFieldWidth() const
{
    const uSize count { getSlotCount() };

    if(0 == count)
        return _maximumFieldWidth;

    const f32 available { getStripRect().z - _fieldSpacing * static_cast<f32>(count + 1) };
    return std::clamp(available / static_cast<f32>(count), _minimumFieldWidth, _maximumFieldWidth);
}

glm::vec4 RetroFuturaGUI::ITab::slotRect(const uSize slot) const
{
    const glm::vec4 area { getFieldArea() };
    const f32 width { getFieldWidth() };

    //The fields stand on the strip's bottom edge, where the content begins, with the spacing above them, and scrolled left
    //by the strip's scroll while they don't fit.
    return glm::vec4(area.x - _stripScroll + _fieldSpacing + static_cast<f32>(slot) * (width + _fieldSpacing), area.y, width, std::max(area.w - _fieldSpacing, 0.0f));
}

glm::vec4 RetroFuturaGUI::ITab::fieldRect(const uSize index) const
{
    glm::vec4 field { slotRect(getFieldSlot(index)) };

    if(index >= _fields.size())
        return field;

    const glm::vec2 shown { _fields[index]._HasShown ? _fields[index]._Shown.Get() : getFieldTarget(index) };
    field.x = shown.x;
    field.z = shown.y;
    return field;
}

glm::vec2 RetroFuturaGUI::ITab::getFieldTarget(const uSize index) const
{
    const glm::vec4 slot { slotRect(getFieldSlot(index)) };

    if(!_isDraggingField)
        return glm::vec2(slot.x, slot.z);

    if(index >= _fields.size())
        return glm::vec2(slot.x, slot.z);

    if(_pressedItem != _fields[index]._Item)
        return glm::vec2(slot.x, slot.z);

    return glm::vec2(_dragLeft, slot.z);
}

void RetroFuturaGUI::ITab::syncFieldRects()
{
    const Transition transition { Transitions::Resolve(_transitionSettings, TransitionProperty::Position) };
    _stripScroll = std::clamp(_stripScroll, 0.0f, getMaxStripScroll()); //a removed field or a wider strip leaves less to scroll.

    for(uSize index = 0; index < _fields.size(); ++index)
    {
        Field& field { _fields[index] };
        const glm::vec2 target { getFieldTarget(index) };
        const bool isDragged { _isDraggingField && _pressedItem == field._Item };

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

uSize RetroFuturaGUI::ITab::slotAt(const f32 x) const
{
    if(_fields.empty())
        return 0;

    const glm::vec4 first { slotRect(0) };
    const f32
        width { getFieldWidth() },
        step { width + _fieldSpacing };

    if(0.0f >= step)
        return 0;

    const f32 slot { std::round((x - first.x - width * 0.5f) / step) };
    return static_cast<uSize>(std::clamp(slot, 0.0f, static_cast<f32>(_fields.size() - 1)));
}

bool RetroFuturaGUI::ITab::isInFieldArea(const glm::vec4& field) const
{
    const glm::vec4 area { getFieldArea() };
    return field.x + field.z > area.x + 0.5f && field.x < area.x + area.z - 0.5f; //half a pixel for rounding.
}

glm::vec4 RetroFuturaGUI::ITab::getFieldArea() const
{
    const glm::vec4 strip { getStripRect() };

    if(!isOverflowing())
        return strip;

    //Between the arrows, a spacing from each.
    const glm::vec4
        left { scrollButtonRect(false) },
        right { scrollButtonRect(true) };
    const f32 areaLeft { left.x + left.z };
    return glm::vec4(areaLeft, strip.y, std::max(right.x - areaLeft, 0.0f), strip.w);
}

bool RetroFuturaGUI::ITab::isOverflowing() const
{
    const uSize count { getSlotCount() };
    const f32 fieldsWidth { _fieldSpacing + static_cast<f32>(count) * (getFieldWidth() + _fieldSpacing) };
    return fieldsWidth > getStripRect().z + 0.5f; //half a pixel for rounding.
}

f32 RetroFuturaGUI::ITab::getMaxStripScroll() const
{
    if(!isOverflowing())
        return 0.0f;

    const f32 fieldsWidth { _fieldSpacing + static_cast<f32>(getSlotCount()) * (getFieldWidth() + _fieldSpacing) };
    return std::max(fieldsWidth - getFieldArea().z, 0.0f);
}

void RetroFuturaGUI::ITab::scrollStrip(const f32 distance)
{
    _stripScroll = std::clamp(_stripScroll + distance, 0.0f, getMaxStripScroll());
}

void RetroFuturaGUI::ITab::scrollSlotIntoView(const uSize slot)
{
    if(!isOverflowing())
        return;

    //Measured along the fields from their start, as the scroll is
    const f32
        visibleWidth { getFieldArea().z },
        width { getFieldWidth() },
        left { static_cast<f32>(slot) * (width + _fieldSpacing) },
        right { left + width + 2.0f * _fieldSpacing };

    if(left < _stripScroll)
        _stripScroll = left;
    else if(right > _stripScroll + visibleWidth)
        _stripScroll = right - visibleWidth;

    scrollStrip(0.0f); //kept within its range
}

glm::vec4 RetroFuturaGUI::ITab::scrollButtonRect(const bool isRight) const
{
    const glm::vec4 strip { getStripRect() };
    const f32
        fieldHeight { std::max(strip.w - _fieldSpacing, 0.0f) },
        size { std::min(_closeButtonSize, fieldHeight) },
        bottom { strip.y + (fieldHeight - size) * 0.5f }; //centered on the fields, which stand on the strip's bottom edge

    if(isRight)
        return glm::vec4(strip.x + strip.z - _fieldSpacing - size, bottom, size, size);

    return glm::vec4(strip.x + _fieldSpacing, bottom, size, size);
}

bool RetroFuturaGUI::ITab::hasHiddenFields(const bool isRight) const
{
    if(!isOverflowing())
        return false;

    return isRight ? _stripScroll < getMaxStripScroll() - 0.5f : 0.5f < _stripScroll;
}

void RetroFuturaGUI::ITab::scrollByWheel(const PointerState& pointer)
{
    if(!isOverflowing())
        return;

    if(!contains(getStripRect(), pointer._Position))
        return;

    //Toward the user or to the right shows the fields further on, a field per notch
    const f32 notches { pointer._Wheel.x - pointer._Wheel.y };

    if(0.0f == notches)
        return;

    scrollStrip(notches * (getFieldWidth() + _fieldSpacing));
    WheelParts takenParts { WheelParts::None };

    if(0.0f != pointer._Wheel.x)
        takenParts = takenParts | WheelParts::Horizontal;

    if(0.0f != pointer._Wheel.y)
        takenParts = takenParts | WheelParts::Vertical;

    Window::TakeWheel(_parentWindow, takenParts); //so nothing around the strip scrolls too
}

glm::vec4 RetroFuturaGUI::ITab::closeButtonRect(const glm::vec4& field) const
{
    const f32
        size { std::min(_closeButtonSize, field.w) },
        margin { (field.w - size) * 0.5f }; //as far from the field's right edge as from its top and bottom

    return glm::vec4(field.x + field.z - margin - size, field.y + margin, size, size);
}

glm::vec4 RetroFuturaGUI::ITab::titleRect(const glm::vec4& field) const
{
    const f32 left { field.x + _titlePadding };

    if(!showsCloseButtons())
        return glm::vec4(left, field.y, std::max(field.x + field.z - _titlePadding - left, 0.0f), field.w); //as far from the right edge as from the left

    const glm::vec4 closeButton { closeButtonRect(field) };
    return glm::vec4(left, field.y, std::max(closeButton.x - _fieldSpacing - left, 0.0f), field.w);
}

void RetroFuturaGUI::ITab::drawFields()
{
    {
        //While they don't fit, the fields at the area's ends are cut at its edges
        std::optional<Shader::ClipScope> clip {};

        if(isOverflowing())
            clip.emplace(getFieldArea(), _position.z, _projection);

        const uSize dragged { _isDraggingField ? indexOf(_pressedItem) : _kNoField };

        for(uSize index = 0; index < _fields.size(); ++index)
        {
            if(index == dragged)
                continue;

            drawField(index, _position.z + _zStep * 2.0f);
        }

        if(_kNoField != dragged)
            drawField(dragged, _position.z + _zStep * 5.0f); //over the others
    }

    drawScrollButtons(_position.z + _zStep * 2.0f);
}

void RetroFuturaGUI::ITab::drawScrollButtons(const f32 z)
{
    if(!isOverflowing())
        return;

    for(const bool isRight : { false, true })
    {
        if(!hasHiddenFields(isRight))
            continue;

        drawScrollButton(isRight, z);
    }
}

void RetroFuturaGUI::ITab::drawScrollButton(const bool isRight, const f32 z)
{
    const glm::vec4 button { scrollButtonRect(isRight) };

    if(0.0f >= button.z)
        return;

    const ScrollSide side { isRight ? ScrollSide::Right : ScrollSide::Left };
    ColorState state { _isEnabledFlag ? ColorState::Enabled : ColorState::Disabled };

    if(side == _hoveredScrollButton)
        state = ColorState::Hover;

    if(side == _pressedScrollButton)
        state = ColorState::Clicked;

    const glm::vec3 center { button.x + button.z * 0.5f, button.y + button.w * 0.5f, z };
    Background& background { _scrollButton.GetBackground() };

    if(isVisible(background.GetShownColors(state)))
    {
        background.SetColorState(state);
        background.SetSize(glm::vec2(button.z, button.w));
        background.SetPosition(center);
        background.Draw();
    }

    //The arrow points left; the right one is turned around.
    Icon& icon { _scrollButton.GetIcon() };
    icon.SetColorState(state);
    icon.SetSize(glm::vec2(button.z * _closeIconScale));
    icon.SetRotation(glm::vec3(0.0f, 0.0f, isRight ? 180.0f : 0.0f));
    icon.SetPosition(center + glm::vec3(0.0f, 0.0f, _zStep));
    icon.Draw();
}

void RetroFuturaGUI::ITab::drawField(const uSize index, const f32 z)
{
    if(index >= _fields.size())
        return;

    const Field& field { _fields[index] };

    if(!field._Item)
        return;

    const glm::vec4 rect { fieldRect(index) };

    if(!isInFieldArea(rect))
        return;

    const bool selected { field._Item == _selectedItem };
    const ColorState state { fieldState(index) };
    drawFieldShape(rect, z, _fieldStyle, fieldStyleState(state, selected));
    drawTitle(field._TitleText.get(), rect, z + _zStep, colorFor(_titleColors, state, false));

    if(!showsCloseButtons())
        return;

    ColorState closeState { _isEnabledFlag ? ColorState::Enabled : ColorState::Disabled };

    if(index == _hoveredCloseButton)
        closeState = ColorState::Hover;

    if(index == _pressedCloseButton)
        closeState = ColorState::Clicked;

    drawCloseButton(rect, z + _zStep, closeState);
}

void RetroFuturaGUI::ITab::drawFieldShape(const glm::vec4& rect, const f32 z, Box& style, const ColorState state)
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

void RetroFuturaGUI::ITab::drawCloseButton(const glm::vec4& field, const f32 z, const ColorState state)
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
    icon.SetColorState(state); //a state without colors shows the Enabled ones
    icon.SetSize(glm::vec2(button.z * _closeIconScale)); //the cross keeps clear of the button's edges
    icon.SetPosition(center + glm::vec3(0.0f, 0.0f, _zStep));
    icon.Draw();
}

void RetroFuturaGUI::ITab::drawTitle(Text* title, const glm::vec4& field, const f32 z, const std::vector<glm::vec4>& colors)
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

void RetroFuturaGUI::ITab::applyTitleFont(Text* title) const
{
    if(!title)
        return;

    title->SetTextAlignment(TextAlignment::Left);

    if(_titleFontFamily.empty())
        return;

    title->SetFontFamily(_titleFontFamily, _titleFontSize, FontSizeUnit::Pixels, _titleFontSlant, _titleFontWeight);
}

RetroFuturaGUI::ColorState RetroFuturaGUI::ITab::fieldStyleState(const ColorState state, const bool selected)
{
    if(ColorState::Disabled == state)
        return ColorState::Disabled;

    return selected ? ColorState::Selected : state;
}

const std::vector<glm::vec4>& RetroFuturaGUI::ITab::colorFor(const FieldColors& colors, const ColorState state, const bool selected)
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

std::vector<glm::vec4>& RetroFuturaGUI::ITab::colorOf(FieldColors& colors, const ColorState state)
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

bool RetroFuturaGUI::ITab::isVisible(std::span<const glm::vec4> colors)
{
    return std::any_of(colors.begin(), colors.end(), [](const glm::vec4& color) { return 0.0f < color.a; });
}

bool RetroFuturaGUI::ITab::contains(const glm::vec4& rect, const glm::vec2& point)
{
    return point.x >= rect.x && point.x <= rect.x + rect.z && point.y >= rect.y && point.y <= rect.y + rect.w;
}

std::string RetroFuturaGUI::ITab::getImagePath(std::string_view fileName)
{
    std::filesystem::path path { PlatformBridge::Paths::GetModuleFolder() };
    path /= "Resources";
    path /= "img";
    path /= std::u8string(fileName.begin(), fileName.end());

    //Not path.string(): on Windows that goes through the ANSI code page and turns names outside it into '?'
    const std::u8string text { path.u8string() };
    return std::string(text.begin(), text.end());
}

void RetroFuturaGUI::ITab::setFieldStyleDefaults()
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