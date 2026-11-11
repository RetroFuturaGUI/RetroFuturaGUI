#include "Lasagna.hpp"
#include <algorithm>
#include <optional>
#include <utility>

RetroFuturaGUI::Lasagna::Lasagna(std::string_view name, Projection* projection, IHierarchyNode* parentWidget, const WidgetTypeID parentWidgetTypeID, GLFWwindow* parentWindow, const AxisDefinition& axisDefinition)
    : IWidget(name, projection, parentWidget, parentWidgetTypeID, parentWindow), _axisDefinition(axisDefinition)
{
    _widgetTypeID = WidgetTypeID::Lasagna;
    _size = glm::vec3(_projection.GetResolution().x, _projection.GetResolution().y, _projection.GetDepth());
    _position = glm::vec3(_size.x * 0.5f, _size.y * 0.5f, 0.0f);

    if(_axisDefinition._RowDefinition.size() > _maxCountPerAxis)
        _axisDefinition._RowDefinition.resize(_maxCountPerAxis);

    if(_axisDefinition._ColumnDefinition.size() > _maxCountPerAxis)
        _axisDefinition._ColumnDefinition.resize(_maxCountPerAxis);

    if(_axisDefinition._LayerDefinition.size() > _maxCountPerAxis)
        _axisDefinition._LayerDefinition.resize(_maxCountPerAxis);

    buildCells();

    _debugBorder = std::make_unique<Frame>(&_projection);

    if(!_debugBorder)
        return;

    _debugBorder->SetPosition(glm::vec3(0.0f));
    _debugBorder->SetSize(glm::vec3(0.0f));
    _debugBorder->SetRotation(glm::vec3(0.0f));
    _debugBorder->SetColors(_debugBorderColor);
    _debugBorder->SetBorderWidth(5.0f);
}

bool RetroFuturaGUI::Lasagna::RemoveWidget(std::string_view name)
{
    const std::optional<TrackIndex> index { findWidgetIndex(name) };

    if(!index)
        return false;

    LasagnaCell& origin { _lasagna[index->_Row][index->_Column][index->_Layer] };

    //Kept alive until the next Interact, since one of its own slots may be running right now
    _removedWidgets.push_back(std::move(origin._Widget));

    for(uSize r = index->_Row; r < index->_Row + origin._RowSpan; ++r)
        for(uSize c = index->_Column; c < index->_Column + origin._ColSpan; ++c)
            for(uSize l = index->_Layer; l < index->_Layer + origin._LayerSpan; ++l)
                _lasagna[r][c][l]._SpanOccupied = false;

    origin._RowSpan = 1;
    origin._ColSpan = 1;
    origin._LayerSpan = 1;
    origin._SizingMode = SizingMode::FILL;
    origin._Show = true;
    resizeCells(); //the origin cell shrinks back from the span's size to its own
    return true;
}

void RetroFuturaGUI::Lasagna::ShowWidget(std::string_view name, const bool show)
{
    LasagnaCell* cell { findCell(name) };

    if(!cell)
        return;

    cell->_Show = show;
}

bool RetroFuturaGUI::Lasagna::IsWidgetShown(std::string_view name) const
{
    const LasagnaCell* cell { findCell(name) };

    if(!cell)
        return false;

    return cell->_Show;
}

RetroFuturaGUI::LasagnaCell* RetroFuturaGUI::Lasagna::findCell(std::string_view name)
{
    return const_cast<LasagnaCell*>(std::as_const(*this).findCell(name));
}

const RetroFuturaGUI::LasagnaCell* RetroFuturaGUI::Lasagna::findCell(std::string_view name) const
{
    const std::optional<TrackIndex> index { findWidgetIndex(name) };

    if(!index)
        return nullptr;

    return GetCell(*index);
}

std::optional<RetroFuturaGUI::TrackIndex> RetroFuturaGUI::Lasagna::findWidgetIndex(std::string_view name) const
{
    for(uSize row = 0; row < _lasagna.size(); ++row)
        for(uSize column = 0; column < _lasagna[row].size(); ++column)
            for(uSize layer = 0; layer < _lasagna[row][column].size(); ++layer)
            {
                const LasagnaCell& cell { _lasagna[row][column][layer] };

                if(cell._Widget && name == cell._Widget->GetName())
                    return TrackIndex { ._Row = row, ._Column = column, ._Layer = layer };
            }

    return std::nullopt;
}

bool RetroFuturaGUI::Lasagna::isPlacementFree(const CellPlacement& placement) const
{
    const u32
        rowSpan { 0 == placement._RowSpan ? 1 : placement._RowSpan },
        colSpan { 0 == placement._ColSpan ? 1 : placement._ColSpan },
        layerSpan { 0 == placement._LayerSpan ? 1 : placement._LayerSpan };

    if(_axisDefinition._RowDefinition.size() < placement._Row + rowSpan
    || _axisDefinition._ColumnDefinition.size() < placement._Column + colSpan
    || _axisDefinition._LayerDefinition.size() < placement._Layer + layerSpan)
        return false;

    for(u32 r = placement._Row; r < placement._Row + rowSpan; ++r)
        for(u32 c = placement._Column; c < placement._Column + colSpan; ++c)
            for(u32 l = placement._Layer; l < placement._Layer + layerSpan; ++l)
                if(_lasagna[r][c][l]._SpanOccupied || _lasagna[r][c][l]._Widget)
                    return false;

    return true;
}

void RetroFuturaGUI::Lasagna::placeWidget(std::unique_ptr<IWidget> widget, const CellPlacement& placement)
{
    const Transitions::InstantScope instant { true }; //a container lays its widgets out at once, whatever transitions they have
    const u32
        row { placement._Row },
        col { placement._Column },
        layer { placement._Layer },
        rowSpan { 0 == placement._RowSpan ? 1 : placement._RowSpan },
        colSpan { 0 == placement._ColSpan ? 1 : placement._ColSpan },
        layerSpan { 0 == placement._LayerSpan ? 1 : placement._LayerSpan };

    LasagnaCell& origin = _lasagna[row][col][layer];
    origin._RowSpan = rowSpan;
    origin._ColSpan = colSpan;
    origin._LayerSpan = layerSpan;
    origin._SizingMode = placement._SizingMode;
    origin._Widget = std::move(widget);

    for(u32 r = row; r < row + rowSpan; ++r)
        for(u32 c = col; c < col + colSpan; ++c)
            for(u32 l = layer; l < layer + layerSpan; ++l)
                if(r != row || c != col || l != layer)
                    _lasagna[r][c][l]._SpanOccupied = true;

    updateSpanSize(origin, row, col, layer);

    const glm::vec3 topLeft { getTopLeftCorner() };

    origin._Widget->SetPosition(glm::vec3(topLeft.x + origin._PositionPixels.x + origin._SizePixels.x * 0.5f,
                                          topLeft.y - origin._PositionPixels.y - origin._SizePixels.y * 0.5f,
                                          topLeft.z + origin._PositionPixels.z + origin._SizePixels.z * 0.5f
                                        ));
    resizeWidget(origin);
}

RetroFuturaGUI::LasagnaCell* RetroFuturaGUI::Lasagna::GetCell(const TrackIndex& index)
{
    return const_cast<LasagnaCell*>(std::as_const(*this).GetCell(index));
}

const RetroFuturaGUI::LasagnaCell* RetroFuturaGUI::Lasagna::GetCell(const TrackIndex& index) const
{
    if(index._Row >= _lasagna.size()
    || index._Column >= _lasagna[index._Row].size()
    || index._Layer >= _lasagna[index._Row][index._Column].size())
        return nullptr;

    return &_lasagna[index._Row][index._Column][index._Layer];
}

RetroFuturaGUI::TrackIndex RetroFuturaGUI::Lasagna::GetTrackCounts() const
{
    return TrackIndex { ._Row = _axisDefinition._RowDefinition.size(),
                        ._Column = _axisDefinition._ColumnDefinition.size(),
                        ._Layer = _axisDefinition._LayerDefinition.size() };
}

void RetroFuturaGUI::Lasagna::Draw()
{
    for(auto& row : _lasagna)
    {
        for(auto& column : row)
        {
            for(auto& cell : column)
            {
                if(_drawDebugLines)
                    drawDebugLines(cell);

                if(!cell._Widget || cell._SpanOccupied || !cell._Show)
                    continue;

                cell._Widget->DrawAtOpacity();
            }
        }
    }
}

void RetroFuturaGUI::Lasagna::interact()
{
    _removedWidgets.clear(); //no slot of theirs runs between frames
    _retiredCells.clear(); //nor a loop over them

    for(auto& row : _lasagna)
    {
        for(auto& column : row)
        {
            for(auto& cell : column)
            {
                                                         // A hidden cell's widget mustn't take input
                if(!cell._Widget || cell._SpanOccupied || !cell._Show)
                    continue;

                cell._Widget->Interact();
            }
        }
    }
}

void RetroFuturaGUI::Lasagna::update()
{
    for(auto& row : _lasagna)
    {
        for(auto& column : row)
        {
            for(auto& cell : column)
            {
                if(!cell._Widget || cell._SpanOccupied)
                    continue;

                cell._Widget->Update();
            }
        }
    }
}

void RetroFuturaGUI::Lasagna::ShowDebugLines(const bool show)
{
    _drawDebugLines = show;
}

void RetroFuturaGUI::Lasagna::applySize(const glm::vec3& size)
{
    _size = size;
    resizeCells();
    resizeAllWidgets();
    moveWidgets();
}


std::vector<std::string> RetroFuturaGUI::Lasagna::SetAxisDefinition(const AxisDefinition& axisDefinition)
{
    if(!axisDefinition.IsValid())
    {
        std::println("ERROR::LASAGNA::INVALID_AXIS_DEFINITION::{}", _name);
        return {};
    }

    if(axisDefinition._RowDefinition == _axisDefinition._RowDefinition
    && axisDefinition._ColumnDefinition == _axisDefinition._ColumnDefinition
    && axisDefinition._LayerDefinition == _axisDefinition._LayerDefinition)
        return {};

    const bool keepsTrackCounts
    {
        axisDefinition._RowDefinition.size() == _axisDefinition._RowDefinition.size()
        && axisDefinition._ColumnDefinition.size() == _axisDefinition._ColumnDefinition.size()
        && axisDefinition._LayerDefinition.size() == _axisDefinition._LayerDefinition.size()
    };
    _axisDefinition = axisDefinition;

    if(keepsTrackCounts) //only the sizes change, so the cells stay
    {
        resizeCells();
        resizeAllWidgets();
        moveWidgets();
        return {};
    }

    //New cells for the new tracks. The old ones stay until the next Interact, since a loop over them may be what called this
    _retiredCells.push_back(std::exchange(_lasagna, {}));
    buildCells();
    CellArray& previousCells { _retiredCells.back() };
    std::vector<std::string> removedNames {};

    for(uSize row = 0; row < previousCells.size(); ++row)
        for(uSize column = 0; column < previousCells[row].size(); ++column)
            for(uSize layer = 0; layer < previousCells[row][column].size(); ++layer)
            {
                LasagnaCell& cell { previousCells[row][column][layer] };

                if(!cell._Widget)
                    continue;

                const CellPlacement placement
                {
                    ._Row = static_cast<u32>(row),
                    ._Column = static_cast<u32>(column),
                    ._Layer = static_cast<u32>(layer),
                    ._SizingMode = cell._SizingMode,
                    ._RowSpan = cell._RowSpan,
                    ._ColSpan = cell._ColSpan,
                    ._LayerSpan = cell._LayerSpan
                };

                if(!isPlacementFree(placement)) //its cell or span reaches past the new last track
                {
                    removedNames.emplace_back(cell._Widget->GetName());
                    _removedWidgets.push_back(std::move(cell._Widget)); //kept alive until the next Interact, as RemoveWidget does
                    continue;
                }

                placeWidget(std::move(cell._Widget), placement);
                _lasagna[row][column][layer]._Show = cell._Show;
            }

    return removedNames;
}

const RetroFuturaGUI::AxisDefinition& RetroFuturaGUI::Lasagna::GetAxisDefinition() const
{
    return _axisDefinition;
}

std::vector<std::string> RetroFuturaGUI::Lasagna::SetColumnDefinition(std::vector<f32> columnDefinition)
{
    AxisDefinition axisDefinition { _axisDefinition };
    axisDefinition._ColumnDefinition = std::move(columnDefinition);
    return SetAxisDefinition(axisDefinition);
}

std::vector<std::string> RetroFuturaGUI::Lasagna::SetRowDefinition(std::vector<f32> rowDefinition)
{
    AxisDefinition axisDefinition { _axisDefinition };
    axisDefinition._RowDefinition = std::move(rowDefinition);
    return SetAxisDefinition(axisDefinition);
}

std::vector<std::string> RetroFuturaGUI::Lasagna::SetLayerDefinition(std::vector<f32> layerDefinition)
{
    AxisDefinition axisDefinition { _axisDefinition };
    axisDefinition._LayerDefinition = std::move(layerDefinition);
    return SetAxisDefinition(axisDefinition);
}

void RetroFuturaGUI::Lasagna::buildCells()
{
    _lasagna.resize(_axisDefinition._RowDefinition.size());

    for(std::vector<std::vector<LasagnaCell>>& row : _lasagna)
    {
        row.resize(_axisDefinition._ColumnDefinition.size());

        for(std::vector<LasagnaCell>& column : row)
            column.resize(_axisDefinition._LayerDefinition.size());
    }

    resizeCells();
}

void RetroFuturaGUI::Lasagna::applyPosition(const glm::vec3& position)
{
    _position = position;
    moveWidgets();
}

glm::vec3 RetroFuturaGUI::Lasagna::getTopLeftCorner() const
{
    // _position is the Lasagna's center, as it is for every other widget; cells run rightward and downward from the top-left
    return glm::vec3(_position.x - _size.x * 0.5f, _position.y + _size.y * 0.5f, _position.z);
}

void RetroFuturaGUI::Lasagna::drawDebugLines(const LasagnaCell& cell)
{
    if(!_debugBorder)
        return;

    const glm::vec3 topLeft { getTopLeftCorner() };

    _debugBorder->SetPosition(glm::vec3(topLeft.x + cell._PositionPixels.x + cell._SizePixels.x * 0.5f, topLeft.y - (cell._PositionPixels.y + cell._SizePixels.y * 0.5f), topLeft.z + cell._PositionPixels.z));
    _debugBorder->SetSize(glm::vec2(cell._SizePixels.x, cell._SizePixels.y));
    _debugBorder->Draw();
}

void RetroFuturaGUI::Lasagna::updateSpanSize(LasagnaCell& originCell, const uSize row, const uSize column, const uSize layer)
{
    glm::vec3
        sizePixels { 0.0f },
        sizeNormalized { 0.0f };

    for(u32 r = 0; r < originCell._RowSpan; ++r)
    {
        sizePixels.y += _lasagna[row + r][column][layer]._SizePixels.y;
        sizeNormalized.y += _lasagna[row + r][column][layer]._SizeNormalized.y;
    }

    for(u32 c = 0; c < originCell._ColSpan; ++c)
    {
        sizePixels.x += _lasagna[row][column + c][layer]._SizePixels.x;
        sizeNormalized.x += _lasagna[row][column + c][layer]._SizeNormalized.x;
    }

    for(u32 l = 0; l < originCell._LayerSpan; ++l)
    {
        sizePixels.z += _lasagna[row][column][layer + l]._SizePixels.z;
        sizeNormalized.z += _lasagna[row][column][layer + l]._SizeNormalized.z;
    }

    originCell._SizePixels = sizePixels;
    originCell._SizeNormalized = sizeNormalized;
}

f32 RetroFuturaGUI::Lasagna::getLayerDepth(const uSize layer) const
{
    if(layer >= _axisDefinition._LayerDefinition.size())
        return _minimumLayerDepth;

    const f32 share { _axisDefinition._LayerDefinition[layer] * _size.z };

    //A single layer has no neighbour to mix with, so it keeps its share, even none (a scene's root lies flat at depth 0)
    if(2 > _axisDefinition._LayerDefinition.size())
        return share;

    return std::max(share, _minimumLayerDepth);
}

void RetroFuturaGUI::Lasagna::resizeCells()
{
    for(uSize row = 0; row < _axisDefinition._RowDefinition.size(); ++row)
    {
        for(uSize column = 0; column < _axisDefinition._ColumnDefinition.size(); ++column)
        {
            for(uSize layer = 0; layer < _axisDefinition._LayerDefinition.size(); ++layer)
            {
                f32
                    posX { 0.0f },
                    posY { 0.0f },
                    posZ { 0.0f };

                for(uSize i = 0; i < column; ++i)
                    posX += _axisDefinition._ColumnDefinition[i] * _size.x;

                for(uSize i = 0; i < row; ++i)
                    posY += _axisDefinition._RowDefinition[i] * _size.y;

                for(uSize i = 0; i < layer; ++i)
                    posZ += getLayerDepth(i);

                posZ = -posZ; // Higher layer indices sit further from the camera (behind lower layers),

                _lasagna[row][column][layer]._PositionPixels = glm::vec3(posX, posY, posZ);
                _lasagna[row][column][layer]._PositionNormalized = glm::vec3(posX / _size.x, posY / _size.y, posZ / _size.z);
                _lasagna[row][column][layer]._SizePixels = glm::vec3(_axisDefinition._ColumnDefinition[column] * _size.x, _axisDefinition._RowDefinition[row] * _size.y, getLayerDepth(layer));
                _lasagna[row][column][layer]._SizeNormalized = glm::vec3(_axisDefinition._ColumnDefinition[column], _axisDefinition._RowDefinition[row], _axisDefinition._LayerDefinition[layer]);
            }
        }
    }

    // Cells that were attached with a span merge their base sizes across the spanned range,
    // so this must run after every cell above has its up-to-date single-cell size.
    for(uSize row = 0; row < _axisDefinition._RowDefinition.size(); ++row)
    {
        for(uSize column = 0; column < _axisDefinition._ColumnDefinition.size(); ++column)
        {
            for(uSize layer = 0; layer < _axisDefinition._LayerDefinition.size(); ++layer)
            {
                LasagnaCell& cell = _lasagna[row][column][layer];

                if(1 < cell._RowSpan || 1 < cell._ColSpan || 1 < cell._LayerSpan)
                    updateSpanSize(cell, row, column, layer);
            }
        }
    }
}

void RetroFuturaGUI::Lasagna::resizeWidget(const LasagnaCell& cell)
{
    const Transitions::InstantScope instant { true }; //a container lays its widgets out at once, whatever transitions they have
    if(!cell._Widget)
        return;

    switch(cell._SizingMode)
    {
        case SizingMode::FILL:
            cell._Widget->SetSize(cell._SizePixels);
        break;
        case SizingMode::FILL_X:
            cell._Widget->SetSize(glm::vec3(cell._SizePixels.x, cell._Widget->GetSize().y, cell._Widget->GetSize().z));
        break;
        case SizingMode::FILL_Y:
            cell._Widget->SetSize(glm::vec3(cell._Widget->GetSize().x, cell._SizePixels.y, cell._Widget->GetSize().z));
        break;
        case SizingMode::FILL_Z:
            cell._Widget->SetSize(glm::vec3(cell._Widget->GetSize().x, cell._Widget->GetSize().y, cell._SizePixels.z));
        break;
        case SizingMode::FILL_XY:
            cell._Widget->SetSize(glm::vec3(cell._SizePixels.x, cell._SizePixels.y, cell._Widget->GetSize().z));
        break;
        case SizingMode::FIXED:
        default:
        break;
    }
}

void RetroFuturaGUI::Lasagna::resizeAllWidgets()
{
    for(auto& row : _lasagna)
        for(auto& column : row)
            for(auto& cell : column)
                resizeWidget(cell);
}

void RetroFuturaGUI::Lasagna::moveWidgets()
{
    const Transitions::InstantScope instant { true }; //a container lays its widgets out at once, whatever transitions they have
    const glm::vec3 topLeft { getTopLeftCorner() };

    for(auto& row : _lasagna)
        for(auto& column : row)
            for(auto& cell : column)
            {
                if(!cell._Widget)
                    continue;

                cell._Widget->SetPosition(glm::vec3(topLeft.x + cell._PositionPixels.x + cell._SizePixels.x * 0.5f,
                                                    topLeft.y - cell._PositionPixels.y - cell._SizePixels.y * 0.5f,
                                                    topLeft.z + cell._PositionPixels.z + cell._SizePixels.z * 0.5f
                                                ));
            }
}