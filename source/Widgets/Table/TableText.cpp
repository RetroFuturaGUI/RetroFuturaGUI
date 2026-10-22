#include "TableText.hpp"

RetroFuturaGUI::TableText::TableText(Table* parentTable, Projection* projection)
    : ITableWidget(parentTable, projection)
{
    _tableWidgetID = TableWidgetTypeID::TableText;
}

void RetroFuturaGUI::TableText::Draw()
{
    ensureText();

    if(!_text)
        return;

    _text->Draw();
}

void RetroFuturaGUI::TableText::ensureText()
{
    if(_text)
        return;

    _text = std::make_unique<Text>(_projection);

    if(!_text)
        return;

    applyTextSettings();
    _text->SetParentSize(glm::vec2(_size.x, _size.y));
    _text->SetPosition(_position);
    _text->SetRotation(_rotation);
    _text->SetTextUTF8(_content);
}

void RetroFuturaGUI::TableText::SetSize(const glm::vec3& size)
{
    ITableWidget::SetSize(size);

    if(_text)
        _text->SetParentSize(glm::vec2(size.x, size.y));
}

void RetroFuturaGUI::TableText::SetPosition(const glm::vec3& position)
{
    ITableWidget::SetPosition(position);

    if(_text)
        _text->SetPosition(position);
}

void RetroFuturaGUI::TableText::SetRotation(const glm::vec3& rotation)
{
    ITableWidget::SetRotation(rotation);

    if(_text)
        _text->SetRotation(rotation);
}

void RetroFuturaGUI::TableText::renderValueText(std::string_view text, const bool)
{
    _content = text;

    if(!_text)
        return;

    _text->SetTextUTF8(text);
}

void RetroFuturaGUI::TableText::SetText(std::string_view text)
{
    renderValueText(text, false);
    updateValueStore(text);
}

void RetroFuturaGUI::TableText::SetTextColors(std::span<glm::vec4> colors, const ColorState colorState)
{
    switch(colorState)
    {
        case ColorState::Disabled:
            _textColorDisabled = colors;
        break;
        case ColorState::Clicked:
            _textColorClicked = colors;
        break;
        case ColorState::Hover:
            _textColorHover = colors;
        break;
        default: //Enabled
            _textColorEnabled = colors;
    }

    setTextColors();
}