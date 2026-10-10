#include "ITypedText.hpp"

void RetroFuturaGUI::ITypedText::SetNumericBase(const u32 base, const bool emitSignal)
{
    ensureValueStore();
    _valueStore->SetNumericBase(base);

    if(_valueStore->GetDataType() != TextTypes::DataTypeID::Text) //text reads the same in every base
        renderValueText(_valueStore->GetValueText(), emitSignal);
}

void RetroFuturaGUI::ITypedText::SetDecimalPrecision(const i32 precision, const bool emitSignal)
{
    ensureValueStore();
    _valueStore->SetDecimalPrecision(precision);

    const TextTypes::DataTypeID dataType { _valueStore->GetDataType() };

    if(dataType == TextTypes::DataTypeID::Float32 || dataType == TextTypes::DataTypeID::Float64) //only floats have decimals
        renderValueText(_valueStore->GetValueText(), emitSignal);
}

void RetroFuturaGUI::ITypedText::ChangeType(const TextTypes::DataTypeID id, const bool emitSignal)
{
    ensureValueStore();
    _valueStore->ChangeType(id);
    renderValueText(_valueStore->GetValueText(), emitSignal);
}

void RetroFuturaGUI::ITypedText::syncValueFromText(std::string_view text)
{
    ensureValueStore();
    _valueStore->SetValue(text);
}

void RetroFuturaGUI::ITypedText::updateValueStore(std::string_view text)
{
    if(!_valueStore)
        return;

    _valueStore->SetValue(text);
}

void RetroFuturaGUI::ITypedText::ensureValueStore()
{
    if(!_valueStore)
        _valueStore = std::make_unique<TextTypes>();
}
