#include "Grid2D.hpp"

RetroFuturaGUI::Grid2d::Grid2d(const IdentityParams &identity, const GeometryParams2D &geometry, const Grid2dAxisDefinition &axisDefinition)
: IWidget(identity, geometry), _axisdefinition(axisDefinition)
{
    _grid.reserve(_maxCountPerAxis);

    for(u32 i = 0; i < _maxCountPerAxis; ++i)
    {
        _grid.emplace_back();
        _grid.back().reserve(_maxCountPerAxis);
    }

    for(u32 row = 0; row < _axisdefinition._RowDefinition.size(); ++row)
    {
        for(u32 column = 0; column < _axisdefinition._ColumnDefinition.size(); ++column)
        {
            f32 posX = 0.0f, posY = 0.0f;

            for(u32 i = 0; i < row; ++i)
                posX += _axisdefinition._RowDefinition[i];

            for(u32 i = 0; i < column; ++i)
                posY += _axisdefinition._ColumnDefinition[i];

            _grid[row].push_back(Grid2dCell
                {
                    glm::vec2(_projection.GetResolution().x * posX, _projection.GetResolution().y * posY),
                    glm::vec2(posX, posY),
                    glm::vec2(_projection.GetResolution().x * _axisdefinition._RowDefinition[row], _projection.GetResolution().y * _axisdefinition._ColumnDefinition[column]),
                    glm::vec2(_axisdefinition._RowDefinition[row], _axisdefinition._RowDefinition[column]),
                    glm::vec2(0.0f),
                    glm::vec2(0.0f),
                    1, 1,
                    nullptr,
                    false,
                    SizingMode::FILL
                }
            );
        }
    }

    GeometryParams2D geometryb = 
    {
        _projection,
        glm::vec2(0.0f),
        glm::vec2(0.0f),
        0.0f
    };

    BorderParams borderP = 
    {
        glm::vec4(1.0f),
        5.0f
    };

    _debugBorder = std::make_unique<LineBorder>(geometryb, borderP);
}

void RetroFuturaGUI::Grid2d::AttachWidget(u32 row, u32 col, std::unique_ptr<IWidget> widget, const SizingMode sizingMode)
{
    if(_axisdefinition._RowDefinition.size() < row || _axisdefinition._ColumnDefinition.size() < col)
        return;
        
    if(_grid[row][col]._SpanOccupied)
        return;

    _grid[row][col]._Widget = std::move(widget);
    _grid[row][col]._Widget->SetPosition(glm::vec2(_grid[row][col]._PositionPixels.x + _grid[row][col]._SizePixels.x * 0.5f,
                                                   _projection.GetResolution().y - _grid[row][col]._PositionPixels.y - _grid[row][col]._SizePixels.y * 0.5f));
    _grid[row][col]._SizingMode = sizingMode;
    resizeWidget(_grid[row][col]);
}

void RetroFuturaGUI::Grid2d::Draw(const bool alsoDrawDebugLines)
{
    for(auto& row : _grid)
    {
        for(auto& cell : row)
        {
            if(alsoDrawDebugLines)
                drawDebugLines(cell);

            if(cell._Widget == nullptr || cell._SpanOccupied)
                continue;

            cell._Widget->Draw();
        }
    }
}

void RetroFuturaGUI::Grid2d::SetSize(const glm::vec2 &size)
{
    _size = size;
    resizeCells();
    resizeAllWidgets();
    moveWidgets();
}

void RetroFuturaGUI::Grid2d::SetPosition(const glm::vec2 &position)
{
    _position = position;
    resizeCells();
    resizeAllWidgets();
}

void RetroFuturaGUI::Grid2d::drawDebugLines(const Grid2dCell &cell)
{
    _debugBorder->Move(glm::vec2(cell._PositionPixels.x + cell._SizePixels.x * 0.5f, _projection.GetResolution().y - (cell._PositionPixels.y + cell._SizePixels.y * 0.5f)));
    _debugBorder->Resize(glm::vec2(cell._SizePixels.x, cell._SizePixels.y));
    _debugBorder->Draw();
}

void RetroFuturaGUI::Grid2d::resizeCells()
{
    for(u32 row = 0; row < _axisdefinition._RowDefinition.size(); ++row)
    {
        for(u32 column = 0; column < _axisdefinition._ColumnDefinition.size(); ++column)
        {
            f32 posX = 0.0f, posY = 0.0f;

            for(u32 i = 0; i < row; ++i)
                posX += _axisdefinition._RowDefinition[i] * _size.x;

            for(u32 i = 0; i < column; ++i)
                posY += _axisdefinition._ColumnDefinition[i] * _size.y;

                
            _grid[row][column]._PositionPixels = glm::vec2(posX, posY);
            _grid[row][column]._PositionNormalized = glm::vec2(posX / _size.x, posY / _size.y);
            _grid[row][column]._SizePixels = glm::vec2(_axisdefinition._RowDefinition[row] * _size.x, _axisdefinition._ColumnDefinition[column] * _size.y);
            _grid[row][column]._SizeNormalized = glm::vec2(_axisdefinition._RowDefinition[row], _axisdefinition._ColumnDefinition[column]);
        }
    }
}

void RetroFuturaGUI::Grid2d::resizeWidget(const Grid2dCell &cell)
{
    if(!cell._Widget)
        return;

    switch(cell._SizingMode)
    {
        case SizingMode::FILL:
            cell._Widget->SetSize(cell._SizePixels);
        break;
        case SizingMode::FILL_X:
            cell._Widget->SetSize(glm::vec2(cell._SizePixels.x, cell._Widget->GetSize().y));
        break;
        case SizingMode::FILL_Y:
            cell._Widget->SetSize(glm::vec2(cell._Widget->GetSize().x,  cell._SizePixels.y));
        break;
        case SizingMode::FIXED:
        default:
        break;
    }
}

void RetroFuturaGUI::Grid2d::resizeAllWidgets()
{
    for(auto& column : _grid)
        for(auto& cell : column)
            resizeWidget(cell);
}

void RetroFuturaGUI::Grid2d::moveWidgets()
{
    for(auto& column : _grid)
    {
        for(auto& cell : column)
        {
            if(!cell._Widget)
                continue;

            cell._Widget->SetPosition(glm::vec2(cell._PositionPixels.x + cell._SizePixels.x * 0.5f,
                _projection.GetResolution().y - cell._PositionPixels.y - cell._SizePixels.y * 0.5f));
        }
    }
}