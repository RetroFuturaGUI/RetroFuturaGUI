#include "Table.hpp"

RetroFuturaGUI::Table::Table(const std::string& name, Projection* projection, IWidget* parentWidget, const WidgetTypeID parentWidgetTypeID, GLFWwindow* parentWindow)
    : IWidget(name, projection, parentWidget, parentWidgetTypeID, parentWindow)
{
    _widgetTypeID = WidgetTypeID::Table;
    _background = std::make_unique<Rectangle>(projection);

    if(_background)
        _background->SetRectangleMode(RectangleMode::Plane);

    _border = std::make_unique<Rectangle>(projection);

    if(_border)
        _border->SetRectangleMode(RectangleMode::Border);

    _highlightedBackgroundCell = std::make_unique<Rectangle>(projection);

    if(_highlightedBackgroundCell)
        _highlightedBackgroundCell->SetRectangleMode(RectangleMode::Plane);

    _highlightedCellBorder = std::make_unique<Rectangle>(projection);

    if(_highlightedCellBorder)
        _highlightedCellBorder->SetRectangleMode(RectangleMode::Border);

    _trackColoringOverlay = std::make_unique<Rectangle>(projection);

    if(_trackColoringOverlay)
        _trackColoringOverlay->SetRectangleMode(RectangleMode::Plane);
    
    _innerBorder = std::make_unique<Rectangle>(projection);

    if(_innerBorder)
        _innerBorder->SetRectangleMode(RectangleMode::Border);
}

void RetroFuturaGUI::Table::Draw()
{
    drawBorder();

    if(_nthTrackColors.empty())
        return;

    const uSize variantCount { _nthTrackColors.size() };

    // clamp contents to viewport
    const glm::vec2 contentOrigin { contentTopLeft() };
    const ScissorState contentScissor { pushScissor(contentOrigin.x, contentOrigin.y - _innerSize.y, _innerSize.x, _innerSize.y) };

    // only draw cells overlapping the viewport
    const uSize rowEnd { _displayedRows[1] < _tableCells.size() ? _displayedRows[1] : _tableCells.size() };

    for(uSize row = _displayedRows[0]; row < rowEnd; ++row)
    {
        const uSize columnEnd { _displayedColumns[1] < _tableCells[row].size() ? _displayedColumns[1] : _tableCells[row].size() };

        for(uSize column = _displayedColumns[0]; column < columnEnd; ++column)
        {
            const TableCell& cell { _tableCells[row][column] };

            if(!cell._TableWidget)
                continue;

            const uSize index { (_tableOrientation == TableOrientation::Row ? row : column) % variantCount };
            TrackColoring& coloring { _nthTrackColors[index] };

            if(_trackColoringOverlay)
            {
                _trackColoringOverlay->SetSize(cell._SizePixels);
                _trackColoringOverlay->SetPosition(cell._PositionPixels - glm::vec3(0.0f, 0.0f, _widgetZOffset));
                _trackColoringOverlay->SetColors(coloring._BackgroundColorEnabled);
                _trackColoringOverlay->Draw();
            }

            if(_innerBorder)
            {
                _innerBorder->SetSize(cell._SizePixels);
                _innerBorder->SetPosition(cell._PositionPixels - glm::vec3(0.0f, 0.0f, _widgetZOffset - 0.01f));
                _innerBorder->SetColors(coloring._InnerBorderColorEnabled);
                _innerBorder->Draw();
            }

            if(cell._TableWidgetTypeID == ITableWidget::TableWidgetTypeID::TableText)
                static_cast<TableText*>(cell._TableWidget.get())->SetTextColors(coloring._TextColorEnabled, ColorState::Enabled);

            cell._TableWidget->Draw();
        }
    }

    popScissor(contentScissor);

    const f32
        tableLeft { _position.x - _size.x * 0.5f },
        tableBottom { _position.y - _size.y * 0.5f };

    if(_displayHorizontalHeader)
    {
        const uSize columnEnd { _horizontalHeaderCells.size() < _displayedColumns[1] ? _horizontalHeaderCells.size() : _displayedColumns[1] };
        const ScissorState headerScissor { pushScissor(contentOrigin.x, tableBottom, _innerSize.x, _size.y) };
        drawHeaderBand(_horizontalHeaderCells, _displayedColumns[0], columnEnd);
        popScissor(headerScissor);
    }

    if(_displayVerticalHeader)
    {
        const uSize rowEnd2 { _verticalHeaderCells.size() < _displayedRows[1] ? _verticalHeaderCells.size() : _displayedRows[1] };
        const ScissorState headerScissor { pushScissor(tableLeft, contentOrigin.y - _innerSize.y, _size.x, _innerSize.y) };
        drawHeaderBand(_verticalHeaderCells, _displayedRows[0], rowEnd2);
        popScissor(headerScissor);
    }
}

void RetroFuturaGUI::Table::drawHeaderBand(std::vector<TableCell>& cells, const uSize first, const uSize end)
{
    for(uSize index = first; index < end && index < cells.size(); ++index)
    {
        TableCell& cell { cells[index] };

        if(!cell._TableWidget)
            continue;

        if(_trackColoringOverlay)
        {
            _trackColoringOverlay->SetSize(cell._SizePixels);
            _trackColoringOverlay->SetPosition(cell._PositionPixels - glm::vec3(0.0f, 0.0f, _widgetZOffset));
            _trackColoringOverlay->SetColors(_headerBackgroundColorsEnabled);
            _trackColoringOverlay->Draw();
        }

        if(_innerBorder)
        {
            _innerBorder->SetBorderWidth(_headerInnerBorderWidth);
            _innerBorder->SetSize(cell._SizePixels);
            _innerBorder->SetPosition(cell._PositionPixels - glm::vec3(0.0f, 0.0f, _widgetZOffset - 0.01f));
            _innerBorder->SetColors(_headerBorderColorsEnabled);
            _innerBorder->Draw();
        }

        if(cell._TableWidgetTypeID == ITableWidget::TableWidgetTypeID::TableText)
            static_cast<TableText*>(cell._TableWidget.get())->SetTextColors(_headerTextColorsEnabled, ColorState::Enabled);

        cell._TableWidget->Draw();
    }
}

RetroFuturaGUI::Table::ScissorState RetroFuturaGUI::Table::pushScissor(const f32 left, const f32 bottom, const f32 width, const f32 height)
{
    // axis-aligned scissor box. breaks when rotation is applied
    ScissorState state {};
    state._WasEnabled = static_cast<bool>(glIsEnabled(GL_SCISSOR_TEST));
    glGetIntegerv(GL_SCISSOR_BOX, state._Previous);

    i32
        clipLeft { static_cast<i32>(left) },
        clipBottom { static_cast<i32>(bottom) },
        clipRight { clipLeft + static_cast<i32>(width) },
        clipTop { clipBottom + static_cast<i32>(height) };

    if(state._WasEnabled)
    {
        const i32
            previousRight { state._Previous[0] + state._Previous[2] },
            previousTop { state._Previous[1] + state._Previous[3] };
        clipLeft = clipLeft > state._Previous[0] ? clipLeft : state._Previous[0];
        clipBottom = clipBottom > state._Previous[1] ? clipBottom : state._Previous[1];
        clipRight = clipRight < previousRight ? clipRight : previousRight;
        clipTop = clipTop < previousTop ? clipTop : previousTop;
    }

    glEnable(GL_SCISSOR_TEST);
    glScissor(clipLeft, clipBottom,
              clipRight > clipLeft ? clipRight - clipLeft : 0,
              clipTop > clipBottom ? clipTop - clipBottom : 0);
    return state;
}

void RetroFuturaGUI::Table::popScissor(const ScissorState& state)
{
    // Scissor state is global, so hand it back exactly as it was found
    if(state._WasEnabled)
        glScissor(state._Previous[0], state._Previous[1], state._Previous[2], state._Previous[3]);
    else
        glDisable(GL_SCISSOR_TEST);
}

void RetroFuturaGUI::Table::SetSize(const glm::vec3& size)
{
    IWidget::SetSize(size);

    if(_background)
        _background->SetSize(size);

    if(_border)
        _border->SetSize(size);

    layoutCells();
}

void RetroFuturaGUI::Table::SetPosition(const glm::vec3& position)
{
    IWidget::SetPosition(position);

    if(_background)
        _background->SetPosition(position);

    if(_border)
        _border->SetPosition(position + glm::vec3(0.0f, 0.0f, 0.1f));

    layoutCells();
}

void RetroFuturaGUI::Table::SetRotation(const glm::vec3& rotation)
{
    _rotation = rotation;

    if(_background)
        _background->SetRotation(rotation);

    if(_border)
        _border->SetRotation(rotation);
}

void RetroFuturaGUI::Table::SetTrackDefinitions(const std::vector<f32>& rowDefinition, const std::vector<f32>& columnDefinition)
{
    std::vector<TrackDefinition>
        rowTracks {},
        columnTracks {};
    rowTracks.reserve(rowDefinition.size());
    columnTracks.reserve(columnDefinition.size());

    for(const f32 weight : rowDefinition)
        rowTracks.push_back({ TrackSizing::Star, weight });

    for(const f32 weight : columnDefinition)
        columnTracks.push_back({ TrackSizing::Star, weight });

    SetTrackDefinitions(rowTracks, columnTracks);
}

void RetroFuturaGUI::Table::SetTrackDefinitions(const std::vector<TrackDefinition>& rowDefinition, const std::vector<TrackDefinition>& columnDefinition)
{
    _rowDefinition = rowDefinition;
    _columnDefinition = columnDefinition;
    _tableCells.resize(_rowDefinition.size());

    for(auto& row : _tableCells)
        row.resize(_columnDefinition.size());

    resizeHeaders();
    layoutCells();
}

void RetroFuturaGUI::Table::SetScrollPosition(const glm::vec2& scrollPosition)
{
    _scrollPosition = scrollPosition;
    layoutCells(); // clamps the new position against the current content extent
}

void RetroFuturaGUI::Table::SetHorizontalScrollPosition(const f32 scrollPosition)
{
    SetScrollPosition(glm::vec2(scrollPosition, _scrollPosition.y));
}

void RetroFuturaGUI::Table::SetVerticalScrollPosition(const f32 scrollPosition)
{
    SetScrollPosition(glm::vec2(_scrollPosition.x, scrollPosition));
}

glm::vec2 RetroFuturaGUI::Table::GetScrollPosition() const
{
    return _scrollPosition;
}

f32 RetroFuturaGUI::Table::GetHorizontalScrollPosition() const
{
    return _scrollPosition.x;
}

f32 RetroFuturaGUI::Table::GetVerticalScrollPosition() const
{
    return _scrollPosition.y;
}

glm::vec2 RetroFuturaGUI::Table::GetMaxScroll() const
{
    // Measured against the viewport left over after the headers, not the table's full size
    return glm::vec2(_contentExtent.x > _innerSize.x ? _contentExtent.x - _innerSize.x : 0.0f,
                     _contentExtent.y > _innerSize.y ? _contentExtent.y - _innerSize.y : 0.0f);
}

glm::vec2 RetroFuturaGUI::Table::headerExtents() const
{
    return glm::vec2(_displayVerticalHeader ? _verticalHeaderWidth : 0.0f,
                     _displayHorizontalHeader ? _horizontalHeaderHeight : 0.0f);
}

glm::vec2 RetroFuturaGUI::Table::contentTopLeft() const
{
    const glm::vec2 headers { headerExtents() };

    // Only a header sitting on that edge pushes the content inwards; Bottom/Right ones take their space off the far side
    return glm::vec2(_position.x - _size.x * 0.5f + (_verticalHeaderPosition == HeaderPosition::Left ? headers.x : 0.0f),
                     _position.y + _size.y * 0.5f - (_horizontalHeaderPosition == HeaderPosition::Top ? headers.y : 0.0f));
}

void RetroFuturaGUI::Table::resolveTrackSizes(const std::vector<TrackDefinition>& tracks, const f32 viewportExtent, std::vector<f32>& outSizes)
{
    outSizes.assign(tracks.size(), 0.0f);

    f32
        fixedTotal { 0.0f },
        starWeightTotal { 0.0f };

    for(const TrackDefinition& track : tracks)
    {
        const f32 value { track._Value > 0.0f ? track._Value : 0.0f };

        if(track._Sizing == TrackSizing::Star)
            starWeightTotal += value;
        else
            fixedTotal += value; // Auto resolves like Fixed until content measurement lands
    }

    // Star tracks only divide what the fixed ones left behind, so they can never push the content past the viewport
    const f32 leftover { viewportExtent > fixedTotal ? viewportExtent - fixedTotal : 0.0f };

    for(uSize track = 0; track < tracks.size(); ++track)
    {
        const f32 value { tracks[track]._Value > 0.0f ? tracks[track]._Value : 0.0f };

        if(tracks[track]._Sizing != TrackSizing::Star)
        {
            outSizes[track] = value;
            continue;
        }

        outSizes[track] = starWeightTotal > 0.0f ? leftover * (value / starWeightTotal) : 0.0f;
    }
}

void RetroFuturaGUI::Table::resolveVisibleRange(const std::vector<f32>& sizes, const f32 scroll, const f32 viewportExtent, uSize& outFirst, uSize& outEnd)
{
    outFirst = 0;
    outEnd = 0;
    bool foundFirst { false };
    f32 offset { 0.0f };

    for(uSize track = 0; track < sizes.size(); ++track)
    {
        // Visible while the track's band in content space overlaps the scrolled viewport's band
        if(offset + sizes[track] > scroll && offset < scroll + viewportExtent)
        {
            if(!foundFirst)
            {
                outFirst = track;
                foundFirst = true;
            }

            outEnd = track + 1;
        }

        offset += sizes[track];
    }
}

void RetroFuturaGUI::Table::layoutCells()
{
    if(_rowDefinition.empty() || _columnDefinition.empty())
        return;

    const glm::vec2 headers { headerExtents() };
    _innerSize = glm::vec3(_size.x > headers.x ? _size.x - headers.x : 0.0f,
                           _size.y > headers.y ? _size.y - headers.y : 0.0f,
                           _size.z);

    // size every track on its own
    resolveTrackSizes(_rowDefinition, _innerSize.y, _resolvedRowSizes);
    resolveTrackSizes(_columnDefinition, _innerSize.x, _resolvedColumnSizes);

    _contentExtent = glm::vec2(0.0f);

    for(const f32 width : _resolvedColumnSizes)
        _contentExtent.x += width;

    for(const f32 height : _resolvedRowSizes)
        _contentExtent.y += height;

    const glm::vec2 maxScroll { GetMaxScroll() };
    _scrollPosition.x = _scrollPosition.x < 0.0f ? 0.0f : (_scrollPosition.x > maxScroll.x ? maxScroll.x : _scrollPosition.x);
    _scrollPosition.y = _scrollPosition.y < 0.0f ? 0.0f : (_scrollPosition.y > maxScroll.y ? maxScroll.y : _scrollPosition.y);

    resolveVisibleRange(_resolvedRowSizes, _scrollPosition.y, _innerSize.y, _displayedRows[0], _displayedRows[1]);
    resolveVisibleRange(_resolvedColumnSizes, _scrollPosition.x, _innerSize.x, _displayedColumns[0], _displayedColumns[1]);

    // place the cells at their track offsets
    const glm::vec2 contentOrigin { contentTopLeft() };
    const f32
        originX { contentOrigin.x - _scrollPosition.x },
        originY { contentOrigin.y + _scrollPosition.y };
    f32 accumY { 0.0f };

    for(uSize row = 0; row < _resolvedRowSizes.size() && row < _tableCells.size(); ++row)
    {
        const f32 cellSizeY { _resolvedRowSizes[row] };
        f32 accumX { 0.0f };

        for(uSize column = 0; column < _resolvedColumnSizes.size() && column < _tableCells[row].size(); ++column)
        {
            const f32 cellSizeX { _resolvedColumnSizes[column] };
            TableCell& cell { _tableCells[row][column] };

            cell._SizeNormalized = glm::vec3(_contentExtent.x > 0.0f ? cellSizeX / _contentExtent.x : 0.0f,
                                             _contentExtent.y > 0.0f ? cellSizeY / _contentExtent.y : 0.0f,
                                             1.0f);
            cell._SizePixels = glm::vec3(cellSizeX, cellSizeY, _size.z);
            cell._PositionPixels = glm::vec3(originX + accumX + cellSizeX * 0.5f,
                                             originY - accumY - cellSizeY * 0.5f,
                                             _position.z + _widgetZOffset);

            if(cell._TableWidget)
            {
                cell._TableWidget->SetSize(cell._SizePixels);
                cell._TableWidget->SetPosition(cell._PositionPixels);
            }

            accumX += cellSizeX;
        }

        accumY += cellSizeY;
    }

    layoutHeaders();
}

void RetroFuturaGUI::Table::layoutHeaders()
{
    const glm::vec2
        headers { headerExtents() },
        contentOrigin { contentTopLeft() };
    const f32
        tableLeft { _position.x - _size.x * 0.5f },
        tableRight { tableLeft + _size.x },
        tableBottom { _position.y - _size.y * 0.5f },
        tableTop { tableBottom + _size.y };

    if(_displayHorizontalHeader)
    {
        const f32 bandCentreY { _horizontalHeaderPosition == HeaderPosition::Bottom
            ? tableBottom + headers.y * 0.5f
            : tableTop - headers.y * 0.5f };
        f32 accumX { 0.0f };

        for(uSize column = 0; column < _horizontalHeaderCells.size() && column < _resolvedColumnSizes.size(); ++column)
        {
            const f32 cellSizeX { _resolvedColumnSizes[column] };
            TableCell& cell { _horizontalHeaderCells[column] };

            cell._SizePixels = glm::vec3(cellSizeX, headers.y, _size.z);
            cell._PositionPixels = glm::vec3(contentOrigin.x - _scrollPosition.x + accumX + cellSizeX * 0.5f,
                                             bandCentreY,
                                             _position.z + _widgetZOffset);

            if(cell._TableWidget)
            {
                cell._TableWidget->SetSize(cell._SizePixels);
                cell._TableWidget->SetPosition(cell._PositionPixels);
            }

            accumX += cellSizeX;
        }
    }

    if(_displayVerticalHeader)
    {
        const f32 bandCentreX { _verticalHeaderPosition == HeaderPosition::Right
            ? tableRight - headers.x * 0.5f
            : tableLeft + headers.x * 0.5f };
        f32 accumY { 0.0f };

        for(uSize row = 0; row < _verticalHeaderCells.size() && row < _resolvedRowSizes.size(); ++row)
        {
            const f32 cellSizeY { _resolvedRowSizes[row] };
            TableCell& cell { _verticalHeaderCells[row] };

            cell._SizePixels = glm::vec3(headers.x, cellSizeY, _size.z);
            cell._PositionPixels = glm::vec3(bandCentreX,
                                             contentOrigin.y + _scrollPosition.y - accumY - cellSizeY * 0.5f,
                                             _position.z + _widgetZOffset);

            if(cell._TableWidget)
            {
                cell._TableWidget->SetSize(cell._SizePixels);
                cell._TableWidget->SetPosition(cell._PositionPixels);
            }

            accumY += cellSizeY;
        }
    }
}

void RetroFuturaGUI::Table::ensureHeaderWidget(TableCell& cell)
{
    if(cell._TableWidget)
        return;

    auto textWidget { std::make_unique<TableText>(this, &_projection) };

    if(_headerTextDefaults._HasFont)
        textWidget->SetFontFamily(_headerTextDefaults._FontFamily, _headerTextDefaults._FontSize, _headerTextDefaults._Slant, _headerTextDefaults._Weight);

    textWidget->SetTextAlignment(_headerTextDefaults._Alignment);
    textWidget->SetTextPadding(_headerTextDefaults._Padding);
    textWidget->SetTextColors(_headerTextColorsEnabled, ColorState::Enabled);
    textWidget->SetTextColors(_headerTextColorsDisabled, ColorState::Disabled);

    cell._TableWidget = std::move(textWidget);
    cell._TableWidgetTypeID = ITableWidget::TableWidgetTypeID::TableText;
    cell._ParentTable = this;
}

void RetroFuturaGUI::Table::resizeHeaders()
{
    _horizontalHeaderCells.resize(_columnDefinition.size());
    _verticalHeaderCells.resize(_rowDefinition.size());
}

void RetroFuturaGUI::Table::ShowHorizontalHeader(const bool show)
{
    _displayHorizontalHeader = show;
    layoutCells(); // the band changes the viewport, so everything downstream has to be re-resolved
}

void RetroFuturaGUI::Table::ShowVerticalHeader(const bool show)
{
    _displayVerticalHeader = show;
    layoutCells();
}

void RetroFuturaGUI::Table::SetHorizontalHeaderPosition(const HeaderPosition position)
{
    if(position == HeaderPosition::Left || position == HeaderPosition::Right)
        return;

    if(position == HeaderPosition::None)
    {
        ShowHorizontalHeader(false);
        return;
    }

    _horizontalHeaderPosition = position;
    layoutCells();
}

void RetroFuturaGUI::Table::SetVerticalHeaderPosition(const HeaderPosition position)
{
    if(position == HeaderPosition::Top || position == HeaderPosition::Bottom)
        return;

    if(position == HeaderPosition::None)
    {
        ShowVerticalHeader(false);
        return;
    }

    _verticalHeaderPosition = position;
    layoutCells();
}

void RetroFuturaGUI::Table::SetHorizontalHeaderTexts(const std::vector<std::string>& texts)
{
    for(uSize column = 0; column < texts.size(); ++column)
        SetHorizontalHeaderText(texts[column], column);
}

void RetroFuturaGUI::Table::SetHorizontalHeaderText(std::string_view text, const uSize columnIndex)
{
    if(columnIndex >= _horizontalHeaderCells.size())
        return;

    TableCell& cell { _horizontalHeaderCells[columnIndex] };
    ensureHeaderWidget(cell);
    layoutHeaders();
    static_cast<TableText*>(cell._TableWidget.get())->SetText(text);
}

void RetroFuturaGUI::Table::SetVerticalHeaderTexts(const std::vector<std::string>& texts)
{
    for(uSize row = 0; row < texts.size(); ++row)
        SetVerticalHeaderText(texts[row], row);
}

void RetroFuturaGUI::Table::SetVerticalHeaderText(std::string_view text, const uSize rowIndex)
{
    if(rowIndex >= _verticalHeaderCells.size())
        return;

    TableCell& cell { _verticalHeaderCells[rowIndex] };
    ensureHeaderWidget(cell);
    layoutHeaders();
    static_cast<TableText*>(cell._TableWidget.get())->SetText(text);
}

void RetroFuturaGUI::Table::SetHeaderFontFamily(std::string_view fontFamily, const f32 fontSize, const PlatformBridge::Fonts::Slant slant, const PlatformBridge::Fonts::Weight fontWeight)
{
    _headerTextDefaults._HasFont = true;
    _headerTextDefaults._FontFamily = fontFamily;
    _headerTextDefaults._FontSize = fontSize;
    _headerTextDefaults._Slant = slant;
    _headerTextDefaults._Weight = fontWeight;

    for(std::vector<TableCell>* band : { &_horizontalHeaderCells, &_verticalHeaderCells })
        for(TableCell& cell : *band)
            if(cell._TableWidgetTypeID == ITableWidget::TableWidgetTypeID::TableText)
                static_cast<TableText*>(cell._TableWidget.get())->SetFontFamily(fontFamily, fontSize, slant, fontWeight);
}

void RetroFuturaGUI::Table::SetHeaderTextAlignment(const TextAlignment alignment)
{
    _headerTextDefaults._Alignment = alignment;

    for(std::vector<TableCell>* band : { &_horizontalHeaderCells, &_verticalHeaderCells })
        for(TableCell& cell : *band)
            if(cell._TableWidgetTypeID == ITableWidget::TableWidgetTypeID::TableText)
                static_cast<TableText*>(cell._TableWidget.get())->SetTextAlignment(alignment);
}

void RetroFuturaGUI::Table::SetHeaderTextPadding(const f32 padding)
{
    _headerTextDefaults._Padding = padding;

    for(std::vector<TableCell>* band : { &_horizontalHeaderCells, &_verticalHeaderCells })
        for(TableCell& cell : *band)
            if(cell._TableWidgetTypeID == ITableWidget::TableWidgetTypeID::TableText)
                static_cast<TableText*>(cell._TableWidget.get())->SetTextPadding(padding);
}

void RetroFuturaGUI::Table::SetHeaderTextColors(std::span<glm::vec4> colors, const ColorState state)
{
    std::vector<glm::vec4>& target { state == ColorState::Disabled ? _headerTextColorsDisabled : _headerTextColorsEnabled };
    target.assign(colors.begin(), colors.end());

    for(std::vector<TableCell>* band : { &_horizontalHeaderCells, &_verticalHeaderCells })
        for(TableCell& cell : *band)
            if(cell._TableWidgetTypeID == ITableWidget::TableWidgetTypeID::TableText)
                static_cast<TableText*>(cell._TableWidget.get())->SetTextColors(target, state);
}

void RetroFuturaGUI::Table::SetHeaderInnerBorderWidth(const f32 width)
{
    _headerInnerBorderWidth = width;
}

void RetroFuturaGUI::Table::SetHeaderOuterBorderWidth(const f32 width)
{
    _headerOuterBorderWidth = width;
}

void RetroFuturaGUI::Table::SetHeaderInnerBorderColors(std::span<glm::vec4> colors, const ColorState state)
{
    std::vector<glm::vec4>& target { state == ColorState::Disabled ? _headerBorderColorsDisabled : _headerBorderColorsEnabled };
    target.assign(colors.begin(), colors.end());
}

void RetroFuturaGUI::Table::SetHeaderOuterBorderColors(std::span<glm::vec4> colors, const ColorState state)
{
    std::vector<glm::vec4>& target { state == ColorState::Disabled ? _headerOuterBorderColorsDisabled : _headerOuterBorderColorsEnabled };
    target.assign(colors.begin(), colors.end());
}

void RetroFuturaGUI::Table::SetHorizontalHeaderSize(const f32 size)
{
    _horizontalHeaderHeight = size > 0.0f ? size : 0.0f;
    layoutCells();
}

void RetroFuturaGUI::Table::SetVerticalHeaderSize(const f32 size)
{
    _verticalHeaderWidth = size > 0.0f ? size : 0.0f;
    layoutCells();
}

void RetroFuturaGUI::Table::Connect_OnTextChange(const typename Signal<>::Slot& slot, const bool async)
{
    if(async)
        _onTextChangeAsync.Connect(slot);
    else
        _onTextChange.Connect(slot);
}

void RetroFuturaGUI::Table::Disconnect_OnTextChange(const typename Signal<>::Slot& slot)
{
    _onTextChange.Disconnect(slot);
    _onTextChangeAsync.Disconnect(slot);
}

void RetroFuturaGUI::Table::SetTextColors(std::span<glm::vec4> colors, const ColorState state, const uSize nthIndex)
{
    if(_nthTrackColors.empty())
        _nthTrackColors.resize(1);

    if(nthIndex >= _nthTrackColors.size())
        return;

    TrackColoring& coloring { _nthTrackColors[nthIndex] };
    std::vector<glm::vec4>* target { nullptr };

    switch(state)
    {
        case ColorState::Disabled:
            target = &coloring._TextColorDisabled;
        break;
        case ColorState::Clicked:
            target = &coloring._TextColorClicked;
        break;
        case ColorState::Hover:
            target = &coloring._TextColorHover;
        break;
        default: //Enabled
            target = &coloring._TextColorEnabled;
    }

    target->assign(colors.begin(), colors.end());

    for(auto& row : _tableCells)
        for(TableCell& cell : row)
        {
            if(cell._TableWidgetTypeID != ITableWidget::TableWidgetTypeID::TableText)
                continue;

            static_cast<TableText*>(cell._TableWidget.get())->SetTextColors(*target, state);
        }
}

std::vector<glm::vec4> RetroFuturaGUI::Table::GetTextColor(const ColorState state) const
{
    if(_nthTrackColors.empty())
        return {};

    const TrackColoring& coloring { _nthTrackColors[0] };

    switch(state)
    {
        case ColorState::Disabled:
            return coloring._TextColorDisabled;
        case ColorState::Clicked:
            return coloring._TextColorClicked;
        case ColorState::Hover:
            return coloring._TextColorHover;
        default: //Enabled
            return coloring._TextColorEnabled;
    }
}

const std::string& RetroFuturaGUI::Table::GetText(const uSize xIndex, const uSize yIndex) const
{
    static const std::string dummy {};

    if(_tableCells.size() <= xIndex)
        return dummy;

    if(_tableCells[xIndex].size() <= yIndex)
        return dummy;

    const TableCell& cell { _tableCells[xIndex][yIndex] };

    if(cell._TableWidgetTypeID != ITableWidget::TableWidgetTypeID::TableText)
        return dummy;

    return static_cast<TableText*>(cell._TableWidget.get())->GetText();
}

void RetroFuturaGUI::Table::SetFontFamily(std::string_view fontFamily, const f32 fontSize, const PlatformBridge::Fonts::Slant slant, const PlatformBridge::Fonts::Weight fontWeight)
{
    // Persisted as a table-wide default so cells created later (e.g. via SetText/SetWidget,
    // possibly called before any cells exist yet) pick it up too, not just cells that already exist.
    _textDefaults._HasFont = true;
    _textDefaults._FontFamily = fontFamily;
    _textDefaults._FontSize = fontSize;
    _textDefaults._Slant = slant;
    _textDefaults._Weight = fontWeight;

    for(auto& row : _tableCells)
        for(TableCell& cell : row)
        {
            if(cell._TableWidgetTypeID != ITableWidget::TableWidgetTypeID::TableText)
                continue;

            static_cast<TableText*>(cell._TableWidget.get())->SetFontFamily(fontFamily, fontSize, slant, fontWeight);
        }
}

void RetroFuturaGUI::Table::SetTextAlignment(const TextAlignment alignment)
{
    _textDefaults._Alignment = alignment;

    for(auto& row : _tableCells)
        for(TableCell& cell : row)
        {
            if(cell._TableWidgetTypeID != ITableWidget::TableWidgetTypeID::TableText)
                continue;

            static_cast<TableText*>(cell._TableWidget.get())->SetTextAlignment(alignment);
        }
}

void RetroFuturaGUI::Table::SetTextPadding(const f32 padding)
{
    _textDefaults._Padding = padding;

    for(auto& row : _tableCells)
        for(TableCell& cell : row)
        {
            if(cell._TableWidgetTypeID != ITableWidget::TableWidgetTypeID::TableText)
                continue;

            static_cast<TableText*>(cell._TableWidget.get())->SetTextPadding(padding);
        }
}

void RetroFuturaGUI::Table::SetTrackBackgroundColors(std::span<glm::vec4> colors, const ColorState state, const uSize nthIndex)
{
    if(_nthTrackColors.empty())
        return;

    if(nthIndex >= _nthTrackColors.size())
        return;

    TrackColoring& coloring { _nthTrackColors[nthIndex] };

    switch(state)
    {
        case ColorState::Disabled:
            coloring._BackgroundColorDisabled = std::vector<glm::vec4>(colors.begin(), colors.end());
            break;
        case ColorState::Clicked:
            coloring._BackgroundColorClicked = std::vector<glm::vec4>(colors.begin(), colors.end());
            break;
        case ColorState::Hover:
            coloring._BackgroundColorHover = std::vector<glm::vec4>(colors.begin(), colors.end());
            break;
        default: //Enabled
            coloring._BackgroundColorEnabled = std::vector<glm::vec4>(colors.begin(), colors.end());
            break;
    }
}

void RetroFuturaGUI::Table::SetTrackBorderColors(std::span<glm::vec4> colors, const ColorState state, const uSize nthIndex)
{
    if(_nthTrackColors.empty())
        return;

    if(nthIndex >= _nthTrackColors.size())
        return;

    TrackColoring& coloring { _nthTrackColors[nthIndex] };

    switch(state)
    {
        case ColorState::Disabled:
            coloring._InnerBorderColorDisabled = std::vector<glm::vec4>(colors.begin(), colors.end());
            break;
        case ColorState::Clicked:
            coloring._InnerBorderColorClicked = std::vector<glm::vec4>(colors.begin(), colors.end());
            break;
        case ColorState::Hover:
            coloring._InnerBorderColorHover = std::vector<glm::vec4>(colors.begin(), colors.end());
            break;
        default: //Enabled
            coloring._InnerBorderColorEnabled = std::vector<glm::vec4>(colors.begin(), colors.end());
            break;
    }
}

void RetroFuturaGUI::Table::SetInnerBorderWidth(const f32 width)
{
    if(_innerBorder)
        _innerBorder->SetBorderWidth(width);
}

void RetroFuturaGUI::Table::SetTableOrientation(const TableOrientation orientation)
{
    _tableOrientation = orientation;
}

void RetroFuturaGUI::Table::SetTrackAlternatingColorCount(const uSize variantCount)
{
    _nthTrackColors.resize(variantCount);
}

void RetroFuturaGUI::Table::SetRowWidgetTypes(const std::vector<ITableWidget::TableWidgetTypeID>& rowWidgetTypes)
{
    _rowWidgetTypes = rowWidgetTypes;
}

void RetroFuturaGUI::Table::SetHeaderBackgroundColors(std::span<glm::vec4> colors, const ColorState state)
{
    if(state == ColorState::Disabled)
        _headerBackgroundColorsDisabled = std::vector<glm::vec4>(colors.begin(), colors.end());
    else
        _headerBackgroundColorsEnabled = std::vector<glm::vec4>(colors.begin(), colors.end());
}