#include "TableColor.hpp"

RetroFuturaGUI::TableColor::TableColor(Table* parentTable, Projection* projection, Rectangle* rectangle)
    : ITableWidget(parentTable, projection), _rectangle(rectangle)
{
    _tableWidgetID = TableWidgetTypeID::TableColor;
}

const glm::vec4& RetroFuturaGUI::TableColor::GetColor() const
{
    if(_color.empty())
        return _dummyColor;

    return _color.front();
}

void RetroFuturaGUI::TableColor::SetColor(const glm::vec4& color)
{
    if(_color.empty())
        _color.push_back(color);
    else
        _color.front() = color;
}

void RetroFuturaGUI::TableColor::Draw()
{
    if(!_rectangle || _color.empty())
        return;

    _rectangle->SetColors(_color);
    _rectangle->SetSize(_size);
    _rectangle->SetPosition(_position);
    _rectangle->SetRotation(_rotation);
    _rectangle->Draw();
}