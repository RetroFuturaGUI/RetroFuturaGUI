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

    _axisColoringOverlay = std::make_unique<Rectangle>(projection);

    if(_axisColoringOverlay)
        _axisColoringOverlay->SetRectangleMode(RectangleMode::Plane);
    
    _innerBorder = std::make_unique<Rectangle>(projection);

    if(_innerBorder)
        _innerBorder->SetRectangleMode(RectangleMode::Border);
}

void RetroFuturaGUI::Table::Draw()
{
    drawBorder();

    if(_nthAxisColors.empty())
        return;

    const uSize variantCount { _nthAxisColors.size() };

    for(uSize row = 0; row < _tableCells.size(); ++row)
    {
        for(uSize column = 0; column < _tableCells[row].size(); ++column)
        {
            const TableCell& cell { _tableCells[row][column] };

            if(!cell._TableWidget)
                continue;

            const uSize index { (_tableOrientation == TableOrientation::Row ? row : column) % variantCount };
            AxisColoring& coloring { _nthAxisColors[index] };

            if(_axisColoringOverlay)
            {
                _axisColoringOverlay->SetSize(cell._SizePixels);
                _axisColoringOverlay->SetPosition(cell._PositionPixels - glm::vec3(0.0f, 0.0f, _widgetZOffset));
                _axisColoringOverlay->SetColors(coloring._BackgroundColorEnabled);
                _axisColoringOverlay->Draw();
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

void RetroFuturaGUI::Table::SetAxisDefinitions(const std::vector<f32>& rowDefinition, const std::vector<f32>& columnDefinition)
{
    _rowDefinition = rowDefinition;
    _columnDefinition = columnDefinition;
    _tableCells.resize(_rowDefinition.size());

    for(auto& row : _tableCells)
        row.resize(_columnDefinition.size());

    layoutCells();
}

void RetroFuturaGUI::Table::layoutCells()
{
    if(_rowDefinition.empty() || _columnDefinition.empty())
        return;

    const f32
        tableLeftX { _position.x - _size.x * 0.5f },
        tableTopY { _position.y + _size.y * 0.5f };
    f32 accumY { 0.0f };

    for(uSize row = 0; row < _rowDefinition.size() && row < _tableCells.size(); ++row)
    {
        const f32 cellSizeY { _rowDefinition[row] * _size.y };
        f32 accumX { 0.0f };

        for(uSize column = 0; column < _columnDefinition.size() && column < _tableCells[row].size(); ++column)
        {
            const f32 cellSizeX { _columnDefinition[column] * _size.x };
            TableCell& cell { _tableCells[row][column] };

            cell._SizeNormalized = glm::vec3(_columnDefinition[column], _rowDefinition[row], 1.0f);
            cell._SizePixels = glm::vec3(cellSizeX, cellSizeY, _size.z);
            cell._PositionPixels = glm::vec3(tableLeftX + accumX + cellSizeX * 0.5f,
                                              tableTopY - accumY - cellSizeY * 0.5f,
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
    if(_nthAxisColors.empty())
        _nthAxisColors.resize(1);

    if(nthIndex >= _nthAxisColors.size())
        return;

    AxisColoring& coloring { _nthAxisColors[nthIndex] };
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
    if(_nthAxisColors.empty())
        return {};

    const AxisColoring& coloring { _nthAxisColors[0] };

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

void RetroFuturaGUI::Table::SetAxisBackgroundColors(std::span<glm::vec4> colors, const ColorState state, const uSize nthIndex)
{
    if(_nthAxisColors.empty())
        return;

    if(nthIndex >= _nthAxisColors.size())
        return;

    AxisColoring& coloring { _nthAxisColors[nthIndex] };

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

void RetroFuturaGUI::Table::SetAxisBorderColors(std::span<glm::vec4> colors, const ColorState state, const uSize nthIndex)
{
    if(_nthAxisColors.empty())
        return;

    if(nthIndex >= _nthAxisColors.size())
        return;

    AxisColoring& coloring { _nthAxisColors[nthIndex] };

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

void RetroFuturaGUI::Table::SetAxisAlternatingColorCount(const uSize variantCount)
{
    _nthAxisColors.resize(variantCount);
}