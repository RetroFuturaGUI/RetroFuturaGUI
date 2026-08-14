#include "Lasagna.hpp"

RetroFuturaGUI::Lasagna::Lasagna(const std::string& name, Projection* projection, IWidget* parentWidget, const WidgetTypeID parentWidgetTypeID, GLFWwindow* parentWindow, AxisDefinition* _axisDefinition)
    : IWidget(name, projection, parentWidget, parentWidgetTypeID, parentWindow), _axisdefinition(*_axisDefinition)
{
    _size.z = _projection.GetDepth(); // keeps feeding a real depth back in, instead of latching to 0 on the first resize.
    _lasagna.reserve(_maxCountPerAxis);

    for(uSize row = 0; row < _maxCountPerAxis; ++row)
    {
        _lasagna.emplace_back();
        _lasagna.back().reserve(_maxCountPerAxis);

        for(uSize column = 0; column < _maxCountPerAxis; ++column)
        {
            _lasagna.back().emplace_back();
            _lasagna.back().back().reserve(_maxCountPerAxis);
        }
    }

    for(uSize row = 0; row < _axisdefinition._RowDefinition.size(); ++row)
    {
        for(uSize column = 0; column < _axisdefinition._ColumnDefinition.size(); ++column)
        {
            for(uSize layer = 0; layer < _axisdefinition._LayerDefinition.size(); ++layer)
            {
                f32
                    posX { 0.0f },
                    posY { 0.0f },
                    posZ { 0.0f };

                for(uSize i = 0; i < column; ++i)
                    posX += _axisdefinition._ColumnDefinition[i];

                for(uSize i = 0; i < row; ++i)
                    posY += _axisdefinition._RowDefinition[i];

                for(uSize i = 0; i < layer; ++i)
                    posZ += _axisdefinition._LayerDefinition[i];

                posZ = -posZ; // Higher layer indices sit further from the camera (behind lower layers),

                _lasagna[row][column].push_back(LasagnaCell
                    {
                        ._PositionPixels = glm::vec3(_projection.GetResolution().x * posX, _projection.GetResolution().y * posY, _projection.GetDepth() * posZ),
                        ._PositionNormalized = glm::vec3(posX, posY, posZ),
                        ._SizePixels = glm::vec3(_projection.GetResolution().x * _axisdefinition._ColumnDefinition[column],
                                                 _projection.GetResolution().y * _axisdefinition._RowDefinition[row],
                                                 _projection.GetDepth() * _axisdefinition._LayerDefinition[layer]
                                                ),
                        ._SizeNormalized = glm::vec3(_axisdefinition._ColumnDefinition[column], _axisdefinition._RowDefinition[row], _axisdefinition._LayerDefinition[layer]),
                        ._PaddingPixels = glm::vec3(0.0f),
                        ._PaddingNormalized = glm::vec3(0.0f),
                        ._RowSpan = 1,
                        ._ColSpan = 1,
                        ._LayerSpan = 1,
                        ._Widget = nullptr,
                        ._SpanOccupied = false,
                        ._SizingMode = SizingMode::FILL
                    }
                );
            }
        }
    }

    _debugBorder = std::make_unique<Rectangle>(&_projection);

    if(_debugBorder)
    {
        _debugBorder->SetRectangleMode(RectangleMode::Border);
        _debugBorder->SetPosition(glm::vec3(0.0f));
        _debugBorder->SetSize(glm::vec3(0.0f));
        _debugBorder->SetRotation(glm::vec3(0.0f));
        _debugBorder->SetColors(_debugBorderColor);
        _debugBorder->SetBorderWidth(5.0f);
    }

}

void RetroFuturaGUI::Lasagna::AttachWidget(const u32 row, const u32 col, const u32 layer, IWidget* widget, const SizingMode sizingMode, const u32 rowSpan, const u32 colSpan, const u32 layerSpan)
{
    const u32
        rowSpanClamped = rowSpan == 0 ? 1 : rowSpan,
        colSpanClamped = colSpan == 0 ? 1 : colSpan,
        layerSpanClamped = layerSpan == 0 ? 1 : layerSpan;

    if(_axisdefinition._RowDefinition.size() < row + rowSpanClamped
    || _axisdefinition._ColumnDefinition.size() < col + colSpanClamped
    || _axisdefinition._LayerDefinition.size() < layer + layerSpanClamped)
        return;

    for(u32 r = row; r < row + rowSpanClamped; ++r)
        for(u32 c = col; c < col + colSpanClamped; ++c)
            for(u32 l = layer; l < layer + layerSpanClamped; ++l)
                if(_lasagna[r][c][l]._SpanOccupied || _lasagna[r][c][l]._Widget != nullptr)
                    return;

    LasagnaCell& origin = _lasagna[row][col][layer];
    origin._RowSpan = rowSpanClamped;
    origin._ColSpan = colSpanClamped;
    origin._LayerSpan = layerSpanClamped;
    origin._SizingMode = sizingMode;
    origin._Widget = widget;

    for(u32 r = row; r < row + rowSpanClamped; ++r)
        for(u32 c = col; c < col + colSpanClamped; ++c)
            for(u32 l = layer; l < layer + layerSpanClamped; ++l)
                if(r != row || c != col || l != layer)
                    _lasagna[r][c][l]._SpanOccupied = true;

    updateSpanSize(origin, row, col, layer);

    origin._Widget->SetPosition(glm::vec3(origin._PositionPixels.x + origin._SizePixels.x * 0.5f,
                                          _projection.GetResolution().y - origin._PositionPixels.y - origin._SizePixels.y * 0.5f,
                                          origin._PositionPixels.z + origin._SizePixels.z * 0.5f
                                        ));
    resizeWidget(origin);
}

void RetroFuturaGUI::Lasagna::Draw(const bool alsoDrawDebugLines)
{
    for(auto& row : _lasagna)
    {
        for(auto& column : row)
        {
            for(auto& cell : column)
            {
                if(alsoDrawDebugLines)
                    drawDebugLines(cell);

                if(cell._Widget == nullptr || cell._SpanOccupied)
                    continue;

                cell._Widget->Draw();
            }
        }
    }
}

void RetroFuturaGUI::Lasagna::SetSize(const glm::vec3& size)
{
    _size = size;
    resizeCells();
    resizeAllWidgets();
    moveWidgets();
}

void RetroFuturaGUI::Lasagna::SetPosition(const glm::vec3& position)
{
    _position = position;
    resizeCells();
    resizeAllWidgets();
}

void RetroFuturaGUI::Lasagna::drawDebugLines(const LasagnaCell& cell)
{
    if(!_debugBorder)
        return;

    _debugBorder->SetPosition(glm::vec3(cell._PositionPixels.x + cell._SizePixels.x * 0.5f, _projection.GetResolution().y - (cell._PositionPixels.y + cell._SizePixels.y * 0.5f), cell._PositionPixels.z));
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

void RetroFuturaGUI::Lasagna::resizeCells()
{
    for(uSize row = 0; row < _axisdefinition._RowDefinition.size(); ++row)
    {
        for(uSize column = 0; column < _axisdefinition._ColumnDefinition.size(); ++column)
        {
            for(uSize layer = 0; layer < _axisdefinition._LayerDefinition.size(); ++layer)
            {
                f32
                    posX { 0.0f },
                    posY { 0.0f },
                    posZ { 0.0f };

                for(uSize i = 0; i < column; ++i)
                    posX += _axisdefinition._ColumnDefinition[i] * _size.x;

                for(uSize i = 0; i < row; ++i)
                    posY += _axisdefinition._RowDefinition[i] * _size.y;

                for(uSize i = 0; i < layer; ++i)
                    posZ += _axisdefinition._LayerDefinition[i] * _size.z;

                posZ = -posZ; // Higher layer indices sit further from the camera (behind lower layers),

                _lasagna[row][column][layer]._PositionPixels = glm::vec3(posX, posY, posZ);
                _lasagna[row][column][layer]._PositionNormalized = glm::vec3(posX / _size.x, posY / _size.y, posZ / _size.z);
                _lasagna[row][column][layer]._SizePixels = glm::vec3(_axisdefinition._ColumnDefinition[column] * _size.x, _axisdefinition._RowDefinition[row] * _size.y, _axisdefinition._LayerDefinition[layer] * _size.z);
                _lasagna[row][column][layer]._SizeNormalized = glm::vec3(_axisdefinition._ColumnDefinition[column], _axisdefinition._RowDefinition[row], _axisdefinition._LayerDefinition[layer]);
            }
        }
    }

    // Cells that were attached with a span merge their base sizes across the spanned range,
    // so this must run after every cell above has its up-to-date single-cell size.
    for(uSize row = 0; row < _axisdefinition._RowDefinition.size(); ++row)
    {
        for(uSize column = 0; column < _axisdefinition._ColumnDefinition.size(); ++column)
        {
            for(uSize layer = 0; layer < _axisdefinition._LayerDefinition.size(); ++layer)
            {
                LasagnaCell& cell = _lasagna[row][column][layer];

                if(cell._RowSpan > 1 || cell._ColSpan > 1 || cell._LayerSpan > 1)
                    updateSpanSize(cell, row, column, layer);
            }
        }
    }
}

void RetroFuturaGUI::Lasagna::resizeWidget(const LasagnaCell& cell)
{
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
    for(auto& row : _lasagna)
        for(auto& column : row)
            for(auto& cell : column)
            {
                if(!cell._Widget)
                    continue;

                cell._Widget->SetPosition(glm::vec3(cell._PositionPixels.x + cell._SizePixels.x * 0.5f,
                                                    _projection.GetResolution().y - cell._PositionPixels.y - cell._SizePixels.y * 0.5f,
                                                    cell._PositionPixels.z + cell._SizePixels.z * 0.5f
                                                ));
            }
}