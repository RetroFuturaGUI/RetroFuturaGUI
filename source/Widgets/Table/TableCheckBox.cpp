#include "TableCheckBox.hpp"
#include "ITableWidget.hpp"


RetroFuturaGUI::TableCheckBox::TableCheckBox(Table* parentTable, Projection* projection, Plane* background, Frame* border, SvgTexture* checkmark)
    : ITableWidget(parentTable, projection)
{
    _tableWidgetID = TableWidgetTypeID::TableCheckBox;
    _background = background;
    _border = border;
    _checkmark = checkmark;
}


void RetroFuturaGUI::TableCheckBox::Draw()
{
    if(!_background)
        return;

    if(!_border)
        return;

    _background->SetSize(_size);
    _background->SetPosition(_position);
    _background->SetRotation(_rotation);
    _background->Draw();

    _border->SetSize(_size);
    _border->SetPosition(_position + glm::vec3(0.0f, 0.0f, 0.01f));
    _border->SetRotation(_rotation);
    _border->Draw();

    if(!_isChecked)
        return;

    if(!_checkmark)
        return;

    const f32
        inset { _border->GetBorderWidth() + _innerPadding },
        extent { _size.x > 2.0f * inset ? _size.x - 2.0f * inset : 0.0f };

    //One checkmark for every cell, so it moves to this cell. SvgTexture rebuilds a matrix only for a value that changed
    _checkmark->SetSize(glm::vec2(extent));
    _checkmark->SetPosition(_position + glm::vec3(0.0f, 0.0f, 0.02f));
    _checkmark->SetRotation(_rotation);
    _checkmark->Draw();
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