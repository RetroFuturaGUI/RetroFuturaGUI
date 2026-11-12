#pragma once
#include "IncludeHelper.hpp"
#include "CheckBox.hpp"
#include "ColorPicker.hpp"
#include "ComboBox.hpp"
#include "DialogKit.hpp"
#include "Label.hpp"
#include "Popup.hpp"
#include "Slider.hpp"
#include "TextBox.hpp"
#include "ValueField.hpp"
#include "Window.hpp"
#include <array>
#include <charconv>
#include <concepts>
#include <format>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <tuple>
#include <utility>
#include <vector>

namespace RetroFuturaGUI
{
    class ColorPreview;

    /// @brief The widget a field of a Value popup is asked for with
    enum class ValueRowKind : u32
    {
        Text,   //a text box
        Number, //a text box that checks its number
        Check,  //a check box
        Color,  //a color picker with brightness and alpha sliders and a preview
        Range,  //a slider with its number beside it
        Choice  //a combo box
    };

    /// @brief Builds and holds a Value popup's window. The bar with its title, the description wrapped above the fields, one
    ///        row per field (its label and its widget) and the OK and Cancel buttons, the window as tall as all of them. What
    ///        doesn't depend on the values' types, so ValuePopup<Ts...> stays a thin template over it
    class ValueForm final : protected DialogKit
    {
    public:
        /// @param kinds One per field, in order
        /// @param labels One per field, in order
        /// @param listItemCounts One per field, in order. A choice's item count, 0 for any other field
        ValueForm(Window* window, const uSize number, std::string_view title, std::string_view description, std::span<const ValueRowKind> kinds, std::span<const std::string_view> labels, std::span<const uSize> listItemCounts);
        ValueForm() = delete;
        ValueForm(const ValueForm&) = delete;
        ValueForm(ValueForm&&) = delete;
        auto operator =(const ValueForm&) = delete;
        auto operator =(ValueForm&&) = delete;
        ~ValueForm() = default;

        /// @brief A row's widgets, nullptr when the row has no such widget.
        TextBox* GetTextBox(const uSize row) const;
        CheckBox* GetCheckBox(const uSize row) const;
        ColorPicker* GetColorPicker(const uSize row) const;
        Slider* GetSlider(const uSize row) const;
        Label* GetRangeLabel(const uSize row) const;
        ComboBox* GetComboBox(const uSize row) const;

        Button* GetOk() const;
        Button* GetCancel() const;
        Button* GetClose() const;
        Scene* GetScene() const;

        /// @brief Hands the scene and its widgets to the popup, which destroys them with its window
        std::unique_ptr<ISceneHost> TakeHost();

        /// @brief Sets a color row's color, its brightness and alpha sliders and its preview
        void SetColor(const uSize row, const glm::vec4& color);

        /// @brief Shows a row's field as invalid (its scope's TextBox:invalid entry, a red border) or not
        void MarkInvalid(const uSize row, const bool isInvalid);

        /// @brief Opens a choice row's list where its items fit: below the field, else above it, else on the roomier side
        ///        showing as many as fit. Called once the items are added.
        void FitChoiceList(const uSize row);

        /// @brief Lets OK confirm, or disables it while a field is invalid
        void EnableOk(const bool enable);

        /// @brief Returns whether Enter was pressed since the last call
        bool WasEnterPressed();

        /// @brief Measures the description's lines and, when their height changed since, makes its row, the window and the
        ///        description that tall, the other rows keeping theirs. Each frame, so a font a scope or flavor changed lays it out again
        void LayoutToDescription();

        /// @brief The widget a field type is asked for with
        template<ValueFieldType T>
        static constexpr ValueRowKind KindOf()
        {
            if constexpr (std::same_as<T, std::string>)
                return ValueRowKind::Text;
            else if constexpr (std::same_as<T, bool>)
                return ValueRowKind::Check;
            else if constexpr (std::same_as<T, glm::vec4>)
                return ValueRowKind::Color;
            else if constexpr (IsValueRange<T>)
                return ValueRowKind::Range;
            else if constexpr (std::same_as<T, ValueChoice>)
                return ValueRowKind::Choice;
            else
                return ValueRowKind::Number;
        }

        /// @brief Reads a number of the type: the whole text, spaces around it aside, within the type's range; nothing otherwise
        template<NumericValueType T> requires (!std::same_as<T, bool>)
        static std::optional<T> ParseNumber(std::string_view text)
        {
            while(!text.empty() && ' ' == text.front())
                text.remove_prefix(1);

            while(!text.empty() && ' ' == text.back())
                text.remove_suffix(1);

            if(text.empty())
                return std::nullopt;

            T value {};
            const std::from_chars_result result { std::from_chars(text.data(), text.data() + text.size(), value) };

            if(std::errc {} != result.ec || text.data() + text.size() != result.ptr)
                return std::nullopt;

            return value;
        }

        /// @brief Writes a number as a text box shows it: in full for a field, at two decimals beside a slider
        template<NumericValueType T> requires (!std::same_as<T, bool>)
        static std::string FormatNumber(const T value, const bool isBesideSlider)
        {
            if constexpr (std::floating_point<T>)
                return isBesideSlider ? std::format("{:.2f}", value) : std::format("{}", value);
            else if constexpr (1 == sizeof(T))
                return std::format("{}", static_cast<i32>(value)); //not as a character
            else
                return std::format("{}", value);
        }

        /// @brief How many items a field's list holds: a choice's, 0 for any other field
        template<ValueFieldType T>
        static uSize ListItemCountOf([[maybe_unused]] const T& value)
        {
            if constexpr (std::same_as<T, ValueChoice>)
                return value._Items.size();
            else
                return 0;
        }

        /// @brief The height the window opens with before it is measured
        static i32 FirstHeight();

        /// @brief Each popup's number, which names its window and scene apart
        static uSize NextNumber();

        static constexpr f32 _width { 520.0f };

    private:
        struct Row
        {
            ValueRowKind _Kind { ValueRowKind::Text };
            TextBox* _TextBox { nullptr };
            CheckBox* _CheckBox { nullptr };
            ColorPicker* _ColorPicker { nullptr };
            Slider
                * _Slider { nullptr },
                * _Brightness { nullptr },
                * _Alpha { nullptr };
            ColorPreview* _Preview { nullptr };
            Label* _RangeLabel { nullptr };
            ComboBox* _ComboBox { nullptr };
            f32
                _SpaceAbove { 0.0f }, //the room for a list above and below the field, the bar left out.
                _SpaceBelow { 0.0f };
            bool _IsInvalid { false };
        };

        /// @brief Builds a row's widgets in its Lasagna row
        void attachRow(Lasagna& lasagna, const uSize lasagnaRow, const ValueRowKind kind, std::string_view label, const AxisDefinition& axis, const glm::vec2& windowSize);

        /// @brief Follows a color row's sliders. The picker takes their brightness and alpha, the preview the color
        void followColor(const uSize row);

        /// @brief Gives a slider the popups' look
        static void styleSlider(Slider& slider);

        /// @brief How tall a row of the kind is
        static f32 rowHeightOf(const ValueRowKind kind);

        /// @brief How tall the description's row is for its lines, padded, or the gap under the bar without a description
        f32 measureDescription() const;

        std::unique_ptr<ViewHost> _host { nullptr };
        std::vector<Row> _rows {};
        Button
            * _ok { nullptr },
            * _cancel { nullptr },
            * _close { nullptr };
        u32 _enterPressCountSeen { 0 };
        Window* _window { nullptr };
        Lasagna* _lasagna { nullptr };
        Label* _description { nullptr };
        bool _hasDescription { false };
        std::vector<f32> _rowHeights {}; //bar, description, the fields, a spacer when a list needs one, buttons

        inline static uSize _popupCount { 0 };
        static constexpr uSize
            _columnCount { 12 },
            _labelColumnCount { 4 };
        static constexpr f32
            _barHeight { 40.0f },
            _fieldHeight { 44.0f },
            _colorHeight { 170.0f },
            _buttonRowHeight { 56.0f },
            _emptyDescription { 8.0f }; //no description still leaves a gap under the bar
        static constexpr ViewCell _descriptionCell { ._Row = 1, ._Column = 0, ._ColumnSpan = _columnCount, ._SizingMode = SizingMode::FIXED, ._Layer = 0 };
        static constexpr uSize _listRowCount { 8 }; //the rows a drop-down list shows by default.
    };

    /// @brief Asks for values of the given types, one field each, and closes with them. OK and Enter close it once every field
    ///        holds a valid value (Connect_OnResult hands them over in the fields' order). Cancel, the bar's close button and
    ///        Escape dismiss it (Connect_OnDismiss). A number field that doesn't hold a number of its type disables OK and
    ///        shows its scope's TextBox:invalid entry (a red border). Opened by Window::OpenPopup with PopupTypes::Value its widgets lie in
    ///        StyleScope::Value
    template<typename... Ts>
    class ValuePopup final : public Popup<Ts...>
    {
    public:
        /// @param fields One per value, in order. Its label and the value it starts at
        template<ValueFieldType... Fs> requires (sizeof...(Fs) == sizeof...(Ts)) && (std::same_as<ValueResultOf<Fs>, Ts> && ...)
        ValuePopup(std::string_view title, std::string_view description, const ValueField<Fs>&... fields)
            : ValuePopup(ValueForm::NextNumber(), title, description, fields...)
        {
        }

        ValuePopup() = delete;
        ValuePopup(const ValuePopup&) = delete;
        ValuePopup(ValuePopup&&) = delete;
        auto operator =(const ValuePopup&) = delete;
        auto operator =(ValuePopup&&) = delete;
        ~ValuePopup() override = default;

    private:
        template<ValueFieldType... Fs>
        ValuePopup(const uSize number, std::string_view title, std::string_view description, const ValueField<Fs>&... fields)
            : Popup<Ts...>(std::format("ValuePopup{}", number), static_cast<i32>(ValueForm::_width), ValueForm::FirstHeight(), StyleScope::Value),
              _form(this->GetWindow(), number, title, description, std::array<ValueRowKind, sizeof...(Fs)> { ValueForm::KindOf<Fs>()... }, std::array<std::string_view, sizeof...(Fs)> { fields._Label... }, std::array<uSize, sizeof...(Fs)> { ValueForm::ListItemCountOf(fields._Value)... })
        {
            setFields(std::index_sequence_for<Fs...> {}, fields...);

            if(Button* ok { _form.GetOk() })
                ok->Connect_OnClick([this]() { confirm(std::index_sequence_for<Ts...> {}); }, false);

            if(Button* cancel { _form.GetCancel() })
                cancel->Connect_OnClick([this]() { this->Dismiss(); }, false);

            if(Button* close { _form.GetClose() })
                close->Connect_OnClick([this]() { this->Dismiss(); }, false);

            if(Scene* scene { _form.GetScene() })
                scene->Connect_OnInteract([this]() { followFields(std::index_sequence_for<Ts...> {}); }, false);

            this->AddScene(_form.TakeHost());
        }

        template<uSize... Is, typename... Fs>
        void setFields(std::index_sequence<Is...>, const ValueField<Fs>&... fields)
        {
            (setField(Is, fields._Value), ...);
        }

        /// @brief Shows a field's starting value in its widget
        template<typename F>
        void setField(const uSize row, const F& value)
        {
            if constexpr (std::same_as<F, std::string>)
                setText(row, value);
            else if constexpr (std::same_as<F, bool>)
                setCheck(row, value);
            else if constexpr (std::same_as<F, glm::vec4>)
                _form.SetColor(row, value);
            else if constexpr (IsValueRange<F>)
                setRange(row, value);
            else if constexpr (std::same_as<F, ValueChoice>)
                setChoice(row, value);
            else
                setText(row, ValueForm::FormatNumber(value, false));
        }

        void setText(const uSize row, std::string_view text)
        {
            TextBox* textBox { _form.GetTextBox(row) };

            if(!textBox)
                return;

            textBox->GetText().SetString(text, false);
        }

        void setCheck(const uSize row, const bool value)
        {
            CheckBox* checkBox { _form.GetCheckBox(row) };

            if(!checkBox)
                return;

            checkBox->SetValue(value, false);
        }

        template<typename T>
        void setRange(const uSize row, const ValueRange<T>& range)
        {
            Slider* slider { _form.GetSlider(row) };

            if(!slider)
                return;

            Label* label { _form.GetRangeLabel(row) };

            if(!label)
                return;

            slider->SetMinValue<T>(range._Min);
            slider->SetMaxValue<T>(range._Max);
            slider->SetValue<T>(range._Value, false);
            label->GetText().SetString(ValueForm::FormatNumber(slider->GetValue<T>(), true), false);
            slider->Connect_OnValueChanged([slider, label]() { label->GetText().SetString(ValueForm::FormatNumber(slider->GetValue<T>(), true), false); }, false);
        }

        void setChoice(const uSize row, const ValueChoice& choice)
        {
            ComboBox* comboBox { _form.GetComboBox(row) };

            if(!comboBox)
                return;

            for(const std::string& item : choice._Items)
                comboBox->AddItem(item);

            comboBox->SetIndex(choice._Selected, false);
            _form.FitChoiceList(row);
        }

        /// @brief Reads a row as the type it hands back: nothing while a number field holds no valid number.
        template<typename T>
        std::optional<T> readRow(const uSize row) const
        {
            if constexpr (std::same_as<T, std::string>)
            {
                TextBox* textBox { _form.GetTextBox(row) };

                if(!textBox)
                    return std::nullopt;

                return textBox->GetText().GetString();
            }
            else if constexpr (std::same_as<T, bool>)
            {
                CheckBox* checkBox { _form.GetCheckBox(row) };

                if(!checkBox)
                    return std::nullopt;

                return checkBox->GetValue();
            }
            else if constexpr (std::same_as<T, glm::vec4>)
            {
                ColorPicker* colorPicker { _form.GetColorPicker(row) };

                if(!colorPicker)
                    return std::nullopt;

                return colorPicker->GetColorRGBAF();
            }
            else
            {
                if(ComboBox* comboBox { _form.GetComboBox(row) })
                    return static_cast<T>(comboBox->GetSelectedIndex());

                if(Slider* slider { _form.GetSlider(row) })
                    return slider->GetValue<T>();

                TextBox* textBox { _form.GetTextBox(row) };

                if(!textBox)
                    return std::nullopt;

                return ValueForm::ParseNumber<T>(textBox->GetText().GetString());
            }
        }

        /// @brief Each frame. Marks the fields that are invalid, lets OK confirm only while none is, and confirms on Enter
        template<uSize... Is>
        void followFields(std::index_sequence<Is...>)
        {
            const std::array<bool, sizeof...(Is)> isValid { readRow<Ts>(Is).has_value()... };
            bool isEveryValid { true };

            for(uSize row = 0; row < isValid.size(); ++row)
            {
                _form.MarkInvalid(row, !isValid[row]);
                isEveryValid = isEveryValid && isValid[row];
            }

            _form.EnableOk(isEveryValid);
            _form.LayoutToDescription();

            if(!_form.WasEnterPressed())
                return;

            confirm(std::index_sequence<Is...> {});
        }

        /// @brief Closes with the fields' values, once every one is valid
        template<uSize... Is>
        void confirm(std::index_sequence<Is...>)
        {
            std::tuple<std::optional<Ts>...> values { readRow<Ts>(Is)... };

            if(!(std::get<Is>(values).has_value() && ...))
                return;

            this->Close(std::move(*std::get<Is>(values))...);
        }

        ValueForm _form;
    };

    template<ValueFieldType... Fs>
    ValuePopup<ValueResultOf<Fs>...>* Window::OpenPopup(const PopupTypes type, std::string_view title, std::string_view description, const ValueField<Fs>&... fields)
    {
        //The fields ask for values; any other kind takes a summary instead
        if(PopupTypes::Value != type)
            return nullptr;

        std::unique_ptr<ValuePopup<ValueResultOf<Fs>...>> popup { std::make_unique<ValuePopup<ValueResultOf<Fs>...>>(title, description, fields...) };
        ValuePopup<ValueResultOf<Fs>...>* opened { popup.get() };

        if(!PushPopup(std::move(popup)))
            return nullptr;

        return opened;
    }
}