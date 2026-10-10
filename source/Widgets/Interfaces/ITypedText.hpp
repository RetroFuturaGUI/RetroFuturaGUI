#pragma once
#include "TextTypes.hpp"
#include <memory>
#include <string_view>

namespace RetroFuturaGUI
{
    //An interface binding a typed value to a widget's displayed text
    class ITypedText
    {
    public:
        /// @brief Sets the value and renders it in the current numeric base and precision.
        template<TextValueType T>
        void SetValue(const T value, const bool emitSignal = true)
        {
            ensureValueStore();
            _valueStore->SetValue(value);
            renderValueText(_valueStore->GetValueText(), emitSignal);
        }

        /// @brief Returns the value converted to T, or a value-initialized T while there is no value store.
        template<NumericValueType T>
        T GetValue() const
        {
            if(!_valueStore)
                return T {};

            return _valueStore->GetValue<T>();
        }

        /// @brief Returns whether a value has been set or the text edited.
        bool HasValueStore() const { return _valueStore != nullptr; }

        /// @brief Sets the numeric base used to render the value and re-renders the text.
        void SetNumericBase(const u32 base, const bool emitSignal = true);

        /// @brief Sets how many decimal digits floats render with, negative for full precision, and re-renders the text.
        void SetDecimalPrecision(const i32 precision, const bool emitSignal = true);

        /// @brief Converts the stored value to another data type and re-renders the text.
        void ChangeType(const TextTypes::DataTypeID id, const bool emitSignal = true);

    protected:
        /// @brief Shows the given text in the widget without going back through the value store.
        virtual void renderValueText(std::string_view text, const bool emitSignal) = 0;

        /// @brief Stores edited text as the value, so GetValue parses what was typed. Creates the store if needed.
        void syncValueFromText(std::string_view text);

        /// @brief Stores text that was set verbatim, but only if a store already exists.
        void updateValueStore(std::string_view text);

        void ensureValueStore();

        std::unique_ptr<TextTypes> _valueStore {};
    };
}
