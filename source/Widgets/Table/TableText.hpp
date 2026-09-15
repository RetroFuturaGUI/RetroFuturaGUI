#pragma once
#include "ITableWidget.hpp"
#include "ITableTextProperties.hpp"
#include "ITextTypes.hpp"
#include "IncludeHelper.hpp"
#include <memory>

namespace RetroFuturaGUI
{
    class TableText final : public ITableWidget, public ITableTextProperties
    {
        public:
            TableText(Table* parentTable, Projection* projection);
            TableText() = delete;

            void Draw() override;
            void SetSize(const glm::vec3& size) override;
            void SetPosition(const glm::vec3& position) override;
            void SetRotation(const glm::vec3& rotation) override;
            /// @brief Sets the cell's text verbatim. Does not allocate a value store; updates one only if it exists.
            void SetText(std::string_view text) override;
            void SetTextColors(std::span<glm::vec4> colors, const ColorState colorState);

        //Typed values
            /// @brief Sets the cell's value and renders it in the cell's numeric base and precision.
            template<NumericValueType T>
            void SetValue(const T value)
            {
                ensureValueStore();
                _valueStore->SetValue(value);
                setRenderedText(_valueStore->GetValueText()); //not SetText: that would re-type the value as Text
            }

            /// @brief Returns the cell's value converted to T, or a value-initialized T if the cell has no value store.
            template<NumericValueType T>
            T GetValue() const
            {
                return _valueStore ? _valueStore->GetValue<T>() : T {};
            }

            /// @brief Returns whether this cell has been given a typed value.
            bool HasValueStore() const { return _valueStore != nullptr; }

            /// @brief Sets the numeric base used to render the value and re-renders the cell text.
            void SetNumericBase(const u32 base);

            /// @brief Sets how many decimal digits floats render with, negative for full precision.
            void SetDecimalPrecision(const i32 precision);

            /// @brief Converts the stored value to another data type and re-renders the cell text.
            void ChangeType(const ITextTypes::DataTypeID id);

            /// @brief Mirrors edited cell text into the value store. A no-op for cells without one.
            void syncValueFromText();

        private:
            void ensureValueStore();

            /// @brief Writes the rendered text without touching the value store.
            void setRenderedText(std::string_view text);

            std::unique_ptr<ITextTypes> _valueStore {};
    };
}