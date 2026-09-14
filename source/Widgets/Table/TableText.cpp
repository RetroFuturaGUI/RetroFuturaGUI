#include "TableText.hpp"

RetroFuturaGUI::TableText::TableText(Table* parentTable, Projection* projection)
    : ITableWidget(parentTable, projection)
{
    _tableWidgetID = TableWidgetTypeID::TableText;
    _text = std::make_unique<Text>(projection);
}

void RetroFuturaGUI::TableText::Draw()
{
    if(!_text)
        return;

    _text->Draw();
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

void RetroFuturaGUI::TableText::setRenderedText(std::string_view text)
{
    if(_text)
        _text->SetTextUTF8(text);
}

void RetroFuturaGUI::TableText::SetText(std::string_view text)
{
    setRenderedText(text);

    if(_valueStore)
        _valueStore->SetValue(text);
}

void RetroFuturaGUI::TableText::ensureValueStore()
{
    if(!_valueStore)
        _valueStore = std::make_unique<ITextTypes>();
}

void RetroFuturaGUI::TableText::SetNumericBase(const u32 base)
{
    ensureValueStore();
    _valueStore->SetNumericBase(base);
    setRenderedText(_valueStore->GetValueText());
}

void RetroFuturaGUI::TableText::SetDecimalPrecision(const i32 precision)
{
    ensureValueStore();
    _valueStore->SetDecimalPrecision(precision);
    setRenderedText(_valueStore->GetValueText());
}

void RetroFuturaGUI::TableText::ChangeType(const ITextTypes::DataTypeID id)
{
    ensureValueStore();
    _valueStore->ChangeType(id);
    setRenderedText(_valueStore->GetValueText());
}

void RetroFuturaGUI::TableText::syncValueFromText()
{
    if(!_text)
        return;

    ensureValueStore();
    _valueStore->SetValue(std::string_view(_text->GetTextUTF8()));
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