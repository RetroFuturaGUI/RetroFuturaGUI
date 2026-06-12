#include "Lasagna.hpp"

RetroFuturaGUI::Lasagna::Lasagna(const std::string& name, Projection* projection, IWidget* parentWidget, const WidgetTypeID parentWidgetTypeID, GLFWwindow* parentWindow, AxisDefinition* _axisDefinition)
    : IWidget(name, projection, parentWidget, parentWidgetTypeID, parentWindow), _axisdefinition(*_axisDefinition)
{
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

                for(uSize i = 0; i < row; ++i)
                    posX += _axisdefinition._RowDefinition[i];

                for(uSize i = 0; i < column; ++i)
                    posY += _axisdefinition._ColumnDefinition[i];

                for(uSize i = 0; i < layer; ++i)
                    posZ += _axisdefinition._LayerDefinition[i];

                _lasagna[row][column].push_back(LasagnaCell
                    {
                        ._PositionPixels = glm::vec3(_projection.GetResolution().x * posX, _projection.GetResolution().y * posY, _projection.GetDepth() * posZ),
                        ._PositionNormalized = glm::vec3(posX, posY, posZ),
                        ._SizePixels = glm::vec3(_projection.GetResolution().x * _axisdefinition._RowDefinition[row],
                                                 _projection.GetResolution().y * _axisdefinition._ColumnDefinition[column],
                                                 _projection.GetDepth() * _axisdefinition._LayerDefinition[layer]
                                                ),
                        ._SizeNormalized = glm::vec3(_axisdefinition._RowDefinition[row], _axisdefinition._RowDefinition[column], _axisdefinition._LayerDefinition[layer]),
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
        _debugBorder->SetRotation(0.0f);
        _debugBorder->SetColors(_debugBorderColor);
        _debugBorder->SetBorderWidth(5.0f);
    }

}

void RetroFuturaGUI::Lasagna::AttachWidget(const u32 row, const u32 col, const u32 layer, IWidget* widget, const SizingMode sizingMode)
{
    if(_axisdefinition._RowDefinition.size() < row
    || _axisdefinition._ColumnDefinition.size() < col
    || _axisdefinition._LayerDefinition.size() < layer)
        return;
        
    if(_lasagna[row][col][layer]._SpanOccupied)
        return;

    _lasagna[row][col][layer]._Widget = widget;
    _lasagna[row][col][layer]._Widget->SetPosition(glm::vec3(_lasagna[row][col][layer]._PositionPixels.x + _lasagna[row][col][layer]._SizePixels.x * 0.5f,
                                                            _projection.GetResolution().y - _lasagna[row][col][layer]._PositionPixels.y - _lasagna[row][col][layer]._SizePixels.y * 0.5f,
                                                            _lasagna[row][col][layer]._PositionPixels.z + _lasagna[row][col][layer]._SizePixels.z * 0.5f
                                                        ));
    _lasagna[row][col][layer]._SizingMode = sizingMode;
    resizeWidget(_lasagna[row][col][layer]);
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

    _debugBorder->SetPosition(glm::vec2(cell._PositionPixels.x + cell._SizePixels.x * 0.5f, _projection.GetResolution().y - (cell._PositionPixels.y + cell._SizePixels.y * 0.5f)));
    _debugBorder->SetSize(glm::vec2(cell._SizePixels.x, cell._SizePixels.y));
    _debugBorder->Draw();
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

                for(uSize i = 0; i < row; ++i)
                    posX += _axisdefinition._RowDefinition[i] * _size.x;

                for(uSize i = 0; i < column; ++i)
                    posY += _axisdefinition._ColumnDefinition[i] * _size.y;

                for(uSize i = 0; i < layer; ++i)
                    posZ += _axisdefinition._LayerDefinition[i] * _size.z;

                _lasagna[row][column][layer]._PositionPixels = glm::vec3(posX, posY, posZ);
                _lasagna[row][column][layer]._PositionNormalized = glm::vec3(posX / _size.x, posY / _size.y, posZ / _size.z);
                _lasagna[row][column][layer]._SizePixels = glm::vec3(_axisdefinition._RowDefinition[row] * _size.x, _axisdefinition._ColumnDefinition[column] * _size.y, _axisdefinition._LayerDefinition[layer] * _size.z);
                _lasagna[row][column][layer]._SizeNormalized = glm::vec3(_axisdefinition._RowDefinition[row], _axisdefinition._ColumnDefinition[column], _axisdefinition._LayerDefinition[layer]);
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