
#pragma once
#include "ITextProperties.hpp"
#include "ITextInteraction.hpp"
#include <memory>
#include "ITextTypes.hpp"

namespace RetroFuturaGUI
{
    //An interface to specialize a widget with editable text capabilities.
    class ITextEditable : virtual public IWindowAccessor, public ITextProperties, public ITextInteraction, public ITextTypes
    {
    public:
        /// @brief Sets whether the widget rejects text input/editing while still allowing selection and copy.
        void SetReadOnly(const bool readOnly);

        /// @brief Returns whether the widget is currently read-only.
        bool IsReadOnly() const;

        /// @brief Sets the caret color(s).
        void SetCaretColors(std::span<glm::vec4> colors);

        /// @brief Sets the caret fill type (solid, linear/radial/huestar gradient).
        void SetCaretFillType(const FillType fillType);

        /// @brief Sets the speed at which the caret's gradient animates over time.
        void SetCaretGradientAnimationSpeed(const f32 speed);

        /// @brief Sets how long, in milliseconds, the caret stays visible/hidden per blink cycle.
        void SetCaretBlinkTime(const f64 milliseconds);

        /// @brief Sets the selection highlight color(s).
        void SetSelectedAreaColors(std::span<glm::vec4> colors);

        /// @brief Sets the selection highlight fill type (solid, linear/radial/huestar gradient).
        void SetSelectedAreaFillType(const FillType fillType);

        /// @brief Sets the speed at which the selection highlight's gradient animates over time.
        void SetSelectedAreaGradientAnimationSpeed(const f32 speed);

        /// @brief Sets the offset applied to the selection highlight gradient's start position.
        void SetSelectedAreaGradientOffset(const f32 gradientOffset);

        /// @brief Sets the angle of the selection highlight's linear gradient, in degrees.
        void SetSelectedAreaGradientDegree(const f32 degree);

        /// @brief Sets the speed at which the selection highlight's gradient rotates over time.
        void SetSelectedAreaGradientRotationSpeed(const f32 rotationSpeed);

        /// @brief Sets the corner rounding radii of the selection highlight.
        void SetSelectedAreaCornerRadii(const glm::vec4& radii);


        /// @brief Sets the placeholder text color for the given color state.
        void SetPlaceholderTextColor(const glm::vec4& color);

        /// @brief Sets the placeholder text content, in UTF-8.
        void SetPlaceholderText(std::string_view text);

        /// @brief Returns the placeholder text content, in UTF-8.
        const std::string& GetPlaceholderText() const;

        /// @brief Sets the font family, size and style used to render the text and placeholder text, loading it if necessary.
        void SetFontFamily(std::string_view fontFamily, const f32 fontSize, const PlatformBridge::Fonts::Slant slant, const PlatformBridge::Fonts::Weight fontWeight) override;

        /// @brief Sets the horizontal alignment of the text and placeholder text.
        void SetTextAlignment(const TextAlignment alignment) override;

        /// @brief Sets the padding applied around the text and placeholder text.
        void SetTextPadding(const f32 padding) override;

    private:
        using ITextProperties::SetText;

    public:

        /// @brief Sets the widget's value and updates the displayed text to match.
        template<TextValueType T>
        void SetValue(const T value, const bool emitSignal = true)
        {
            ITextTypes::SetValue(value);
            ITextProperties::SetText(ITextTypes::GetValueText(), emitSignal);
        }

        /// @brief Sets the numeric base used to render the value and re-renders the displayed text.
        void SetNumericBase(const u32 base, const bool emitSignal = true)
        {
            ITextTypes::SetNumericBase(base);

            if(_currentDataType != DataTypeID::Text)
                ITextProperties::SetText(ITextTypes::GetValueText(), emitSignal);
        }

        /// @brief Sets how many decimal digits floats render with, negative for full precision, and re-renders the displayed text.
        void SetDecimalPrecision(const i32 precision, const bool emitSignal = true)
        {
            ITextTypes::SetDecimalPrecision(precision);

            if(_currentDataType == DataTypeID::Float32 || _currentDataType == DataTypeID::Float64)
                ITextProperties::SetText(ITextTypes::GetValueText(), emitSignal);
        }

        /// @brief Converts the stored value to another data type and re-renders the displayed text.
        void ChangeType(const DataTypeID id, const bool emitSignal = true)
        {
            ITextTypes::ChangeType(id);
            ITextProperties::SetText(ITextTypes::GetValueText(), emitSignal);
        }

    protected:
    //ITextInteraction hooks: this widget edits the single Text it owns, and draws its own caret and selection.
        Text* activeText() const override { return _text.get(); }
        void updateCaretPosition() override;
        void updateSelectedArea() override;
        void emitChange() override;
        bool isTextReadOnly() const override { return _readOnly; }

        /// @brief Mirrors the edited text into ITextTypes::_valueText so GetValue/GetValueText see what the user typed.
        void syncValueFromText();
        void drawSelectedArea();
        virtual f32 clampToTextBounds(const f32 worldX, const f32 = 0.0f) const { return worldX; }
        virtual f32 keepCaretVisible(const f32 worldX, const f32 halfExtent = 0.0f) { return clampToTextBounds(worldX, halfExtent); }

        //Caret
        std::unique_ptr<Rectangle> _caret;
        std::vector<glm::vec4> _caretColors { glm::vec4(1.0f) };

        //input logic
        bool _readOnly { false };
        std::vector<char> _prevKeyStates {};

        //Selection
        std::unique_ptr<Rectangle> _selectedArea;
        std::vector<glm::vec4> _selectedAreaColors { glm::vec4(0.24f, 0.47f, 0.85f, 0.4f) };

        //Placeholder Text
        std::unique_ptr<Text> _placeholderText { nullptr };
        std::vector<glm::vec4> _placeholderTextColors { glm::vec4(0.5f, 0.5f, 0.5f, 1.0f) };

        //The caret/selection/clipboard/key-repeat editing loop lives in ITextInteraction.
    };
}