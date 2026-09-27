#include "MenuBar.hpp"
#include "PlatformBridge.hpp"

#if defined(TARGET_PLATFORM_LINUX)
    #define GLFW_EXPOSE_NATIVE_X11
#elif defined(TARGET_PLATFORM_WINDOWS)
    #define GLFW_EXPOSE_NATIVE_WIN32
#endif
#include <GLFW/glfw3native.h>

RetroFuturaGUI::MenuBar::MenuBar(const std::string& name, Projection* projection, IWidget* parentWidget, const WidgetTypeID parentWidgetTypeID, GLFWwindow* parentWindow)
    : IWidget(name, projection, parentWidget, parentWidgetTypeID, parentWindow)
{
    _widgetTypeID = WidgetTypeID::MenuBar;
    _background = std::make_unique<Rectangle>(projection);
    _border = std::make_unique<Rectangle>(projection);
    _itemBackground = std::make_unique<Rectangle>(projection);
    _itemBorder = std::make_unique<Rectangle>(projection);

    if(_background)
        _background->SetRectangleMode(RectangleMode::Plane);

    if(_border)
        _border->SetRectangleMode(RectangleMode::Border);

    if(_itemBackground)
        _itemBackground->SetRectangleMode(RectangleMode::Plane);

    if(_itemBorder)
        _itemBorder->SetRectangleMode(RectangleMode::Border);
}

void RetroFuturaGUI::MenuBar::Draw()
{
    interact();
    drawBackground();
    drawBorder();
    drawSeparatorLines();
    drawItems();
}

void RetroFuturaGUI::MenuBar::SetPosition(const glm::vec3& position)
{
    IWidget::SetPosition(position);

    if(_background)
        _background->SetPosition(position);

    if(_border)
        _border->SetPosition(position + glm::vec3(0.0f, 0.0f, 0.01f));

    placeItems();
}

void RetroFuturaGUI::MenuBar::SetSize(const glm::vec3& size)
{
    IWidget::SetSize(size);

    if(_background)
        _background->SetSize(size);

    if(_border)
        _border->SetSize(size);

    updateLayout();
}

void RetroFuturaGUI::MenuBar::SetRotation(const glm::vec3& rotation)
{
    IWidget::SetRotation(rotation);

    if(_background)
        _background->SetRotation(rotation);

    if(_border)
        _border->SetRotation(rotation);

    for(const std::unique_ptr<IWidget>& item : _itemFlex)
    {
        if(!item)
            continue;

        item->SetRotation(rotation);
    }
}

void RetroFuturaGUI::MenuBar::SetMargin(const f32 margin)
{
    _margin = margin;
    updateLayout();
}

void RetroFuturaGUI::MenuBar::SetDockingEdge(const DockingEdge dockingEdge)
{
    _dockingEdge = dockingEdge;
    updateLayout();
}

void RetroFuturaGUI::MenuBar::SetEdgeAlignment(const EdgeAlignment edgeAlignment)
{
    _edgeAlignment = edgeAlignment;
    updateLayout();
}

void RetroFuturaGUI::MenuBar::SetFlexCount(const uSize count)
{
    _itemFlex.resize(count);

    /* Cells added here have no policy of their own, so they take an even share. Cells that already had
       one keep it, which is what lets a count change follow a SetFlexDefinition without undoing it. */
    _flexDefinition.resize(count, _kEvenShare);

    if(_hoveredItemIndex >= _itemFlex.size())
        _hoveredItemIndex = _kNoItem;

    placeItems();
}

void RetroFuturaGUI::MenuBar::SetFlexDefinition(std::span<FlexDefinition> flexDefinitions)
{
    _flexDefinition.assign(flexDefinitions.begin(), flexDefinitions.end());

    //The definitions are what say how many cells there are, so the flex follows them.
    _itemFlex.resize(_flexDefinition.size());

    if(_hoveredItemIndex >= _itemFlex.size())
        _hoveredItemIndex = _kNoItem;

    placeItems();
}

const RetroFuturaGUI::MenuBar::FlexDefinition& RetroFuturaGUI::MenuBar::GetFlexDefinition(const uSize index) const
{
    if(index >= _flexDefinition.size())
        return _kEvenShare;

    return _flexDefinition[index];
}

bool RetroFuturaGUI::MenuBar::RemoveWidget(const uSize index)
{
    if(index >= _itemFlex.size())
        return false;

    std::unique_ptr<IWidget>& cell { *std::next(_itemFlex.begin(), static_cast<i64>(index)) };

    if(!cell)
        return false;

    cell.reset();
    return true;
}

void RetroFuturaGUI::MenuBar::EnableSeparatorLines(const bool enable)
{
    _useSeparatorLines = enable;

    if(!_useSeparatorLines)
        return;

    if(!_separatorLine)
    {
        _separatorLine = std::make_unique<Rectangle>(&_projection);

        if(_separatorLine)
            _separatorLine->SetRectangleMode(RectangleMode::Plane);
    }

    setSeparatorLinesColors();
}

void RetroFuturaGUI::MenuBar::SetSeparatorLinesThickness(const f32 thickness)
{
    _separatorLinesThickness = thickness;
}

void RetroFuturaGUI::MenuBar::SetSeparatorLinesColor(const glm::vec4& color, const ColorState state)
{
    switch(state)
    {
        case ColorState::Enabled:
            _separatorLinesColorsEnabled.clear();
            _separatorLinesColorsEnabled.resize(1, color);
        break;
        case ColorState::Disabled:
            _separatorLinesColorsDisabled.clear();
            _separatorLinesColorsDisabled.resize(1, color);
        break;
        default:
            [[unlikely]]
        break;
    }

    setSeparatorLinesColors();
}

void RetroFuturaGUI::MenuBar::SetSeparatorLinesColors(std::span<glm::vec4> colors, const ColorState state)
{
    switch(state)
    {
        case ColorState::Enabled:
            _separatorLinesColorsEnabled.assign(colors.begin(), colors.end());
        break;
        case ColorState::Disabled:
            _separatorLinesColorsDisabled.assign(colors.begin(), colors.end());
        break;
        default:
            [[unlikely]]
        break;
    }

    setSeparatorLinesColors();
}

void RetroFuturaGUI::MenuBar::SetSeparatorLinesFillType(const FillType fillType)
{
    if(!_separatorLine)
        return;

    _separatorLine->SetFillType(fillType);
}

void RetroFuturaGUI::MenuBar::SetSeparatorLinesGradientOffset(const f32 gradientOffset)
{
    if(!_separatorLine)
        return;

    _separatorLine->SetGradientOffset(gradientOffset);
}

void RetroFuturaGUI::MenuBar::SetSeparatorLinesGradientAnimationSpeed(const f32 animationSpeed)
{
    if(!_separatorLine)
        return;

    _separatorLine->SetGradientAnimationSpeed(animationSpeed);
}

void RetroFuturaGUI::MenuBar::SetSeparatorLinesGradientDegree(const f32 degree)
{
    if(!_separatorLine)
        return;

    _separatorLine->SetGradientDegree(degree);
}

void RetroFuturaGUI::MenuBar::SetSeparatorLinesGradientRotationSpeed(const f32 rotationSpeed)
{
    if(!_separatorLine)
        return;

    _separatorLine->SetGradientRotationSpeed(rotationSpeed);
}

void RetroFuturaGUI::MenuBar::SetSeparatorLinesGaps(const BackgroundGap& gap)
{
    if(!_separatorLine)
        return;

    _separatorLine->SetBackgroundGaps(gap);
}

void RetroFuturaGUI::MenuBar::SetItemBackgroundColor(const glm::vec4& color, const ColorState state)
{
    switch(state)
    {
        case ColorState::Enabled:
            _itemBackgroundColorsEnabled.clear();
            _itemBackgroundColorsEnabled.resize(1, color);
        break;
        case ColorState::Disabled:
            _itemBackgroundColorsDisabled.clear();
            _itemBackgroundColorsDisabled.resize(1, color);
        break;
        case ColorState::Clicked:
            _itemBackgroundColorsClick.clear();
            _itemBackgroundColorsClick.resize(1, color);
        break;
        case ColorState::Hover:
            _itemBackgroundColorsHover.clear();
            _itemBackgroundColorsHover.resize(1, color);
        break;
        default:
            [[unlikely]]
        break;
    }

    setItemBackgroundColors();
}

void RetroFuturaGUI::MenuBar::SetItemBackgroundColors(std::span<glm::vec4> colors, const ColorState state)
{
    switch(state)
    {
        case ColorState::Enabled:
            _itemBackgroundColorsEnabled.assign(colors.begin(), colors.end());
        break;
        case ColorState::Disabled:
            _itemBackgroundColorsDisabled.assign(colors.begin(), colors.end());
        break;
        case ColorState::Clicked:
            _itemBackgroundColorsClick.assign(colors.begin(), colors.end());
        break;
        case ColorState::Hover:
            _itemBackgroundColorsHover.assign(colors.begin(), colors.end());
        break;
        default:
            [[unlikely]]
        break;
    }

    setItemBackgroundColors();
}

void RetroFuturaGUI::MenuBar::SetItemCornerRadii(const glm::vec4& radii)
{
    setItemBackgroundCornerRadii(radii);
    setItemBorderCornerRadii(radii);
}

void RetroFuturaGUI::MenuBar::SetItemBackgroundFillType(const FillType fillType)
{
    if(!_itemBackground)
        return;

    _itemBackground->SetFillType(fillType);
}

void RetroFuturaGUI::MenuBar::SetItemBackgroundGradientOffset(const f32 gradientOffset)
{
    if(!_itemBackground)
        return;

    _itemBackground->SetGradientOffset(gradientOffset);
}

void RetroFuturaGUI::MenuBar::SetItemBackgroundGradientAnimationSpeed(const f32 animationSpeed)
{
    if(!_itemBackground)
        return;

    _itemBackground->SetGradientAnimationSpeed(animationSpeed);
}

void RetroFuturaGUI::MenuBar::SetItemBackgroundGradientDegree(const f32 degree)
{
    if(!_itemBackground)
        return;

    _itemBackground->SetGradientDegree(degree);
}

void RetroFuturaGUI::MenuBar::SetItemBackgroundGradientRotationSpeed(const f32 rotationSpeed)
{
    if(!_itemBackground)
        return;

    _itemBackground->SetGradientRotationSpeed(rotationSpeed);
}

void RetroFuturaGUI::MenuBar::SetItemWindowBackgroundImageTextureID(const u32 textureID)
{
    if(!_itemBackground)
        return;

    _itemBackground->SetWindowBackgroundImageTextureID(textureID);
}

void RetroFuturaGUI::MenuBar::SetItemBackgroundDotColor(const glm::vec4& color)
{
    if(!_itemBackground)
        return;

    _itemBackground->SetDotColor(color);
}

void RetroFuturaGUI::MenuBar::SetItemBackgroundDotDistance(const f32 distance)
{
    if(!_itemBackground)
        return;

    _itemBackground->SetDotDistance(distance);
}

void RetroFuturaGUI::MenuBar::SetItemBackgroundDotSizeTransferDegree(const f32 degree)
{
    if(!_itemBackground)
        return;

    _itemBackground->SetDotSizeTransferDegree(degree);
}

void RetroFuturaGUI::MenuBar::SetItemBackgroundDotRadiusTransfer(std::span<f32> radiusTransfer)
{
    /* Kept in the member vector because Rectangle holds the curve as a span,
       so the caller's container is free to die right after this call. */
    _itemBackgroundDotRadiusTransfer.assign(radiusTransfer.begin(), radiusTransfer.end());

    if(!_itemBackground)
        return;

    _itemBackground->SetDotRadiusTransfer(_itemBackgroundDotRadiusTransfer);
}

void RetroFuturaGUI::MenuBar::SetItemBackgroundDotTransparencyTransfer(const f32 transparencyTransfer)
{
    if(!_itemBackground)
        return;

    _itemBackground->SetDotTransparencyTransfer(transparencyTransfer);
}

void RetroFuturaGUI::MenuBar::SetItemBackgroundDotAnimationSpeed(const f32 animationSpeed)
{
    if(!_itemBackground)
        return;

    _itemBackground->SetDotAnimationSpeed(animationSpeed);
}

void RetroFuturaGUI::MenuBar::SetItemBackgroundFogAlpha(const f32 alpha)
{
    if(!_itemBackground)
        return;

    _itemBackground->SetFogAlpha(alpha);
}

void RetroFuturaGUI::MenuBar::SetItemBackgroundFogSpeed(const f32 speed)
{
    if(!_itemBackground)
        return;

    _itemBackground->SetFogSpeed(speed);
}

void RetroFuturaGUI::MenuBar::SetItemBackgroundFogDensity(std::span<f32> density)
{
    _itemBackgroundFogDensity.assign(density.begin(), density.end());

    if(!_itemBackground)
        return;

    _itemBackground->SetFogDensity(_itemBackgroundFogDensity);
}

void RetroFuturaGUI::MenuBar::SetItemBackgroundFogClearing(const f32 clearing)
{
    if(!_itemBackground)
        return;

    _itemBackground->SetFogClearing(clearing);
}

void RetroFuturaGUI::MenuBar::SetItemBackgroundGaps(const BackgroundGap& gap)
{
    if(!_itemBackground)
        return;

    _itemBackground->SetBackgroundGaps(gap);
}

//Item border

void RetroFuturaGUI::MenuBar::SetItemBorderColor(const glm::vec4& color, const ColorState state)
{
    switch(state)
    {
        case ColorState::Enabled:
            _itemBorderColorsEnabled.clear();
            _itemBorderColorsEnabled.resize(1, color);
        break;
        case ColorState::Disabled:
            _itemBorderColorsDisabled.clear();
            _itemBorderColorsDisabled.resize(1, color);
        break;
        case ColorState::Clicked:
            _itemBorderColorsClick.clear();
            _itemBorderColorsClick.resize(1, color);
        break;
        case ColorState::Hover:
            _itemBorderColorsHover.clear();
            _itemBorderColorsHover.resize(1, color);
        break;
        default: [[unlikely]]
        break;
    }

    setItemBorderColors();
}

void RetroFuturaGUI::MenuBar::SetItemBorderColors(std::span<glm::vec4> colors, const ColorState state)
{
    switch(state)
    {
        case ColorState::Enabled:
            _itemBorderColorsEnabled.assign(colors.begin(), colors.end());
        break;
        case ColorState::Disabled:
            _itemBorderColorsDisabled.assign(colors.begin(), colors.end());
        break;
        case ColorState::Clicked:
            _itemBorderColorsClick.assign(colors.begin(), colors.end());
        break;
        case ColorState::Hover:
            _itemBorderColorsHover.assign(colors.begin(), colors.end());
        break;
        default:
            [[unlikely]]
        break;
    }

    setItemBorderColors();
}

void RetroFuturaGUI::MenuBar::SetItemBorderFillType(const FillType fillType)
{
    if(!_itemBorder)
        return;

    _itemBorder->SetFillType(fillType);
}

void RetroFuturaGUI::MenuBar::SetItemBorderGradientOffset(const f32 gradientOffset)
{
    if(!_itemBorder)
        return;

    _itemBorder->SetGradientOffset(gradientOffset);
}

void RetroFuturaGUI::MenuBar::SetItemBorderGradientAnimationSpeed(const f32 animationSpeed)
{
    if(!_itemBorder)
        return;

    _itemBorder->SetGradientAnimationSpeed(animationSpeed);
}

void RetroFuturaGUI::MenuBar::SetItemBorderGradientDegree(const f32 degree)
{
    if(!_itemBorder)
        return;

    _itemBorder->SetGradientDegree(degree);
}

void RetroFuturaGUI::MenuBar::SetItemBorderGradientRotationSpeed(const f32 rotationSpeed)
{
    if(!_itemBorder)
        return;

    _itemBorder->SetGradientRotationSpeed(rotationSpeed);
}

void RetroFuturaGUI::MenuBar::SetItemWindowBorderImageTextureID(const u32 textureID)
{
    if(!_itemBorder)
        return;

    _itemBorder->SetWindowBackgroundImageTextureID(textureID);
}

void RetroFuturaGUI::MenuBar::SetItemBorderDotColor(const glm::vec4& color)
{
    if(!_itemBorder)
        return;

    _itemBorder->SetDotColor(color);
}

void RetroFuturaGUI::MenuBar::SetItemBorderDotDistance(const f32 distance)
{
    if(!_itemBorder)
        return;

    _itemBorder->SetDotDistance(distance);
}

void RetroFuturaGUI::MenuBar::SetItemBorderDotSizeTransferDegree(const f32 degree)
{
    if(!_itemBorder)
        return;

    _itemBorder->SetDotSizeTransferDegree(degree);
}

void RetroFuturaGUI::MenuBar::SetItemBorderDotRadiusTransfer(std::span<f32> radiusTransfer)
{
    _itemBorderDotRadiusTransfer.assign(radiusTransfer.begin(), radiusTransfer.end());

    if(!_itemBorder)
        return;

    _itemBorder->SetDotRadiusTransfer(_itemBorderDotRadiusTransfer);
}

void RetroFuturaGUI::MenuBar::SetItemBorderDotTransparencyTransfer(const f32 transparencyTransfer)
{
    if(!_itemBorder)
        return;

    _itemBorder->SetDotTransparencyTransfer(transparencyTransfer);
}

void RetroFuturaGUI::MenuBar::SetItemBorderDotAnimationSpeed(const f32 animationSpeed)
{
    if(!_itemBorder)
        return;

    _itemBorder->SetDotAnimationSpeed(animationSpeed);
}

void RetroFuturaGUI::MenuBar::SetItemBorderWidth(const f32 borderWidth)
{
    if(!_itemBorder)
        return;

    _itemBorder->SetBorderWidth(borderWidth);
}

void RetroFuturaGUI::MenuBar::SetItemBorderGaps(std::span<BorderGap> gaps)
{
    if(!_itemBorder)
        return;

    _itemBorder->SetBorderGaps(gaps);
}

void RetroFuturaGUI::MenuBar::interact()
{
    i32 mouseX { 0 }, mouseY { 0 };
    bool hasMousePosition { false };

#if defined(TARGET_PLATFORM_LINUX)
    hasMousePosition = PlatformBridge::Input::GetMouseWindowPosition(glfwGetX11Window(_parentWindow), mouseX, mouseY);
#elif defined(TARGET_PLATFORM_WINDOWS)
    hasMousePosition = PlatformBridge::Input::GetMouseWindowPosition(glfwGetWin32Window(_parentWindow), mouseX, mouseY);
#endif

    glm::vec2 mousePos { static_cast<f32>(mouseX), _projection.GetResolution().y - static_cast<f32>(mouseY) };
    bool isMouseButtonPressed { PlatformBridge::Input::IsMouseButtonDown(PlatformBridge::MouseButton::Left) };
    bool isMouseInside { hasMousePosition && isPointInside(mousePos) };

    updateWindowDrag(MouseState
    {
        ._WorldPoint = mousePos,
        ._WindowPoint = glm::i32vec2(mouseX, mouseY),
        ._HasPosition = hasMousePosition,
        ._IsPressed = isMouseButtonPressed
    });

    if(!_isEnabledFlag || !isMouseInside) //no action and mouse leave
    {
        _hoveredItemIndex = _kNoItem;

        if(_mouseEnteredFlag)
        {
            _mouseEnteredFlag = false;
            _onMouseLeaveAsync.EmitAsync();
            _onMouseLeave.Emit();
        }

        _wasClicked = isMouseButtonPressed;
        return;
    }

    _hoveredItemIndex = _kNoItem;

    for(uSize index { 0 }; index < _itemFlex.size(); ++index)
    {
        if(!isPointInsideRect(mousePos, glm::vec3(cellSize(index), _size.z), cellPosition(index), _rotation))
            continue;

        _hoveredItemIndex = index;
        break;
    }

    _whileHoverAsync.EmitAsync();
    _whileHover.Emit();

    if(!_mouseLeftFlag && !_mouseEnteredFlag) //enter
    {
        _mouseEnteredFlag = true;
        _onMouseEnterAsync.EmitAsync();
        _onMouseEnter.Emit();
    }

    if(isMouseButtonPressed && !_wasClicked) //click
    {
        _onClickAsync.EmitAsync();
        _onClick.Emit();
    }
    else if(!isMouseButtonPressed && _wasClicked) //release
    {
        _onReleaseAsync.EmitAsync();
        _onRelease.Emit();
    }

    _wasClicked = isMouseButtonPressed;
}

void RetroFuturaGUI::MenuBar::updateWindowDrag(const MouseState& mouse)
{
    if(!_parentWindow)
        return;

    const bool pressedThisFrame { mouse._IsPressed && !_wasDragMousePressed };
    _wasDragMousePressed = mouse._IsPressed;

    if(!mouse._IsPressed)
    {
        _isDraggingWindow = false;
        return;
    }

    if(!mouse._HasPosition)
        return;

    if(pressedThisFrame)
    {
        //Only the press that lands on a drag cell begins one
        if(!isPointInsideDragCell(mouse._WorldPoint))
            return;

        _isDraggingWindow = true;
        _dragGrabPoint = mouse._WindowPoint;
        return;
    }

    if(!_isDraggingWindow)
        return;

    // How far the cursor has moved from the point it took hold of
    const glm::i32vec2 slip { mouse._WindowPoint - _dragGrabPoint };

    if(slip.x == 0 && slip.y == 0)
        return;

    i32 windowX { 0 }, windowY { 0 };
    glfwGetWindowPos(_parentWindow, &windowX, &windowY);
    glfwSetWindowPos(_parentWindow, windowX + slip.x, windowY + slip.y);
}

bool RetroFuturaGUI::MenuBar::isPointInsideDragCell(const glm::vec2& point) const
{
    uSize index { 0 };

    for(const std::unique_ptr<IWidget>& item : _itemFlex)
    {
        const uSize cell { index };
        ++index;

        if(!GetFlexDefinition(cell)._IsWindowDrag)
            continue;

        //The flag only counts on a Label, so a cell holding something clickable keeps its clicks.
        if(!dynamic_cast<Label*>(item.get()))
            continue;

        if(!isPointInsideRect(point, glm::vec3(cellSize(cell), _size.z), cellPosition(cell), _rotation))
            continue;

        return true;
    }

    return false;
}

void RetroFuturaGUI::MenuBar::updateLayout()
{
    SetPosition(calculateDockedPosition());
}

void RetroFuturaGUI::MenuBar::placeItems()
{
    resolveCellSizes();

    uSize index { 0 };

    for(const std::unique_ptr<IWidget>& item : _itemFlex)
    {
        const glm::vec3 center { cellPosition(index) };
        ++index;

        if(!item)
            continue;

        item->SetPosition(glm::vec3(center.x, center.y, center.z + 0.02f));
    }
}

f32 RetroFuturaGUI::MenuBar::alignAlongEdge(const f32 edgeLength, const f32 barLength) const
{
    f32 center { edgeLength * 0.5f };

    switch(_edgeAlignment)
    {
        case EdgeAlignment::Start:
            center = _margin + barLength * 0.5f;
        break;
        case EdgeAlignment::Center:
            center = edgeLength * 0.5f;
        break;
        case EdgeAlignment::End:
            center = edgeLength - _margin - barLength * 0.5f;
        break;
        default: [[unlikely]]
        break;
    }

    return center;
}

glm::vec3 RetroFuturaGUI::MenuBar::calculateDockedPosition() const
{
    const glm::vec2 resolution { _projection.GetResolution() };
    f32
        x { resolution.x * 0.5f },
        y { resolution.y * 0.5f };

    switch(_dockingEdge)
    {
        case DockingEdge::Left:
            x = _margin + _size.x * 0.5f;
            y = alignAlongEdge(resolution.y, _size.y);
        break;
        case DockingEdge::Right:
            x = resolution.x - _margin - _size.x * 0.5f;
            y = alignAlongEdge(resolution.y, _size.y);
        break;
        case DockingEdge::Top:
            x = alignAlongEdge(resolution.x, _size.x);
            y = resolution.y - _margin - _size.y * 0.5f;
        break;
        case DockingEdge::Bottom:
            x = alignAlongEdge(resolution.x, _size.x);
            y = _margin + _size.y * 0.5f;
        break;
        default:
            [[unlikely]]
        break;
    }

    return glm::vec3(x, y, _position.z);
}

bool RetroFuturaGUI::MenuBar::isHorizontal() const
{
    return _dockingEdge == DockingEdge::Top || _dockingEdge == DockingEdge::Bottom;
}

void RetroFuturaGUI::MenuBar::resolveCellSizes()
{
    _resolvedCellSizes.assign(_itemFlex.size(), 0.0f);

    if(_itemFlex.empty())
        return;

    const bool horizontal { isHorizontal() };
    const f32
        lengthExtent { horizontal ? _size.x : _size.y },   // along the bar
        thicknessExtent { horizontal ? _size.y : _size.x };// across it

    f32
        sizedTotal { 0.0f },
        starWeightTotal { 0.0f };
    uSize index { 0 };

    //First pass: every cell that knows its own extent claims it, and the Star weights are counted up.
    for(const std::unique_ptr<IWidget>& item : _itemFlex)
    {
        const FlexDefinition& definition { GetFlexDefinition(index) };
        const f32 value { definition._Width > 0.0f ? definition._Width : 0.0f };

        switch(definition._FlexSizing)
        {
            case FlexSizing::Star:
                starWeightTotal += value;
            break;
            case FlexSizing::Fixed:
                _resolvedCellSizes[index] = value;
            break;
            case FlexSizing::Auto:
                _resolvedCellSizes[index] = item ? (horizontal ? item->GetSize().x : item->GetSize().y) : 0.0f;
            break;
            case FlexSizing::Square:
                _resolvedCellSizes[index] = thicknessExtent;
            break;
            default:
                [[unlikely]]
            break;
        }

        sizedTotal += _resolvedCellSizes[index];
        ++index;
    }

    const f32 leftover { lengthExtent > sizedTotal ? lengthExtent - sizedTotal : 0.0f };

    for(index = 0; index < _resolvedCellSizes.size(); ++index)
    {
        const FlexDefinition& definition { GetFlexDefinition(index) };

        if(definition._FlexSizing != FlexSizing::Star)
            continue;

        const f32 value { definition._Width > 0.0f ? definition._Width : 0.0f };
        _resolvedCellSizes[index] = starWeightTotal > 0.0f ? leftover * (value / starWeightTotal) : 0.0f;
    }
}

glm::vec2 RetroFuturaGUI::MenuBar::cellSize(const uSize index) const
{
    if(index >= _resolvedCellSizes.size())
        return glm::vec2(0.0f);

    if(isHorizontal())
        return glm::vec2(_resolvedCellSizes[index], _size.y);

    return glm::vec2(_size.x, _resolvedCellSizes[index]);
}

f32 RetroFuturaGUI::MenuBar::cellOffset(const uSize index) const
{
    f32 offset { 0.0f };

    for(uSize cell { 0 }; cell < index && cell < _resolvedCellSizes.size(); ++cell)
        offset += _resolvedCellSizes[cell];

    return offset;
}

glm::vec3 RetroFuturaGUI::MenuBar::cellPosition(const uSize index) const
{
    if(index >= _resolvedCellSizes.size())
        return _position;

    const f32
        offset { cellOffset(index) },
        extent { _resolvedCellSizes[index] };

    if(isHorizontal())
        return glm::vec3(_position.x - _size.x * 0.5f + offset + extent * 0.5f, _position.y, _position.z);

    return glm::vec3(_position.x, _position.y + _size.y * 0.5f - offset - extent * 0.5f, _position.z);
}

RetroFuturaGUI::ColorState RetroFuturaGUI::MenuBar::itemState(const uSize index) const
{
    if(!_isEnabledFlag)
        return ColorState::Disabled;

    if(index != _hoveredItemIndex)
        return ColorState::Enabled;

    if(_wasClicked)
        return ColorState::Clicked;

    return ColorState::Hover;
}

void RetroFuturaGUI::MenuBar::drawItems()
{
    uSize index { 0 };

    for(const std::unique_ptr<IWidget>& item : _itemFlex)
    {
        const glm::vec3 center { cellPosition(index) };
        const glm::vec2 cell { cellSize(index) };
        setItemColors(itemState(index));
        ++index;

        if(_itemBackground)
        {
            _itemBackground->SetSize(cell);
            _itemBackground->SetPosition(center);
            _itemBackground->SetRotation(_rotation);
            _itemBackground->Draw();
        }

        if(_itemBorder)
        {
            _itemBorder->SetSize(cell);
            _itemBorder->SetPosition(center + glm::vec3(0.0f, 0.0f, 0.01f));
            _itemBorder->SetRotation(_rotation);
            _itemBorder->Draw();
        }

        if(!item)
            continue;

        item->Draw();
    }
}

void RetroFuturaGUI::MenuBar::drawSeparatorLines()
{
    if(!_useSeparatorLines)
        return;

    if(!_separatorLine)
        return;

    if(_itemFlex.size() < 2)
        return;

    const glm::vec2 lineSize { isHorizontal()
        ? glm::vec2(_separatorLinesThickness, _size.y)
        : glm::vec2(_size.x, _separatorLinesThickness) };

    _separatorLine->SetSize(lineSize);
    _separatorLine->SetRotation(_rotation);

    // A line goes on every seam between two cells. one fewer than there are cells
    for(uSize seam { 1 }; seam < _itemFlex.size(); ++seam)
    {
        //The seam is the near edge of this cell, which is where the previous one ended.
        const f32 offset { cellOffset(seam) };
        const glm::vec3 seamCenter { isHorizontal()
            ? glm::vec3(_position.x - _size.x * 0.5f + offset, _position.y, _position.z)
            : glm::vec3(_position.x, _position.y + _size.y * 0.5f - offset, _position.z) };

        _separatorLine->SetPosition(seamCenter + glm::vec3(0.0f, 0.0f, 0.015f));
        _separatorLine->Draw();
    }
}

void RetroFuturaGUI::MenuBar::setItemColors(const ColorState state)
{
    _itemBackgroundColorState = state;
    _itemBorderColorState = state;
    setItemBackgroundColors();
    setItemBorderColors();
}

void RetroFuturaGUI::MenuBar::setSeparatorLinesColors()
{
    if(!_separatorLine)
        return;

    std::vector<glm::vec4>* colors { nullptr };

    switch(_separatorLinesColorState)
    {
        case ColorState::Enabled:
            colors = &_separatorLinesColorsEnabled;
        break;
        case ColorState::Disabled:
            colors = &_separatorLinesColorsDisabled;
        break;
        default:
            [[unlikely]]
        break;
    }

    pushElementColors(_separatorLine.get(), colors);
}

void RetroFuturaGUI::MenuBar::setItemBackgroundColors()
{
    if(!_itemBackground)
        return;

    std::vector<glm::vec4>* colors { nullptr };

    switch(_itemBackgroundColorState)
    {
        case ColorState::Enabled:
            colors = &_itemBackgroundColorsEnabled;
        break;
        case ColorState::Disabled:
            colors = &_itemBackgroundColorsDisabled;
        break;
        case ColorState::Clicked:
            colors = &_itemBackgroundColorsClick;
        break;
        case ColorState::Hover:
            colors = &_itemBackgroundColorsHover;
        break;
        default:
            [[unlikely]]
        break;
    }

    pushElementColors(_itemBackground.get(), colors);
}

void RetroFuturaGUI::MenuBar::setItemBorderColors()
{
    if(!_itemBorder)
        return;

    std::vector<glm::vec4>* colors { nullptr };

    switch(_itemBorderColorState)
    {
        case ColorState::Enabled:
            colors = &_itemBorderColorsEnabled;
        break;
        case ColorState::Disabled:
            colors = &_itemBorderColorsDisabled;
        break;
        case ColorState::Clicked:
            colors = &_itemBorderColorsClick;
        break;
        case ColorState::Hover:
            colors = &_itemBorderColorsHover;
        break;
        default:
            [[unlikely]]
        break;
    }

    pushElementColors(_itemBorder.get(), colors);
}

void RetroFuturaGUI::MenuBar::pushElementColors(Rectangle* element, std::vector<glm::vec4>* colors)
{
    if(!element)
        return;

    if(!colors)
        return;

    if(colors->empty())
        return;

    element->SetColors(*colors);
}

void RetroFuturaGUI::MenuBar::setItemBackgroundCornerRadii(const glm::vec4& radii)
{
    if(!_itemBackground)
        return;

    _itemBackground->SetCornerRadii(radii);
}

void RetroFuturaGUI::MenuBar::setItemBorderCornerRadii(const glm::vec4& radii)
{
    if(!_itemBorder)
        return;

    _itemBorder->SetCornerRadii(radii);
}
