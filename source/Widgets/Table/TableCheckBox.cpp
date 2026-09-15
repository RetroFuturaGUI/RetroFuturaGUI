#include "TableCheckBox.hpp"
#include "ITableWidget.hpp"


RetroFuturaGUI::TableCheckBox::TableCheckBox(Table* parentTable, Projection* projection, Rectangle* background, Rectangle* border, SvgTexture* checkmark)
    : ITableWidget(parentTable, projection)
{
    _tableWidgetID = TableWidgetTypeID::TableCheckBox;
    _background = background;
    _border = border;
    _checkmark = checkmark;
}


void RetroFuturaGUI::TableCheckBox::Draw()
{
    if(_background)
    {
        _background->SetSize(_size);
        _background->SetPosition(_position);
        _background->SetRotation(_rotation);
        _background->Draw();
    }

    if(_border)
    {
        _border->SetSize(_size);
        _border->SetPosition(_position + glm::vec3(0.0f, 0.0f, 0.01f));
        _border->SetRotation(_rotation);
        _border->Draw();
    }

    if(!_isChecked)
        return;

    if(_checkmark)
    {
        _checkmark->SetSize(_size);
        _checkmark->SetPosition(_position + glm::vec3(0.0f, 0.0f, 0.02f));
        _checkmark->SetRotation(_rotation);
        _checkmark->Draw();
    }
}

void RetroFuturaGUI::TableCheckBox::SetSize(const glm::vec3& size)
{
    const f32
        shortest { size.x < size.y ? size.x : size.y },
        extent { shortest > _cellMargin * 2.0f ? shortest - _cellMargin * 2.0f : 0.0f };

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

void RetroFuturaGUI::TableCheckBox::SetCellMargin(const f32 margin)
{
    _cellMargin = margin > 0.0f ? margin : 0.0f;
}

void RetroFuturaGUI::TableCheckBox::SetValue(const bool value)
{
    _isChecked = value;
}

bool RetroFuturaGUI::TableCheckBox::GetValue() const
{
    return _isChecked;
}