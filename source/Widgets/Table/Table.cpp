#include "Table.hpp"
#include "ITableWidget.hpp"
#include "IncludeHelper.hpp"
#include "PlatformBridge.hpp"
#include "SvgImage.hpp"
#include "SvgTexture.hpp"
#include "TableCheckBox.hpp"
#include "Window.hpp"
#include <algorithm>
#include <chrono>
#include <memory>

#if defined(TARGET_PLATFORM_LINUX)
    #define GLFW_EXPOSE_NATIVE_X11
#elif defined(TARGET_PLATFORM_WINDOWS)
    #define GLFW_EXPOSE_NATIVE_WIN32
#endif
#include <GLFW/glfw3native.h>

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

    initEditVisuals(projection);

    _cellColorPlane = std::make_unique<Rectangle>(projection);

    if(_cellColorPlane)
    {
        _cellColorPlane->SetRectangleMode(RectangleMode::Plane);
        _cellColorPlane->SetFillType(FillType::SOLID);
    }

    _checkBoxBackground = std::make_unique<Rectangle>(projection);

    if(_checkBoxBackground)
    {
        _checkBoxBackground->SetRectangleMode(RectangleMode::Plane);
        _checkBoxBackground->SetFillType(FillType::SOLID);
        _checkBoxBackground->SetColors(_checkBoxBackgroundColorEnabled);
    }

    _checkBoxBorder = std::make_unique<Rectangle>(projection);

    if(_checkBoxBorder)
    {
        _checkBoxBorder->SetRectangleMode(RectangleMode::Border);
        _checkBoxBorder->SetFillType(FillType::SOLID);
        _checkBoxBorder->SetColors(_checkBoxBorderColorEnabled);
    }

    _checkmark = ResourceManager::GetCheckmarkIcon();

    if(_checkmark)
        _checkmark->SetProjection(projection);

    _cellImagePlane = std::make_unique<Rectangle>(projection);

    if(_cellImagePlane)
    {
        _cellImagePlane->SetRectangleMode(RectangleMode::Plane);
        _cellImagePlane->SetFillType(FillType::SOLID);
        _cellImagePlane->SetColors(_imagePlaneColors);
    }

    _sharedColor = std::make_unique<TableColor>(this, projection, _cellColorPlane.get());
    _sharedImage = std::make_unique<TableImage>(this, projection, _cellImagePlane.get());

    if(_sharedImage)
        _sharedImage->SetPadding(_imagePadding);

    _sharedCheckBox = std::make_unique<TableCheckBox>(this, projection, _checkBoxBackground.get(), _checkBoxBorder.get(), _checkmark.get());

    if(_sharedCheckBox)
    {
        _sharedCheckBox->SetPadding(_checkBoxPadding);
        _sharedCheckBox->SetInnerPadding(_checkBoxInnerPadding);
    }

    _selectionBand = std::make_unique<Rectangle>(projection);

    if(_selectionBand)
    {
        _selectionBand->SetRectangleMode(RectangleMode::Plane);
        _selectionBand->SetFillType(FillType::SOLID);
    }

    _selectionBandBorder = std::make_unique<Rectangle>(projection);

    if(_selectionBandBorder)
    {
        _selectionBandBorder->SetRectangleMode(RectangleMode::Border);
        _selectionBandBorder->SetFillType(FillType::SOLID);
        _selectionBandBorder->SetBorderWidth(1.0f);
    }
}

void RetroFuturaGUI::Table::SetCheckBoxBackgroundColors(std::span<glm::vec4> colors, const ColorState state)
{
    switch(state)
    {
        case ColorState::Enabled:
            _checkBoxBackgroundColorEnabled.assign(colors.begin(), colors.end());
        break;
        case ColorState::Hover:
            _checkBoxBackgroundColorHover.assign(colors.begin(), colors.end());
        break;
        case ColorState::Clicked:
            _checkBoxBackgroundColorClicked.assign(colors.begin(), colors.end());
        break;
        default: //disabled
            _checkBoxBackgroundColorDisabled.assign(colors.begin(), colors.end());
    }
}

void RetroFuturaGUI::Table::SetCheckBoxBorderColors(std::span<glm::vec4> colors, const ColorState state)
{
    switch(state)
    {
        case ColorState::Enabled:
            _checkBoxBorderColorEnabled.assign(colors.begin(), colors.end());
        break;
        case ColorState::Hover:
            _checkBoxBorderColorHover.assign(colors.begin(), colors.end());
        break;
        case ColorState::Clicked:
            _checkBoxBorderColorClicked.assign(colors.begin(), colors.end());
        break;
        default: //disabled
            _checkBoxBorderColorDisabled.assign(colors.begin(), colors.end());
    }
}

void RetroFuturaGUI::Table::SetCheckBoxCheckmarkColors(std::span<glm::vec4> colors, const ColorState state)
{
    switch(state)
    {
        case ColorState::Enabled:
            _checkBoxCheckmarkColorEnabled.assign(colors.begin(), colors.end());
        break;
        case ColorState::Hover:
            _checkBoxCheckmarkColorHover.assign(colors.begin(), colors.end());
        break;
        case ColorState::Clicked:
            _checkBoxCheckmarkColorClicked.assign(colors.begin(), colors.end());
        break;
        default: //disabled
            _checkBoxCheckmarkColorDisabled.assign(colors.begin(), colors.end());
    }
}

void RetroFuturaGUI::Table::SetCheckBoxPadding(const f32 padding)
{
    _checkBoxPadding = padding;

    if(!_sharedCheckBox)
        return;

    _sharedCheckBox->SetPadding(padding); //each box is fitted to its cell right before it draws, so the new padding applies from the next frame
}

void RetroFuturaGUI::Table::SetCheckBoxInnerPadding(const f32 padding)
{
    _checkBoxInnerPadding = padding;

    if(!_sharedCheckBox)
        return;

    _sharedCheckBox->SetInnerPadding(padding);
}

void RetroFuturaGUI::Table::Draw()
{
    interact();
    drawBorder();

    if(_nthTrackColors.empty())
        return;

    const uSize variantCount { _nthTrackColors.size() };

    // clamp contents to viewport
    const glm::vec2 contentOrigin { contentTopLeft() };
    const ScissorState contentScissor { pushScissor(contentOrigin.x, contentOrigin.y - _innerSize.y, _innerSize.x, _innerSize.y) };

    // only draw cells overlapping the viewport
    const uSize
        rowEnd { _displayedRows[1] < _rowDefinition.size() ? _displayedRows[1] : _rowDefinition.size() },
        columnEnd { _displayedColumns[1] < _columnDefinition.size() ? _displayedColumns[1] : _columnDefinition.size() };

    for(uSize row = _displayedRows[0]; row < rowEnd; ++row)
    {
        for(uSize column = _displayedColumns[0]; column < columnEnd; ++column)
        {
            const BodyCell* cell { cellAt(row, column) };

            if(!cell)
                continue;

            if(ITableWidget::TableWidgetTypeID::None == cell->_Type)
                continue; //an empty cell draws nothing, its background neither

            glm::vec3
                position { 0.0f },
                size { 0.0f };
            cellRect(row, column, position, size);

            const uSize selectableRow { _tableOrientation == TableOrientation::Row ? row : column };
            TrackColoring& coloring { _nthTrackColors[selectableRow % variantCount] };
            const bool
                isSelected { selectableRow < _selectedRowFlags.size() && _selectedRowFlags[selectableRow] },
                isDropTarget { _hasDropTargetRow && _dropTargetRow == selectableRow };

            if(_trackColoringOverlay)
            {
                _trackColoringOverlay->SetSize(size);
                _trackColoringOverlay->SetPosition(position - glm::vec3(0.0f, 0.0f, _widgetZOffset));
                _trackColoringOverlay->SetColors(isDropTarget ? _dropTargetRowBackgroundColors : isSelected ? _selectedRowBackgroundColors : coloring._BackgroundColorEnabled);
                _trackColoringOverlay->Draw();
            }

            if(_innerBorder)
            {
                _innerBorder->SetBorderWidth(_innerBorderWidth);
                _innerBorder->SetSize(size);
                _innerBorder->SetPosition(position - glm::vec3(0.0f, 0.0f, _widgetZOffset - 0.01f));
                _innerBorder->SetColors(coloring._InnerBorderColorEnabled);
                _innerBorder->Draw();
            }

            switch(cell->_Type)
            {
                case ITableWidget::TableWidgetTypeID::TableText:
                    drawTextCell(row, column, isSelected ? _selectedRowTextColors : coloring._TextColorEnabled);
                break;
                case ITableWidget::TableWidgetTypeID::TableCheckBox:
                    drawCheckBoxCell(*cell, row, column, position, size);
                break;
                case ITableWidget::TableWidgetTypeID::TableColor:
                    drawColorCell(*cell, position, size);
                break;
                case ITableWidget::TableWidgetTypeID::TableImage:
                    drawImageCell(*cell, position, size);
                break;
                default:
                break;
            }
        }
    }

    drawSelectionBand();
    drawCaretAndSelection();

    popScissor(contentScissor);

    const f32
        tableLeft { _position.x - _size.x * 0.5f },
        tableBottom { _position.y - _size.y * 0.5f };

    if(_displayHorizontalHeader)
    {
        const uSize headerColumnEnd { _horizontalHeaderCells.size() < _displayedColumns[1] ? _horizontalHeaderCells.size() : _displayedColumns[1] };
        const ScissorState headerScissor { pushScissor(contentOrigin.x, tableBottom, _innerSize.x, _size.y) };
        drawHeaderBand(_horizontalHeaderCells, _displayedColumns[0], headerColumnEnd);
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

void RetroFuturaGUI::Table::drawTextCell(const uSize row, const uSize column, std::span<glm::vec4> textColors)
{
    TableText* text { bindText(row, column) };

    if(!text)
        return;

    text->SetTextColors(textColors, ColorState::Enabled);
    text->Draw();
}

void RetroFuturaGUI::Table::drawCheckBoxCell(const BodyCell& cell, const uSize row, const uSize column, const glm::vec3& position, const glm::vec3& size)
{
    if(!_sharedCheckBox)
        return;

    setCheckBoxColors(checkBoxColorState(row, column)); //the boxes share one background, border and checkmark, so each cell is colored just before it draws
    _sharedCheckBox->SetValue(cell._IsChecked);
    _sharedCheckBox->SetSize(size); //fits the box into this cell
    _sharedCheckBox->SetPosition(position);
    _sharedCheckBox->Draw();
}

void RetroFuturaGUI::Table::drawColorCell(const BodyCell& cell, const glm::vec3& position, const glm::vec3& size)
{
    if(!_sharedColor)
        return;

    _sharedColor->SetColor(cell._Color);
    _sharedColor->SetSize(size);
    _sharedColor->SetPosition(position);
    _sharedColor->Draw();
}

void RetroFuturaGUI::Table::drawImageCell(const BodyCell& cell, const glm::vec3& position, const glm::vec3& size)
{
    if(!_sharedImage)
        return;

    _sharedImage->SetTexture(cell._Texture);
    _sharedImage->SetSize(size);
    _sharedImage->SetPosition(position);
    _sharedImage->Draw();
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

RetroFuturaGUI::Table::ScissorState RetroFuturaGUI::Table::pushScissor(const f32 left, const f32 bottom, const f32 width, const f32 height) const
{
    // axis-aligned scissor box. breaks when rotation is applied
    ScissorState state {};
    state._WasEnabled = static_cast<bool>(glIsEnabled(GL_SCISSOR_TEST));
    glGetIntegerv(GL_SCISSOR_BOX, state._Previous);

    const glm::ivec2 origin { Window::DrawOriginOf(_parentWindow) }; //off the corner while the window is drawn inside another window's tab
    i32
        clipLeft { origin.x + static_cast<i32>(left) },
        clipBottom { origin.y + static_cast<i32>(bottom) },
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

    if(_selectedArea)
        _selectedArea->SetPosition(position + glm::vec3(0.0f, 0.0f, _widgetZOffset - 0.01f));

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
    if(_hasEditCell)
        EndEdit(); //The cells keep their values where their row and column still exist; the texts drawing them go back to the pool

    releaseTexts();

    const uSize
        previousRowCount { _rowDefinition.size() },
        previousColumnCount { _columnDefinition.size() };
    _rowDefinition = rowDefinition;
    _columnDefinition = columnDefinition;

    if(previousColumnCount == _columnDefinition.size())
    {
        _cells.resize(_rowDefinition.size() * _columnDefinition.size()); //row after row, so the rows kept stay where they are
    }
    else
    {
        std::vector<BodyCell> cells(_rowDefinition.size() * _columnDefinition.size());

        for(uSize row = 0; row < previousRowCount && row < _rowDefinition.size(); ++row)
            for(uSize column = 0; column < previousColumnCount && column < _columnDefinition.size(); ++column)
                cells[row * _columnDefinition.size() + column] = std::move(_cells[row * previousColumnCount + column]);

        _cells = std::move(cells);
    }

    resizeHeaders();
    layoutCells();
    resizeTrackReadOnlyFlags();
    resetSelection(); //the row indices mean other rows now
}

void RetroFuturaGUI::Table::resizeTrackReadOnlyFlags()
{
    if(_tableOrientation == TableOrientation::Row)
        _trackReadOnlyFlags.resize(_columnDefinition.size());
    else
        _trackReadOnlyFlags.resize(_rowDefinition.size());
}

void RetroFuturaGUI::Table::SetScrollPosition(const glm::vec2& scrollPosition)
{
    _scrollPosition = scrollPosition;
    layoutView(); //clamps the new position against the content extent; the tracks themselves don't change
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

void RetroFuturaGUI::Table::resolveTrackOffsets(const std::vector<f32>& sizes, std::vector<f32>& outOffsets)
{
    outOffsets.assign(sizes.size() + 1, 0.0f);

    for(uSize track = 0; track < sizes.size(); ++track)
        outOffsets[track + 1] = outOffsets[track] + sizes[track];
}

void RetroFuturaGUI::Table::resolveVisibleRange(const std::vector<f32>& offsets, const f32 scroll, const f32 viewportExtent, uSize& outFirst, uSize& outEnd)
{
    outFirst = 0;
    outEnd = 0;

    if(2 > offsets.size())
        return; //no tracks

    //A track is in view while its band overlaps the viewport's: it ends past the scroll and starts before the viewport's end. Both are found by halving, since the offsets only grow.
    const uSize
        trackCount { offsets.size() - 1 },
        first { static_cast<uSize>(std::upper_bound(offsets.begin() + 1, offsets.end(), scroll) - (offsets.begin() + 1)) },
        end { static_cast<uSize>(std::lower_bound(offsets.begin(), offsets.begin() + trackCount, scroll + viewportExtent) - offsets.begin()) };

    if(first >= end)
        return;

    outFirst = first;
    outEnd = end;
}

bool RetroFuturaGUI::Table::trackAtOffset(const std::vector<f32>& offsets, const f32 offset, uSize& outTrack)
{
    if(2 > offsets.size())
        return false;

    if(0.0f > offset || offsets.back() <= offset)
        return false;

    outTrack = static_cast<uSize>(std::upper_bound(offsets.begin(), offsets.end(), offset) - offsets.begin()) - 1;
    return true;
}

void RetroFuturaGUI::Table::layoutCells()
{
    if(_columnDefinition.empty())
        return; //without rows the columns are still laid out, so their headers show over an empty table

    const glm::vec2 headers { headerExtents() };
    _innerSize = glm::vec3(_size.x > headers.x ? _size.x - headers.x : 0.0f,
                           _size.y > headers.y ? _size.y - headers.y : 0.0f,
                           _size.z);

    // size every track on its own
    resolveTrackSizes(_rowDefinition, _innerSize.y, _resolvedRowSizes);
    resolveTrackSizes(_columnDefinition, _innerSize.x, _resolvedColumnSizes);
    resolveTrackOffsets(_resolvedRowSizes, _rowOffsets);
    resolveTrackOffsets(_resolvedColumnSizes, _columnOffsets);
    _contentExtent = glm::vec2(_columnOffsets.back(), _rowOffsets.back());

    layoutView();
}

void RetroFuturaGUI::Table::layoutView()
{
    if(_columnDefinition.empty())
        return;

    const glm::vec2 maxScroll { GetMaxScroll() };
    _scrollPosition.x = _scrollPosition.x < 0.0f ? 0.0f : (_scrollPosition.x > maxScroll.x ? maxScroll.x : _scrollPosition.x);
    _scrollPosition.y = _scrollPosition.y < 0.0f ? 0.0f : (_scrollPosition.y > maxScroll.y ? maxScroll.y : _scrollPosition.y);

    resolveVisibleRange(_rowOffsets, _scrollPosition.y, _innerSize.y, _displayedRows[0], _displayedRows[1]);
    resolveVisibleRange(_columnOffsets, _scrollPosition.x, _innerSize.x, _displayedColumns[0], _displayedColumns[1]);

    //The texts scrolled out of view go back to the pool, the edited cell keeping its own with its caret and selection.
    //The rest move with their cells. Cells not drawn yet take their text when they first draw.
    for(uSize index = 0; index < _shownTexts.size();)
    {
        const ShownText& shown { _shownTexts[index] };

        if(!isCellDisplayed(shown._Row, shown._Column) && !isEditCell(shown._Row, shown._Column))
        {
            releaseText(shown._Row, shown._Column); //moves the last one here, so the index stays
            continue;
        }

        if(shown._Widget)
        {
            glm::vec3
                position { 0.0f },
                size { 0.0f };
            cellRect(shown._Row, shown._Column, position, size);
            shown._Widget->SetSize(size);
            shown._Widget->SetPosition(position);
        }

        ++index;
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

void RetroFuturaGUI::Table::SetTrackReadOnly(const uSize trackIndex, const bool readOnly)
{
    resizeTrackReadOnlyFlags();

    if(trackIndex >= _trackReadOnlyFlags.size())
        return;

    _trackReadOnlyFlags[trackIndex] = readOnly;
}

bool RetroFuturaGUI::Table::IsTrackReadOnly(const uSize trackIndex) const
{
    return trackIndex < _trackReadOnlyFlags.size() ? _trackReadOnlyFlags[trackIndex] : false;
}

void RetroFuturaGUI::Table::SetCaretSize(const glm::vec2& size)
{
    if(_caret)
        _caret->SetSize(size);
}

bool RetroFuturaGUI::Table::GetEditedCell(uSize& outRow, uSize& outColumn) const
{
    outRow = _editRow;
    outColumn = _editColumn;
    return _hasEditCell;
}

void RetroFuturaGUI::Table::emitChange()
{
    syncEditedCell();
    _onTextChangeAsync.EmitAsync();
    _onTextChange.Emit();
}

void RetroFuturaGUI::Table::syncEditedCell()
{
    const TableText* text { activeTableText() };

    if(!text)
        return;

    BodyCell* cell { cellAt(_editRow, _editColumn) };

    if(!cell)
        return;

    cell->_Text = text->GetText();

    if(!cell->_ValueStore)
        cell->_ValueStore = std::make_unique<TextTypes>();

    cell->_ValueStore->SetValue(std::string_view { cell->_Text }); //so GetValue parses what was typed
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
    restyleTexts();
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

const std::string& RetroFuturaGUI::Table::GetText(const TrackIndex& index) const
{
    static const std::string dummy {};
    const BodyCell* cell { cellAt(index._Row, index._Column) };

    if(!cell)
        return dummy;

    if(cell->_Type != ITableWidget::TableWidgetTypeID::TableText)
        return dummy;

    return cell->_Text;
}

void RetroFuturaGUI::Table::SetFontFamily(std::string_view fontFamily, const f32 fontSize, const PlatformBridge::Fonts::Slant slant, const PlatformBridge::Fonts::Weight fontWeight)
{
    //A table-wide default, which every text taken from the pool is styled with
    _textDefaults._HasFont = true;
    _textDefaults._FontFamily = fontFamily;
    _textDefaults._FontSize = fontSize;
    _textDefaults._Slant = slant;
    _textDefaults._Weight = fontWeight;
    restyleTexts();
}

void RetroFuturaGUI::Table::SetTextAlignment(const TextAlignment alignment)
{
    _textDefaults._Alignment = alignment;
    restyleTexts();
}

void RetroFuturaGUI::Table::SetTextPadding(const f32 padding)
{
    _textDefaults._Padding = padding;
    restyleTexts();
}

void RetroFuturaGUI::Table::restyleTexts()
{
    for(std::unique_ptr<TableText>& text : _freeTexts)
    {
        if(!text)
            continue;

        styleText(*text);
    }

    //A font swap leaves the glyph meshes behind, so the texts in view are laid out again
    for(ShownText& shown : _shownTexts)
    {
        if(!shown._Widget)
            continue;

        const BodyCell* cell { cellAt(shown._Row, shown._Column) };

        if(!cell)
            continue;

        styleText(*shown._Widget);
        shown._Widget->SetText(cell->_Text);
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
    _innerBorderWidth = width;
}

void RetroFuturaGUI::Table::SetTableOrientation(const TableOrientation orientation)
{
    _tableOrientation = orientation;
    resizeTrackReadOnlyFlags();
    resetSelection(); //selected rows become columns, or the other way round
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

RetroFuturaGUI::TableText* RetroFuturaGUI::Table::activeTableText() const
{
    if(!_hasEditCell)
        return nullptr;

    return shownTextAt(_editRow, _editColumn); //beginEdit bound it, and the layout keeps it bound while it is edited
}

RetroFuturaGUI::Text* RetroFuturaGUI::Table::activeText() const
{
    TableText* tableText { activeTableText() };
    return tableText ? tableText->_text.get() : nullptr;
}

bool RetroFuturaGUI::Table::isEditedTrackReadOnly() const
{
    if(!_hasEditCell)
        return true;

    if(_isEditFromCode)
        return false; //EditCell opens a read-only track for this one edit

    // resizeTrackReadOnlyFlags() sizes the flags across the axis the orientation doesn't band along
    const uSize track { _tableOrientation == TableOrientation::Row ? _editColumn : _editRow };
    return track < _trackReadOnlyFlags.size() ? _trackReadOnlyFlags[track] : false;
}

RetroFuturaGUI::ITextEditVisuals::TextArea RetroFuturaGUI::Table::textArea() const
{
    if(!_hasEditCell || !cellAt(_editRow, _editColumn)) //nothing edited: fall back to the whole table
        return { ._Left = _position.x - _size.x * 0.5f, ._Bottom = _position.y - _size.y * 0.5f, ._Right = _position.x + _size.x * 0.5f, ._Top = _position.y + _size.y * 0.5f };

    glm::vec3
        position { 0.0f },
        size { 0.0f };
    cellRect(_editRow, _editColumn, position, size);

    return { ._Left = position.x - size.x * 0.5f, ._Bottom = position.y - size.y * 0.5f, ._Right = position.x + size.x * 0.5f, ._Top = position.y + size.y * 0.5f };
}

void RetroFuturaGUI::Table::openEdit(const uSize row, const uSize column)
{
    if(_hasEditCell && (_editRow != row || _editColumn != column))
        EndEdit(); // leaving a cell resets its horizontal text scroll

    const bool isNewEdit { !_hasEditCell };
    _editRow = row;
    _editColumn = column;
    _hasEditCell = true;
    _editingEnabled = true;
    _showCaret = true;
    takeKeyboardFocus();
    bindText(row, column); //the caret and the edits need the cell's Text before it draws

    if(!isNewEdit)
        return; //a press into the cell being edited only moves the caret

    const BodyCell* cell { cellAt(row, column) };
    _editStartText = cell ? cell->_Text : std::string {};
    _editKeepKey._PressCountSeen = PlatformBridge::Input::GetKeyPressCount(_editKeepKey._Key);
    _editCancelKey._PressCountSeen = PlatformBridge::Input::GetKeyPressCount(_editCancelKey._Key);
}

void RetroFuturaGUI::Table::beginEdit(const uSize row, const uSize column, const f32 worldX)
{
    openEdit(row, column);
    Text* text { activeText() };

    if(!text)
        return;

    _selectedPositionFirst = _selectedPositionLast = text->GetBoundaryAtPosition(clampToTextBounds(worldX));
    setCaretFromBoundary(_selectedPositionFirst);
    updateSelectedArea();
    _isMarking = true;
}

void RetroFuturaGUI::Table::EditCell(const TrackIndex& index, const uSize selectedLength)
{
    const BodyCell* cell { cellAt(index._Row, index._Column) };

    if(!cell)
        return;

    if(cell->_Type != ITableWidget::TableWidgetTypeID::TableText)
        return;

    ScrollRowIntoView(_tableOrientation == TableOrientation::Row ? index._Row : index._Column, true); //before the text binds, at its place in view
    openEdit(index._Row, index._Column);
    _isEditFromCode = true;
    Text* text { activeText() };

    if(!text)
        return;

    const uSize
        glyphCount { text->GetGlyphCount() },
        selectedCount { selectedLength < glyphCount ? selectedLength : glyphCount };

    _selectedPositionFirst = 0;
    _selectedPositionLast = selectedCount;
    _isSelected = 0 < selectedCount;
    _isMarking = false;
    setCaretFromBoundary(selectedCount);
    updateSelectedArea();
    resetCaretBlink();
}

void RetroFuturaGUI::Table::EndEdit()
{
    endEdit(EditEnd::Kept);
}

void RetroFuturaGUI::Table::endEdit(const EditEnd end)
{
    const bool wasEditing { _hasEditCell };
    const TrackIndex index { ._Row = _editRow, ._Column = _editColumn, ._Layer = 0 };

    if(Text* text { activeText() })
        text->SetScrollOffset(0.0f); // show the cell from its start again once it loses focus

    if(wasEditing && EditEnd::Cancelled == end)
        restoreEditedCell();

    _hasEditCell = false;
    _isEditFromCode = false;
    _editingEnabled = false;
    _showCaret = false;
    _selectedPositionFirst = 0;
    _selectedPositionLast = 0;
    deselect();

    if(!wasEditing)
        return;

    const std::string text { GetText(index) };
    _onCellEditEndedAsync.EmitAsync(index, end, text);
    _onCellEditEnded.Emit(index, end, text); //last, since a slot may refill the table
}

void RetroFuturaGUI::Table::restoreEditedCell()
{
    BodyCell* cell { cellAt(_editRow, _editColumn) };

    if(!cell)
        return;

    cell->_Text = _editStartText;

    if(cell->_ValueStore)
        cell->_ValueStore->SetValue(std::string_view { cell->_Text });

    refreshShownText(_editRow, _editColumn);
}

void RetroFuturaGUI::Table::followEditKeys()
{
    if(!_hasEditCell)
        return;

    if(!hasInputFocus())
        return;

    //Both taken every frame of the edit, so neither fires late
    const bool
        isKept { isKeyTriggered(_editKeepKey, false) },
        isCancelled { isKeyTriggered(_editCancelKey, false) };

    if(isCancelled)
    {
        endEdit(EditEnd::Cancelled);
        return;
    }

    if(!isKept)
        return;

    endEdit(EditEnd::Kept);
}

void RetroFuturaGUI::Table::Connect_OnCellEditEnded(const typename Signal<TrackIndex, EditEnd, std::string>::Slot& slot, const bool async)
{
    if(async)
        _onCellEditEndedAsync.Connect(slot);
    else
        _onCellEditEnded.Connect(slot);
}

void RetroFuturaGUI::Table::Disconnect_OnCellEditEnded(const typename Signal<TrackIndex, EditEnd, std::string>::Slot& slot)
{
    _onCellEditEnded.Disconnect(slot);
    _onCellEditEndedAsync.Disconnect(slot);
}

bool RetroFuturaGUI::Table::isCellDisplayed(const uSize row, const uSize column) const
{
    return _displayedRows[0] <= row && _displayedRows[1] > row && _displayedColumns[0] <= column && _displayedColumns[1] > column;
}

bool RetroFuturaGUI::Table::isEditCell(const uSize row, const uSize column) const
{
    return _hasEditCell && _editRow == row && _editColumn == column;
}

bool RetroFuturaGUI::Table::findCellAt(const glm::vec2& point, const ITableWidget::TableWidgetTypeID typeID, uSize& outRow, uSize& outColumn) const
{
    // only the cells overlapping the viewport can be hit, the rest are scrolled out of sight
    if(!isPointInContent(point))
        return false;

    const glm::vec2 contentPoint { toContentPoint(point) };
    uSize
        row { 0 },
        column { 0 };

    if(!trackAtOffset(_rowOffsets, contentPoint.y, row))
        return false;

    if(!trackAtOffset(_columnOffsets, contentPoint.x, column))
        return false;

    const BodyCell* cell { cellAt(row, column) };

    if(!cell)
        return false;

    if(cell->_Type != typeID)
        return false;

    outRow = row;
    outColumn = column;
    return true;
}

RetroFuturaGUI::Table::BodyCell* RetroFuturaGUI::Table::cellAt(const uSize row, const uSize column)
{
    if(_rowDefinition.size() <= row || _columnDefinition.size() <= column)
        return nullptr;

    const uSize index { row * _columnDefinition.size() + column };
    return index < _cells.size() ? &_cells[index] : nullptr;
}

const RetroFuturaGUI::Table::BodyCell* RetroFuturaGUI::Table::cellAt(const uSize row, const uSize column) const
{
    if(_rowDefinition.size() <= row || _columnDefinition.size() <= column)
        return nullptr;

    const uSize index { row * _columnDefinition.size() + column };
    return index < _cells.size() ? &_cells[index] : nullptr;
}

RetroFuturaGUI::Table::BodyCell* RetroFuturaGUI::Table::cellOfType(const TrackIndex& index, const ITableWidget::TableWidgetTypeID type)
{
    BodyCell* cell { cellAt(index._Row, index._Column) };

    if(!cell)
        return nullptr;

    if(cell->_Type == type)
        return cell;

    //A cell of another type turns into this one: what it held goes, and so does the text drawing it, the edited one's too
    if(isEditCell(index._Row, index._Column))
        EndEdit();

    releaseText(index._Row, index._Column);
    cell->_Text.clear();
    cell->_ValueStore.reset();
    cell->_Color = glm::vec4(0.0f);
    cell->_Texture = nullptr;
    cell->_IsChecked = false;
    cell->_Type = type;
    return cell;
}

void RetroFuturaGUI::Table::cellRect(const uSize row, const uSize column, glm::vec3& outPosition, glm::vec3& outSize) const
{
    outPosition = glm::vec3(0.0f);
    outSize = glm::vec3(0.0f);

    if(_rowOffsets.size() <= row + 1 || _columnOffsets.size() <= column + 1)
        return;

    const glm::vec2 contentOrigin { contentTopLeft() };
    const f32
        width { _columnOffsets[column + 1] - _columnOffsets[column] },
        height { _rowOffsets[row + 1] - _rowOffsets[row] };

    outSize = glm::vec3(width, height, _size.z);
    outPosition = glm::vec3(contentOrigin.x - _scrollPosition.x + _columnOffsets[column] + width * 0.5f,
                            contentOrigin.y + _scrollPosition.y - _rowOffsets[row] - height * 0.5f,
                            _position.z + _widgetZOffset);
}

RetroFuturaGUI::TableText* RetroFuturaGUI::Table::bindText(const uSize row, const uSize column)
{
    BodyCell* cell { cellAt(row, column) };

    if(!cell)
        return nullptr;

    if(cell->_Type != ITableWidget::TableWidgetTypeID::TableText)
        return nullptr;

    if(cell->_HasShownText)
        return shownTextAt(row, column);

    std::unique_ptr<TableText> text {};

    if(!_freeTexts.empty())
    {
        text = std::move(_freeTexts.back());
        _freeTexts.pop_back();
    }
    else
    {
        text = std::make_unique<TableText>(this, &_projection);

        if(text)
            styleText(*text);
    }

    if(!text)
        return nullptr;

    glm::vec3
        position { 0.0f },
        size { 0.0f };
    cellRect(row, column, position, size);
    text->SetSize(size);
    text->SetPosition(position);
    text->SetText(cell->_Text);
    text->ensureText();

    cell->_HasShownText = true;
    TableText* bound { text.get() };
    _shownTexts.push_back(ShownText { ._Row = row, ._Column = column, ._Widget = std::move(text) });
    return bound;
}

RetroFuturaGUI::TableText* RetroFuturaGUI::Table::shownTextAt(const uSize row, const uSize column) const
{
    for(const ShownText& shown : _shownTexts)
    {
        if(shown._Row != row || shown._Column != column)
            continue;

        return shown._Widget.get();
    }

    return nullptr;
}

void RetroFuturaGUI::Table::releaseText(const uSize row, const uSize column)
{
    for(uSize index = 0; index < _shownTexts.size(); ++index)
    {
        if(_shownTexts[index]._Row != row || _shownTexts[index]._Column != column)
            continue;

        if(_shownTexts[index]._Widget)
            _freeTexts.push_back(std::move(_shownTexts[index]._Widget));

        //The last one takes its place, since the order means nothing
        _shownTexts[index] = std::move(_shownTexts.back());
        _shownTexts.pop_back();
        break;
    }

    BodyCell* cell { cellAt(row, column) };

    if(!cell)
        return;

    cell->_HasShownText = false;
}

void RetroFuturaGUI::Table::releaseTexts()
{
    for(ShownText& shown : _shownTexts)
    {
        if(shown._Widget)
            _freeTexts.push_back(std::move(shown._Widget));

        BodyCell* cell { cellAt(shown._Row, shown._Column) };

        if(!cell)
            continue;

        cell->_HasShownText = false;
    }

    _shownTexts.clear();
}

void RetroFuturaGUI::Table::refreshShownText(const uSize row, const uSize column)
{
    const BodyCell* cell { cellAt(row, column) };

    if(!cell)
        return;

    if(!cell->_HasShownText)
        return;

    TableText* text { shownTextAt(row, column) };

    if(!text)
        return;

    text->SetText(cell->_Text);
}

void RetroFuturaGUI::Table::styleText(TableText& text)
{
    if(_textDefaults._HasFont)
        text.SetFontFamily(_textDefaults._FontFamily, _textDefaults._FontSize, _textDefaults._Slant, _textDefaults._Weight);

    text.SetTextAlignment(_textDefaults._Alignment);
    text.SetTextPadding(_textDefaults._Padding);

    if(_nthTrackColors.empty())
        return;

    //The text keeps spans into the table's colors, which outlive every pooled text
    TrackColoring& coloring { _nthTrackColors[0] };
    text.SetTextColors(coloring._TextColorEnabled, ColorState::Enabled);
    text.SetTextColors(coloring._TextColorDisabled, ColorState::Disabled);
    text.SetTextColors(coloring._TextColorClicked, ColorState::Clicked);
    text.SetTextColors(coloring._TextColorHover, ColorState::Hover);
}

RetroFuturaGUI::ColorState RetroFuturaGUI::Table::checkBoxColorState(const uSize row, const uSize column) const
{
    // resizeTrackReadOnlyFlags() sizes the flags across the axis the orientation doesn't band along
    const uSize track { _tableOrientation == TableOrientation::Row ? column : row };

    if(!_isEnabledFlag || IsTrackReadOnly(track))
        return ColorState::Disabled;

    if(!_hasHoveredCheckBox || _hoveredCheckBoxRow != row || _hoveredCheckBoxColumn != column)
        return ColorState::Enabled;

    return _isHoveredCheckBoxHeld ? ColorState::Clicked : ColorState::Hover;
}

void RetroFuturaGUI::Table::setCheckBoxColors(const ColorState state)
{
    std::vector<glm::vec4>
        *background { &_checkBoxBackgroundColorEnabled },
        *border { &_checkBoxBorderColorEnabled },
        *checkmark { &_checkBoxCheckmarkColorEnabled };

    switch(state)
    {
        case ColorState::Hover:
            background = &_checkBoxBackgroundColorHover;
            border = &_checkBoxBorderColorHover;
            checkmark = &_checkBoxCheckmarkColorHover;
        break;
        case ColorState::Clicked:
            background = &_checkBoxBackgroundColorClicked;
            border = &_checkBoxBorderColorClicked;
            checkmark = &_checkBoxCheckmarkColorClicked;
        break;
        case ColorState::Disabled:
            background = &_checkBoxBackgroundColorDisabled;
            border = &_checkBoxBorderColorDisabled;
            checkmark = &_checkBoxCheckmarkColorDisabled;
        break;
        default: //Enabled
        break;
    }

    if(_checkBoxBackground)
        _checkBoxBackground->SetColors(*background);

    if(_checkBoxBorder)
        _checkBoxBorder->SetColors(*border);

    // SetPathFill re-rasterizes the whole SVG, so only touch it when the fill actually changed
    if(_checkmark && _checkmarkFill.colors != *checkmark)
    {
        _checkmarkFill.colors = *checkmark;
        _checkmark->SetPathFill("CheckmarkPath", _checkmarkFill);
    }
}

void RetroFuturaGUI::Table::interact()
{
    const PointerState pointer { readPointer() };

    if(pointer._IsCaptured)
    {
        cancelPress(); //a container or a popup took the pointer, so a band or a drag can't go on
        return; //ScrollView above has taken the pointer
    }

    const glm::vec2 mousePos { pointer._Position };
    const bool isMousePressed { pointer._LeftButtonPressed };

    const MouseState mouse
    {
        ._Position = mousePos,
        ._HasPosition = pointer._HasPosition,
        ._IsInside = pointer._HasPosition && isPointInside(mousePos),
        ._IsPressed = isMousePressed,
        ._PressedThisFrame = isMousePressed && !_wasClicked // every cell kind shares IClickable's _wasClicked, so the press edge is resolved here, once
    };

    const bool
        wasReleasedThisFrame { !isMousePressed && _wasClicked },
        isHovering { _isEnabledFlag && mouse._IsInside };
    _wasClicked = isMousePressed;

    if(isHovering)
    {
        _whileHoverAsync.EmitAsync();
        _whileHover.Emit();
    }

    if(isHovering && !_mouseEnteredFlag) // enter
    {
        _mouseEnteredFlag = true;
        _onMouseEnterAsync.EmitAsync();
        _onMouseEnter.Emit();
    }
    else if(!isHovering && _mouseEnteredFlag) // leave
    {
        _mouseEnteredFlag = false;
        _onMouseLeaveAsync.EmitAsync();
        _onMouseLeave.Emit();
    }

    interactTableText(mouse);
    interactTableCheckBox(mouse);
    followPress(mouse);
    navigateByKeys();

    if(wasReleasedThisFrame) // a release belongs to the table, not to whichever cell kind happens to run last
    {
        _onReleaseAsync.EmitAsync();
        _onRelease.Emit();
    }
}

void RetroFuturaGUI::Table::interactTableCheckBox(const MouseState& mouse)
{
    uSize
        row { 0 },
        column { 0 };

    //The box can be smaller than its cell, so it is the box itself that has to be under the pointer
    _hasHoveredCheckBox = _isEnabledFlag && mouse._IsInside && findCellAt(mouse._Position, ITableWidget::TableWidgetTypeID::TableCheckBox, row, column)
        && isPointOnCheckBox(mouse._Position, row, column);
    _hoveredCheckBoxRow = row;
    _hoveredCheckBoxColumn = column;

    const uSize track { _tableOrientation == TableOrientation::Row ? column : row };
    const bool isInteractive { _hasHoveredCheckBox && !IsTrackReadOnly(track) }; // a read-only box still shows its value, it just doesn't take the click

    _isHoveredCheckBoxHeld = isInteractive && mouse._IsPressed;

    if(isInteractive)
        requestCursorIcon(PlatformBridge::CursorIcon::Hand);

    if(!isInteractive || !mouse._PressedThisFrame)
        return;

    BodyCell* cell { cellAt(row, column) };

    if(!cell)
        return;

    cell->_IsChecked = !cell->_IsChecked;
    _onClickAsync.EmitAsync();
    _onClick.Emit();
    _onCheckBoxChangeAsync.EmitAsync();
    _onCheckBoxChange.Emit();
}

bool RetroFuturaGUI::Table::isPointOnCheckBox(const glm::vec2& point, const uSize row, const uSize column)
{
    if(!_sharedCheckBox)
        return false;

    glm::vec3
        position { 0.0f },
        size { 0.0f };
    cellRect(row, column, position, size);
    _sharedCheckBox->SetSize(size); //fits the box into the cell, as for drawing
    _sharedCheckBox->SetPosition(position);
    return isPointInsideRect(point, _sharedCheckBox->GetSize(), _sharedCheckBox->GetPosition(), _rotation);
}

bool RetroFuturaGUI::Table::GetValue(const TrackIndex& index) const
{
    const BodyCell* cell { cellAt(index._Row, index._Column) };

    if(!cell)
        return false;

    if(cell->_Type != ITableWidget::TableWidgetTypeID::TableCheckBox)
        return false;

    return cell->_IsChecked;
}

void RetroFuturaGUI::Table::SetCheckBoxCornerRadii(const glm::vec4& radii)
{
    if(_checkBoxBackground)
        _checkBoxBackground->SetCornerRadii(radii);
    
    if(_checkBoxBorder)
        _checkBoxBorder->SetCornerRadii(radii);
}

void RetroFuturaGUI::Table::SetCheckBoxSize(const glm::vec3& size)
{
    if(_checkBoxBackground)
        _checkBoxBackground->SetSize(size);
    
    if(_checkBoxBorder)
        _checkBoxBorder->SetSize(size);
        
    if(_checkmark)
        _checkmark->SetSize(size);
}

void RetroFuturaGUI::Table::SetCheckBoxBorderWidth(const f32 width)
{
    if(_checkBoxBorder)
        _checkBoxBorder->SetBorderWidth(width);
}

bool RetroFuturaGUI::Table::pressEditsTextCell(const uSize row, const uSize column) const
{
    const bool isSelecting { RowSelection::None != _rowSelection && _isEnabledFlag };

    //resizeTrackReadOnlyFlags() sizes the flags across the axis the orientation doesn't band along
    const uSize track { _tableOrientation == TableOrientation::Row ? column : row };
    return !isSelecting || !IsTrackReadOnly(track) || isEditCell(row, column); //a press into a cell EditCell opened places the caret
}

bool RetroFuturaGUI::Table::isOverEditableText(const MouseState& mouse) const
{
    if(!_isEnabledFlag || !mouse._IsInside || !isPointInContent(mouse._Position))
        return false;

    uSize
        row { 0 },
        column { 0 };

    if(!findCellAt(mouse._Position, ITableWidget::TableWidgetTypeID::TableText, row, column))
        return false;

    return pressEditsTextCell(row, column);
}

void RetroFuturaGUI::Table::interactTableText(const MouseState& mouse)
{
    if(_isMarking || isOverEditableText(mouse))
        requestCursorIcon(PlatformBridge::CursorIcon::Text); //also all through a drag that marks text

    // Dragging extends the selection within the cell that already has focus
    if(_isMarking)
    {
        if(mouse._IsPressed && mouse._HasPosition)
        {
            if(Text* text { activeText() })
            {
                _selectedPositionLast = text->GetBoundaryAtPosition(clampToTextBounds(mouse._Position.x));
                setCaretFromBoundary(_selectedPositionLast);
                updateSelectedArea();
            }
        }
        else
        {
            _isMarking = false;
        }
    }

    if(mouse._PressedThisFrame) // a fresh press decides which cell, if any, takes focus
    {
        uSize
            row { 0 },
            column { 0 },
            headerColumn { 0 };
        const bool
            isSelecting { RowSelection::None != _rowSelection && _isEnabledFlag },
            isInContent { mouse._IsInside && isPointInContent(mouse._Position) };

        if(mouse._IsInside && findHorizontalHeaderAt(mouse._Position, headerColumn))
        {
            EndEdit();
            _onHorizontalHeaderClickedAsync.EmitAsync(headerColumn);
            _onHorizontalHeaderClicked.Emit(headerColumn); //last, since a slot may refill the table
        }
        else if(isInContent && findCellAt(mouse._Position, ITableWidget::TableWidgetTypeID::TableText, row, column))
        {
            const uSize selectableRow { _tableOrientation == TableOrientation::Row ? row : column };
            const bool isEditing { pressEditsTextCell(row, column) };

            if(isEditing)
            {
                beginEdit(row, column, mouse._Position.x);
            }
            else
            {
                EndEdit(); //a read-only cell in a selecting table shows no caret
                TakeKeyboardFocus();
            }

            _onClickAsync.EmitAsync();
            _onClick.Emit();

            //Last, since a double-click slot may redefine the tracks. A drag in an edited cell marks text instead.
            if(isSelecting)
                beginRowPress(selectableRow, mouse._Position, !isEditing);
        }
        else if(isInContent && isSelecting) // a row of checkboxes or colors, or the space past the rows
        {
            EndEdit();
            TakeKeyboardFocus();

            if(GetRowAt(mouse._Position, row))
                beginRowPress(row, mouse._Position, true);
            else
                beginEmptyPress(mouse._Position);
        }
        else // clicking a gap, a checkbox or anything outside gives up focus
        {
            EndEdit();
        }
    }

    editText();
    moveCaret();
    followEditKeys();

    if(!_caretNeverBlinks)
        updateCaretBlink();
}

void RetroFuturaGUI::Table::drawCaretAndSelection()
{
    if(!_hasEditCell)
        return;

    drawSelectedArea();
    drawCaret();
}

void RetroFuturaGUI::Table::SetValue(const bool value, const TrackIndex& index, const bool emitSignal)
{
    BodyCell* cell { cellOfType(index, ITableWidget::TableWidgetTypeID::TableCheckBox) }; //a cell of another type turns into a checkbox

    if(!cell)
        return;

    cell->_IsChecked = value;

    if(!emitSignal)
        return;

    _onCheckBoxChangeAsync.EmitAsync();
    _onCheckBoxChange.Emit();
}

void RetroFuturaGUI::Table::SetValue(std::string_view text, const TrackIndex& index, const bool emitSignal)
{
    BodyCell* cell { cellOfType(index, ITableWidget::TableWidgetTypeID::TableText) }; //a cell of another type turns into text

    if(!cell)
        return;

    cell->_Text = text;

    if(cell->_ValueStore)
        cell->_ValueStore->SetValue(text); //a value store kept up to date, but none made for verbatim text

    refreshShownText(index._Row, index._Column);

    if(!emitSignal)
        return;

    _onTextChangeAsync.EmitAsync();
    _onTextChange.Emit();
}

void  RetroFuturaGUI::Table::SetTableWidget(const glm::vec4 color, const TrackIndex& index, const bool emitSignal)
{
    BodyCell* cell { cellAt(index._Row, index._Column) };

    if(!cell)
        return;

    //Only an empty cell becomes a color; one holding something else keeps it
    if(ITableWidget::TableWidgetTypeID::None == cell->_Type)
        cell->_Type = ITableWidget::TableWidgetTypeID::TableColor;

    if(cell->_Type != ITableWidget::TableWidgetTypeID::TableColor)
        return;

    cell->_Color = color;

    if(!emitSignal)
        return;

    _onColorChangeAsync.EmitAsync();
    _onColorChange.Emit();
}

void RetroFuturaGUI::Table::SetValue(const ITexture* texture, const TrackIndex& index)
{
    BodyCell* cell { cellOfType(index, ITableWidget::TableWidgetTypeID::TableImage) }; //a cell of another type turns into an image

    if(!cell)
        return;

    cell->_Texture = texture;
}

void RetroFuturaGUI::Table::SetImagePadding(const f32 padding)
{
    _imagePadding = padding;

    if(!_sharedImage)
        return;

    _sharedImage->SetPadding(padding);
}

void RetroFuturaGUI::Table::GetDisplayedRows(uSize& firstOut, uSize& endOut) const
{
    firstOut = _displayedRows[0];
    endOut = (std::min)(_displayedRows[1], _rowDefinition.size());
}

void RetroFuturaGUI::Table::SetRowSelection(const RowSelection selection)
{
    _rowSelection = selection;
    resetSelection();
}

RetroFuturaGUI::Table::RowSelection RetroFuturaGUI::Table::GetRowSelection() const
{
    return _rowSelection;
}

std::vector<uSize> RetroFuturaGUI::Table::GetSelectedRows() const
{
    std::vector<uSize> rows {};

    for(uSize row = 0; row < _selectedRowFlags.size(); ++row)
    {
        if(_selectedRowFlags[row])
            rows.push_back(row);
    }

    return rows;
}

void RetroFuturaGUI::Table::SelectRows(std::span<const uSize> rows, const bool emitSignal)
{
    if(RowSelection::None == _rowSelection)
        return;

    const std::vector<bool> previousFlags { _selectedRowFlags };
    std::fill(_selectedRowFlags.begin(), _selectedRowFlags.end(), false);
    _hasSelectionAnchor = false;

    for(const uSize row : rows)
    {
        if(_selectedRowFlags.size() <= row)
            continue;

        _selectedRowFlags[row] = true;
        _selectionAnchorRow = row;
        _hasSelectionAnchor = true;
        _cursorRow = row; //the keys go on from the last row selected
        _hasCursorRow = true;

        if(RowSelection::Single == _rowSelection)
            break;
    }

    if(!emitSignal)
        return;

    emitSelectionChange(previousFlags);
}

void RetroFuturaGUI::Table::ClearSelection(const bool emitSignal)
{
    const std::vector<bool> previousFlags { _selectedRowFlags };
    std::fill(_selectedRowFlags.begin(), _selectedRowFlags.end(), false);
    _hasSelectionAnchor = false;

    if(!emitSignal)
        return;

    emitSelectionChange(previousFlags);
}

void RetroFuturaGUI::Table::SetSelectedRowBackgroundColors(std::span<glm::vec4> colors)
{
    _selectedRowBackgroundColors.assign(colors.begin(), colors.end());
}

void RetroFuturaGUI::Table::SetSelectedRowTextColors(std::span<glm::vec4> colors)
{
    _selectedRowTextColors.assign(colors.begin(), colors.end());
}

void RetroFuturaGUI::Table::SetDropTargetRow(const uSize row)
{
    if(selectableRowCount() <= row)
    {
        ClearDropTargetRow();
        return;
    }

    _dropTargetRow = row;
    _hasDropTargetRow = true;
}

void RetroFuturaGUI::Table::ClearDropTargetRow()
{
    _hasDropTargetRow = false;
}

void RetroFuturaGUI::Table::SetDropTargetRowBackgroundColors(std::span<glm::vec4> colors)
{
    _dropTargetRowBackgroundColors.assign(colors.begin(), colors.end());
}

void RetroFuturaGUI::Table::Connect_OnSelectionChanged(const typename Signal<>::Slot& slot, const bool async)
{
    if(async)
        _onSelectionChangedAsync.Connect(slot);
    else
        _onSelectionChanged.Connect(slot);
}

void RetroFuturaGUI::Table::Disconnect_OnSelectionChanged(const typename Signal<>::Slot& slot)
{
    _onSelectionChanged.Disconnect(slot);
    _onSelectionChangedAsync.Disconnect(slot);
}

void RetroFuturaGUI::Table::Connect_OnRowDoubleClicked(const typename Signal<uSize>::Slot& slot, const bool async)
{
    if(async)
        _onRowDoubleClickedAsync.Connect(slot);
    else
        _onRowDoubleClicked.Connect(slot);
}

void RetroFuturaGUI::Table::Disconnect_OnRowDoubleClicked(const typename Signal<uSize>::Slot& slot)
{
    _onRowDoubleClicked.Disconnect(slot);
    _onRowDoubleClickedAsync.Disconnect(slot);
}

void RetroFuturaGUI::Table::Connect_OnRowSlowDoubleClicked(const typename Signal<uSize>::Slot& slot, const bool async)
{
    if(async)
        _onRowSlowDoubleClickedAsync.Connect(slot);
    else
        _onRowSlowDoubleClicked.Connect(slot);
}

void RetroFuturaGUI::Table::Disconnect_OnRowSlowDoubleClicked(const typename Signal<uSize>::Slot& slot)
{
    _onRowSlowDoubleClicked.Disconnect(slot);
    _onRowSlowDoubleClickedAsync.Disconnect(slot);
}

void RetroFuturaGUI::Table::SetSlowDoubleClickTime(const std::chrono::milliseconds earliest, const std::chrono::milliseconds latest)
{
    _slowDoubleClickEarliest = earliest;
    _slowDoubleClickLatest = latest < earliest ? earliest : latest;
}

uSize RetroFuturaGUI::Table::selectableRowCount() const
{
    return _tableOrientation == TableOrientation::Row ? _rowDefinition.size() : _columnDefinition.size();
}

void RetroFuturaGUI::Table::resetSelection()
{
    _selectedRowFlags.assign(selectableRowCount(), false);
    _hasSelectionAnchor = false;
    _hasDropTargetRow = false; //an index into the rows from before
    _hasPendingDoubleClick = false;
    _hasCursorRow = false;

    //A band and a pending press point at rows by index, which now mean other rows. A drag carries on: it only reports the pointer.
    if(PressGesture::SelectionDrag != _pressGesture)
        _pressGesture = PressGesture::None;

    _collapseOnRelease = false;
    _isSlowDoubleClickPending = false;
}

void RetroFuturaGUI::Table::beginRowPress(const uSize row, const glm::vec2& point, const bool armsGesture)
{
    if(_selectedRowFlags.size() <= row)
        return;

    const std::vector<bool> previousFlags { _selectedRowFlags };
    const bool
        isMultiple { RowSelection::Multiple == _rowSelection },
        isAlt { isAltHeld() },
        isShift { isShiftHeld() },
        wasSelected { _selectedRowFlags[row] },
        wasOnlySelected { wasSelected && 1 == std::count(previousFlags.begin(), previousFlags.end(), true) };

    _pressGesture = armsGesture ? PressGesture::Pending : PressGesture::None;
    _pressPoint = point;
    _lastDragPoint = point;
    _pressContentPoint = toContentPoint(point);
    _pressedRow = row;
    _collapseOnRelease = false;
    _canDragSelection = false;
    _bandBaseFlags = isAlt ? previousFlags : std::vector<bool>(previousFlags.size(), false);
    _escapeKey._PressCountSeen = PlatformBridge::Input::GetKeyPressCount(_escapeKey._Key);

    if(isMultiple && isShift && _hasSelectionAnchor)
    {
        //Every row from the anchor to this one; with Alt held too they add to the selection. The anchor stays.
        if(!isAlt)
            std::fill(_selectedRowFlags.begin(), _selectedRowFlags.end(), false);

        const uSize
            first { _selectionAnchorRow < row ? _selectionAnchorRow : row },
            last { _selectionAnchorRow < row ? row : _selectionAnchorRow };

        for(uSize selected = first; selected <= last && selected < _selectedRowFlags.size(); ++selected)
            _selectedRowFlags[selected] = true;

        _pressGesture = PressGesture::None; //a range neither drags nor draws a band
    }
    else if(isMultiple && isAlt)
    {
        _selectedRowFlags[row] = !_selectedRowFlags[row];
        _selectionAnchorRow = row;
        _hasSelectionAnchor = true;
    }
    else if(wasSelected)
    {
        //Kept as it is, so the press can drag every selected row. Released without a drag, it selects this row alone.
        _collapseOnRelease = true;
        _canDragSelection = true;
        _selectionAnchorRow = row;
        _hasSelectionAnchor = true;
    }
    else
    {
        std::fill(_selectedRowFlags.begin(), _selectedRowFlags.end(), false);
        _selectedRowFlags[row] = true;
        _selectionAnchorRow = row;
        _hasSelectionAnchor = true;
        _canDragSelection = !isMultiple; //in Multiple a drag from off the selection draws a band
    }

    _cursorRow = row;
    _hasCursorRow = true;

    const std::chrono::steady_clock::time_point now { std::chrono::steady_clock::now() };
    const std::chrono::steady_clock::duration sinceLastPress { now - _lastPressTime };
    const std::chrono::milliseconds
        doubleClickTime { PlatformBridge::Input::GetDoubleClickTime() },
        slowDoubleClickEarliest { _slowDoubleClickEarliest > doubleClickTime ? _slowDoubleClickEarliest : doubleClickTime };
    const bool isDoubleClick { _hasPendingDoubleClick && _lastPressedRow == row && sinceLastPress <= doubleClickTime };

    //Too late for a double-click but early enough for a slow one, on the row that was the only one selected. It counts once the press is released without a drag.
    _isSlowDoubleClickPending = !isDoubleClick && PressGesture::Pending == _pressGesture && wasOnlySelected && !isAlt && !isShift
        && _lastPressedRow == row && slowDoubleClickEarliest < sinceLastPress && _slowDoubleClickLatest >= sinceLastPress;
    _hasPendingDoubleClick = !isDoubleClick; //a third press starts over instead of making a second double-click
    _lastPressedRow = row;
    _lastPressTime = now;

    emitSelectionChange(previousFlags);

    if(!isDoubleClick)
        return;

    _onRowDoubleClickedAsync.EmitAsync(row);
    _onRowDoubleClicked.Emit(row);
}

void RetroFuturaGUI::Table::beginEmptyPress(const glm::vec2& point)
{
    const std::vector<bool> previousFlags { _selectedRowFlags };
    const bool isAlt { isAltHeld() };

    _pressGesture = RowSelection::Multiple == _rowSelection ? PressGesture::Pending : PressGesture::None;
    _pressPoint = point;
    _lastDragPoint = point;
    _pressContentPoint = toContentPoint(point);
    _collapseOnRelease = false;
    _canDragSelection = false;
    _bandBaseFlags = isAlt ? previousFlags : std::vector<bool>(previousFlags.size(), false);
    _escapeKey._PressCountSeen = PlatformBridge::Input::GetKeyPressCount(_escapeKey._Key);
    _hasPendingDoubleClick = false;

    if(!isAlt)
    {
        std::fill(_selectedRowFlags.begin(), _selectedRowFlags.end(), false);
        _hasSelectionAnchor = false;
    }

    emitSelectionChange(previousFlags);
}

void RetroFuturaGUI::Table::followPress(const MouseState& mouse)
{
    if(PressGesture::None == _pressGesture)
        return;

    if(!mouse._IsPressed)
    {
        const PressGesture gesture { _pressGesture };
        _pressGesture = PressGesture::None;

        if(PressGesture::SelectionDrag == gesture)
        {
            _onSelectionDragAsync.EmitAsync(SelectionDragPhase::Released, _lastDragPoint);
            _onSelectionDrag.Emit(SelectionDragPhase::Released, _lastDragPoint);
            return;
        }

        if(PressGesture::Pending != gesture || _selectedRowFlags.size() <= _pressedRow)
            return;

        const bool isSlowDoubleClick { _isSlowDoubleClickPending };
        _isSlowDoubleClickPending = false;

        if(_collapseOnRelease)
        {
            //A plain press on a selected row that didn't drag: now it selects that row alone
            const std::vector<bool> previousFlags { _selectedRowFlags };
            std::fill(_selectedRowFlags.begin(), _selectedRowFlags.end(), false);
            _selectedRowFlags[_pressedRow] = true;
            _collapseOnRelease = false;
            emitSelectionChange(previousFlags);
        }

        if(!isSlowDoubleClick)
            return;

        _onRowSlowDoubleClickedAsync.EmitAsync(_pressedRow);
        _onRowSlowDoubleClicked.Emit(_pressedRow); //last, since a slot may open a prompt or refill the table
        return;
    }

    if(isKeyTriggered(_escapeKey, false))
    {
        cancelPress();
        return;
    }

    const glm::vec2 point { mouse._HasPosition ? mouse._Position : _lastDragPoint };

    if(PressGesture::Pending == _pressGesture)
    {
        if(_dragThreshold >= glm::length(point - _pressPoint))
            return;

        _collapseOnRelease = false;
        _lastAutoScrollTime = std::chrono::steady_clock::now();

        if(_canDragSelection)
        {
            _pressGesture = PressGesture::SelectionDrag;
            _lastDragPoint = point;
            _onSelectionDragAsync.EmitAsync(SelectionDragPhase::Started, point);
            _onSelectionDrag.Emit(SelectionDragPhase::Started, point);
            return;
        }

        if(RowSelection::Multiple != _rowSelection)
        {
            _pressGesture = PressGesture::None;
            return;
        }

        _pressGesture = PressGesture::Band;
    }

    const bool hasScrolled { scrollTowardPointer(point) };

    if(PressGesture::Band == _pressGesture)
    {
        _lastDragPoint = point;
        selectBandRows(point);
        return;
    }

    if(!hasScrolled && point == _lastDragPoint)
        return;

    _lastDragPoint = point;
    _onSelectionDragAsync.EmitAsync(SelectionDragPhase::Moved, point);
    _onSelectionDrag.Emit(SelectionDragPhase::Moved, point);
}

void RetroFuturaGUI::Table::cancelPress()
{
    const PressGesture gesture { _pressGesture };
    _pressGesture = PressGesture::None;
    _collapseOnRelease = false;
    _isSlowDoubleClickPending = false;

    if(PressGesture::SelectionDrag != gesture)
        return;

    _onSelectionDragAsync.EmitAsync(SelectionDragPhase::Cancelled, _lastDragPoint);
    _onSelectionDrag.Emit(SelectionDragPhase::Cancelled, _lastDragPoint);
}

void RetroFuturaGUI::Table::selectBandRows(const glm::vec2& point)
{
    const std::vector<f32>& offsets { selectableTrackOffsets() };
    const std::vector<bool> previousFlags { _selectedRowFlags };
    const f32
        start { selectableAxisOf(_pressContentPoint) },
        end { selectableAxisOf(toContentPoint(point)) },
        low { start < end ? start : end },
        high { start < end ? end : start };

    _selectedRowFlags = _bandBaseFlags.size() == selectableRowCount() ? _bandBaseFlags : std::vector<bool>(selectableRowCount(), false);

    if(2 > offsets.size())
    {
        emitSelectionChange(previousFlags);
        return;
    }

    //The band takes every row that ends past its low edge and starts before or at its high edge
    const uSize
        first { static_cast<uSize>(std::upper_bound(offsets.begin() + 1, offsets.end(), low) - (offsets.begin() + 1)) },
        endRow { static_cast<uSize>(std::upper_bound(offsets.begin(), offsets.end() - 1, high) - offsets.begin()) };

    for(uSize row = first; row < endRow && row < _selectedRowFlags.size(); ++row)
        _selectedRowFlags[row] = true;

    emitSelectionChange(previousFlags);
}

bool RetroFuturaGUI::Table::scrollTowardPointer(const glm::vec2& point)
{
    const std::chrono::steady_clock::time_point now { std::chrono::steady_clock::now() };
    const f32 elapsed { std::chrono::duration<f32>(now - _lastAutoScrollTime).count() };
    const f32 seconds { 0.1f < elapsed ? 0.1f : elapsed }; //a stalled frame doesn't throw the rows far
    _lastAutoScrollTime = now;

    const glm::vec2 contentOrigin { contentTopLeft() };
    const bool isRowOrientation { _tableOrientation == TableOrientation::Row };
    f32 beyond { 0.0f }; //how far the pointer is past the rows: negative before the first, positive after the last

    if(isRowOrientation)
    {
        if(contentOrigin.y < point.y)
            beyond = contentOrigin.y - point.y;
        else if(contentOrigin.y - _innerSize.y > point.y)
            beyond = contentOrigin.y - _innerSize.y - point.y;
    }
    else
    {
        if(contentOrigin.x > point.x)
            beyond = point.x - contentOrigin.x;
        else if(contentOrigin.x + _innerSize.x < point.x)
            beyond = point.x - contentOrigin.x - _innerSize.x;
    }

    if(0.0f == beyond)
        return false;

    const f32
        distance { 0.0f > beyond ? -beyond : beyond },
        speed { distance * _autoScrollPixelsPerSecond < _autoScrollMaxPixelsPerSecond ? distance * _autoScrollPixelsPerSecond : _autoScrollMaxPixelsPerSecond },
        step { (0.0f > beyond ? -speed : speed) * seconds };
    const glm::vec2 previousScroll { _scrollPosition };

    SetScrollPosition(isRowOrientation ? glm::vec2(_scrollPosition.x, _scrollPosition.y + step) : glm::vec2(_scrollPosition.x + step, _scrollPosition.y));

    if(previousScroll == _scrollPosition)
        return false;

    _onScrolledAsync.EmitAsync();
    _onScrolled.Emit();
    return true;
}

void RetroFuturaGUI::Table::drawSelectionBand()
{
    if(PressGesture::Band != _pressGesture)
        return;

    if(!_selectionBand)
        return;

    if(!_selectionBandBorder)
        return;

    //The start is kept in content space, so it moves with its row while the table scrolls
    const glm::vec2
        contentOrigin { contentTopLeft() },
        start { contentOrigin.x + _pressContentPoint.x - _scrollPosition.x, contentOrigin.y - _pressContentPoint.y + _scrollPosition.y },
        center { (start + _lastDragPoint) * 0.5f },
        size { glm::abs(_lastDragPoint - start) };
    const f32 z { _position.z + _widgetZOffset + 0.02f }; //over the cells' text

    _selectionBand->SetSize(size);
    _selectionBand->SetPosition(glm::vec3(center, z));
    _selectionBand->SetColors(_selectionBandColors);
    _selectionBand->Draw();
    _selectionBandBorder->SetSize(size);
    _selectionBandBorder->SetPosition(glm::vec3(center, z + 0.01f));
    _selectionBandBorder->SetColors(_selectionBandBorderColors);
    _selectionBandBorder->Draw();
}

void RetroFuturaGUI::Table::navigateByKeys()
{
    if(_hasEditCell || RowSelection::None == _rowSelection || !_isEnabledFlag || !hasInputFocus())
    {
        _hadKeyboardFocus = false; //the keys belong to the edited cell, or to another widget
        return;
    }

    if(!_hadKeyboardFocus)
    {
        syncKeyCounts(); //keys pressed while another widget had them are spent
        _hadKeyboardFocus = true;
        return;
    }

    //Every key is polled each frame, so none fires late once Ctrl or Alt is released
    std::array<bool, 6> isTriggered {};

    for(uSize index = 0; index < _navigationKeys.size(); ++index)
        isTriggered[index] = isKeyTriggered(_navigationKeys[index], 4 > index); //Up, Down, PageUp and PageDown repeat while held

    const bool isSelectAllTriggered { isKeyTriggered(_selectAllKey, false) };
    const uSize rowCount { selectableRowCount() };

    if(0 == rowCount)
        return;

    if(isSelectAllTriggered && isControlHeld())
    {
        if(RowSelection::Multiple != _rowSelection)
            return;

        const std::vector<bool> previousFlags { _selectedRowFlags };
        std::fill(_selectedRowFlags.begin(), _selectedRowFlags.end(), true);
        emitSelectionChange(previousFlags);
        return;
    }

    if(isControlHeld() || isAltHeld())
        return; //the arrows with Ctrl or Alt are the app's, an explorer's Back, Forward and Up for instance

    const bool isRowOrientation { _tableOrientation == TableOrientation::Row };
    const uSize
        last { rowCount - 1 },
        current { _hasCursorRow && _cursorRow < rowCount ? _cursorRow : 0 },
        shownCount { isRowOrientation ? _displayedRows[1] - _displayedRows[0] : _displayedColumns[1] - _displayedColumns[0] },
        page { 2 < shownCount ? shownCount - 2 : 1 }; //the rows wholly in view, without the two cut at the edges
    uSize target { current };

    if(isTriggered[0]) //Up
        target = _hasCursorRow && 0 < current ? current - 1 : 0;
    else if(isTriggered[1]) //Down
        target = _hasCursorRow && current < last ? current + 1 : (_hasCursorRow ? last : 0);
    else if(isTriggered[2]) //PageUp
        target = current > page ? current - page : 0;
    else if(isTriggered[3]) //PageDown
        target = current + page < last ? current + page : last;
    else if(isTriggered[4]) //Home
        target = 0;
    else if(isTriggered[5]) //End
        target = last;
    else
        return;

    moveCursorTo(target, RowSelection::Multiple == _rowSelection && isShiftHeld());
}

void RetroFuturaGUI::Table::moveCursorTo(const uSize row, const bool extend)
{
    if(_selectedRowFlags.size() <= row)
        return;

    const std::vector<bool> previousFlags { _selectedRowFlags };
    std::fill(_selectedRowFlags.begin(), _selectedRowFlags.end(), false);

    if(extend && _hasSelectionAnchor)
    {
        const uSize
            first { _selectionAnchorRow < row ? _selectionAnchorRow : row },
            last { _selectionAnchorRow < row ? row : _selectionAnchorRow };

        for(uSize selected = first; selected <= last && selected < _selectedRowFlags.size(); ++selected)
            _selectedRowFlags[selected] = true;
    }
    else
    {
        _selectedRowFlags[row] = true;
        _selectionAnchorRow = row;
        _hasSelectionAnchor = true;
    }

    _cursorRow = row;
    _hasCursorRow = true;
    _hasPendingDoubleClick = false;
    ScrollRowIntoView(row, true);
    emitSelectionChange(previousFlags); //last, since a slot may refill the table
}

void RetroFuturaGUI::Table::ScrollRowIntoView(const uSize row, const bool emitSignal)
{
    const std::vector<f32>& offsets { selectableTrackOffsets() };

    if(offsets.size() <= row + 1)
        return;

    const bool isRowOrientation { _tableOrientation == TableOrientation::Row };
    const f32
        start { offsets[row] },
        end { offsets[row + 1] },
        viewport { isRowOrientation ? _innerSize.y : _innerSize.x },
        scroll { isRowOrientation ? _scrollPosition.y : _scrollPosition.x };
    f32 target { scroll };

    if(start < scroll)
        target = start;
    else if(end > scroll + viewport)
        target = end - viewport;

    if(target == scroll)
        return;

    SetScrollPosition(isRowOrientation ? glm::vec2(_scrollPosition.x, target) : glm::vec2(target, _scrollPosition.y));

    if(!emitSignal)
        return;

    _onScrolledAsync.EmitAsync();
    _onScrolled.Emit();
}

bool RetroFuturaGUI::Table::isKeyTriggered(KeyRepeat& key, const bool repeats)
{
    const u32 pressCount { PlatformBridge::Input::GetKeyPressCount(key._Key) };

    if(pressCount != key._PressCountSeen)
    {
        key._PressCountSeen = pressCount;
        key._HoldFrames = 0;
        return true;
    }

    if(!repeats || !PlatformBridge::Input::IsKeyDown(key._Key))
    {
        key._HoldFrames = 0;
        return false;
    }

    ++key._HoldFrames;
    return shouldRepeat(key._HoldFrames);
}

void RetroFuturaGUI::Table::syncKeyCounts()
{
    for(KeyRepeat& key : _navigationKeys)
    {
        key._PressCountSeen = PlatformBridge::Input::GetKeyPressCount(key._Key);
        key._HoldFrames = 0;
    }

    _selectAllKey._PressCountSeen = PlatformBridge::Input::GetKeyPressCount(_selectAllKey._Key);
    _escapeKey._PressCountSeen = PlatformBridge::Input::GetKeyPressCount(_escapeKey._Key);
}

void RetroFuturaGUI::Table::TakeKeyboardFocus()
{
    takeKeyboardFocus();
    syncKeyCounts();
    _hadKeyboardFocus = true;
}

bool RetroFuturaGUI::Table::HasKeyboardFocus() const
{
    return hasInputFocus();
}

bool RetroFuturaGUI::Table::findHorizontalHeaderAt(const glm::vec2& point, uSize& outColumn) const
{
    if(!_displayHorizontalHeader)
        return false;

    //The band is clipped to the content's width, so a header scrolled under the row header can't be hit
    const glm::vec2 contentOrigin { contentTopLeft() };

    if(contentOrigin.x > point.x || contentOrigin.x + _innerSize.x < point.x)
        return false;

    const uSize columnEnd { _displayedColumns[1] < _horizontalHeaderCells.size() ? _displayedColumns[1] : _horizontalHeaderCells.size() };

    for(uSize column = _displayedColumns[0]; column < columnEnd; ++column)
    {
        const TableCell& cell { _horizontalHeaderCells[column] };

        if(!isPointInsideRect(point, cell._SizePixels, cell._PositionPixels, _rotation))
            continue;

        outColumn = column;
        return true;
    }

    return false;
}

bool RetroFuturaGUI::Table::isPointInContent(const glm::vec2& point) const
{
    const glm::vec2 contentOrigin { contentTopLeft() };

    return contentOrigin.x <= point.x && contentOrigin.x + _innerSize.x >= point.x
        && contentOrigin.y >= point.y && contentOrigin.y - _innerSize.y <= point.y;
}

glm::vec2 RetroFuturaGUI::Table::toContentPoint(const glm::vec2& point) const
{
    const glm::vec2 contentOrigin { contentTopLeft() };
    return glm::vec2(point.x - contentOrigin.x + _scrollPosition.x, contentOrigin.y - point.y + _scrollPosition.y);
}

const std::vector<f32>& RetroFuturaGUI::Table::selectableTrackOffsets() const
{
    return _tableOrientation == TableOrientation::Row ? _rowOffsets : _columnOffsets;
}

f32 RetroFuturaGUI::Table::selectableAxisOf(const glm::vec2& contentPoint) const
{
    return _tableOrientation == TableOrientation::Row ? contentPoint.y : contentPoint.x;
}

bool RetroFuturaGUI::Table::GetRowAt(const glm::vec2& point, uSize& outRow) const
{
    if(!isPointInContent(point))
        return false;

    return trackAtOffset(selectableTrackOffsets(), selectableAxisOf(toContentPoint(point)), outRow);
}

bool RetroFuturaGUI::Table::isAltHeld()
{
    return PlatformBridge::Input::IsKeyDown(PB_KEY_ALT_L) || PlatformBridge::Input::IsKeyDown(PB_KEY_ALT_R);
}

bool RetroFuturaGUI::Table::isShiftHeld()
{
    return PlatformBridge::Input::IsKeyDown(PB_KEY_SHIFT_L) || PlatformBridge::Input::IsKeyDown(PB_KEY_SHIFT_R);
}

bool RetroFuturaGUI::Table::isControlHeld()
{
    return PlatformBridge::Input::IsKeyDown(PB_KEY_CONTROL_L) || PlatformBridge::Input::IsKeyDown(PB_KEY_CONTROL_R);
}

void RetroFuturaGUI::Table::SetSelectionBandColors(std::span<glm::vec4> colors)
{
    _selectionBandColors.assign(colors.begin(), colors.end());
}

void RetroFuturaGUI::Table::SetSelectionBandBorderColors(std::span<glm::vec4> colors)
{
    _selectionBandBorderColors.assign(colors.begin(), colors.end());
}

void RetroFuturaGUI::Table::Connect_OnSelectionDrag(const typename Signal<SelectionDragPhase, glm::vec2>::Slot& slot, const bool async)
{
    if(async)
        _onSelectionDragAsync.Connect(slot);
    else
        _onSelectionDrag.Connect(slot);
}

void RetroFuturaGUI::Table::Disconnect_OnSelectionDrag(const typename Signal<SelectionDragPhase, glm::vec2>::Slot& slot)
{
    _onSelectionDrag.Disconnect(slot);
    _onSelectionDragAsync.Disconnect(slot);
}

void RetroFuturaGUI::Table::Connect_OnHorizontalHeaderClicked(const typename Signal<uSize>::Slot& slot, const bool async)
{
    if(async)
        _onHorizontalHeaderClickedAsync.Connect(slot);
    else
        _onHorizontalHeaderClicked.Connect(slot);
}

void RetroFuturaGUI::Table::Disconnect_OnHorizontalHeaderClicked(const typename Signal<uSize>::Slot& slot)
{
    _onHorizontalHeaderClicked.Disconnect(slot);
    _onHorizontalHeaderClickedAsync.Disconnect(slot);
}

void RetroFuturaGUI::Table::Connect_OnScrolled(const typename Signal<>::Slot& slot, const bool async)
{
    if(async)
        _onScrolledAsync.Connect(slot);
    else
        _onScrolled.Connect(slot);
}

void RetroFuturaGUI::Table::Disconnect_OnScrolled(const typename Signal<>::Slot& slot)
{
    _onScrolled.Disconnect(slot);
    _onScrolledAsync.Disconnect(slot);
}

void RetroFuturaGUI::Table::emitSelectionChange(const std::vector<bool>& previousFlags)
{
    if(previousFlags == _selectedRowFlags)
        return;

    _onSelectionChangedAsync.EmitAsync();
    _onSelectionChanged.Emit();
}