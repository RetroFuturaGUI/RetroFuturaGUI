#include "ITableWidget.hpp"

RetroFuturaGUI::ITableWidget::ITableWidget(Table* parentTable, Projection* projection)
    : _parentTable(parentTable), _projection(projection)
{

}

void RetroFuturaGUI::ITableWidget::SetSize(const glm::vec3& size)
{
    _size = size;
}

void RetroFuturaGUI::ITableWidget::SetPosition(const glm::vec3& position)
{
    _position = position;
}

void RetroFuturaGUI::ITableWidget::SetRotation(const glm::vec3& rotation)
{
    _rotation = rotation;
}

glm::vec3 RetroFuturaGUI::ITableWidget::GetSize() const
{
    return _size;
}

glm::vec3 RetroFuturaGUI::ITableWidget::GetPosition() const
{
    return _position;
}

glm::vec3 RetroFuturaGUI::ITableWidget::GetRotation() const
{
    return _rotation;
}