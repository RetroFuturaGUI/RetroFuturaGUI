#include "TableCheckBox.hpp"
#include "ITableWidget.hpp"


RetroFuturaGUI::TableCheckBox::TableCheckBox(Table* parentTable, Projection* projection, CellCheckBox* style)
    : ITableWidget(parentTable, projection), _style(style)
{
    _tableWidgetID = TableWidgetTypeID::TableCheckBox;
}


void RetroFuturaGUI::TableCheckBox::Draw()
{
    if(!_style)
        return;

    Background& background { _style->GetBackground() };
    background.SetSize(glm::vec2(_size));
    background.SetPosition(_position);
    background.SetRotation(_rotation);
    background.Draw();

    Border& border { _style->GetBorder() };
    border.SetSize(glm::vec2(_size));
    border.SetPosition(_position + glm::vec3(0.0f, 0.0f, 0.01f));
    border.SetRotation(_rotation);
    border.Draw();

    if(!_isChecked)
        return;

    const f32
        inset { border.GetWidth() + _innerPadding },
        extent { _size.x > 2.0f * inset ? _size.x - 2.0f * inset : 0.0f };

    //One checkmark for every cell, so it moves to this cell. SvgTexture rebuilds a matrix only for a value that changed
    Icon& checkmark { _style->GetCheckmark() };
    checkmark.SetSize(glm::vec2(extent));
    checkmark.SetPosition(_position + glm::vec3(0.0f, 0.0f, 0.02f));
    checkmark.SetRotation(_rotation);
    checkmark.Draw();
}

void RetroFuturaGUI::TableCheckBox::SetSize(const glm::vec3& size)
{
    const f32
        shortest { size.x < size.y ? size.x : size.y },
        extent { shortest > _padding * 2.0f ? shortest - _padding * 2.0f : 0.0f };

    ITableWidget::SetSize(glm::vec3(extent, extent, size.z));
}

void RetroFuturaGUI::TableCheckBox::SetPosition(const glm::vec3& position)
{
    ITableWidget::SetPosition(position);
}

void RetroFuturaGUI::TableCheckBox::SetRotation(const glm::vec3& rotation)
{
    ITableWidget::SetRotation(rotation);
}

void RetroFuturaGUI::TableCheckBox::SetPadding(const f32 padding)
{
    _padding = 0.0f < padding ? padding : 0.0f;
}

void RetroFuturaGUI::TableCheckBox::SetInnerPadding(const f32 padding)
{
    _innerPadding = 0.0f < padding ? padding : 0.0f;
}

void RetroFuturaGUI::TableCheckBox::SetValue(const bool value)
{
    _isChecked = value;
}

bool RetroFuturaGUI::TableCheckBox::GetValue() const
{
    return _isChecked;
}