#pragma once
#include "IncludeHelper.hpp"
#include <concepts>
#include <string>
#include <string_view>
#include <vector>

namespace RetroFuturaGUI
{
    /// @brief A number between two bounds, which a Value popup asks for with a slider showing the number. Hands back a T
    template<NumericValueType T> requires (!std::same_as<T, bool>)
    struct ValueRange
    {
        T
            _Min {},
            _Max {},
            _Value {};
    };

    /// @brief Whether a type is a ValueRange.
    template<typename T> inline constexpr bool IsValueRange { false };
    template<typename T> inline constexpr bool IsValueRange<ValueRange<T>> { true };

    /// @brief One of a list of texts, which a Value popup asks for with a combo box. Hands back the chosen one's index
    struct ValueChoice
    {
        std::vector<std::string> _Items {};
        uSize _Selected { 0 }; //the index chosen when the popup opens.
    };

    /// @brief The types a Value popup can ask for: any number (a text box that checks it), std::string (a text box), bool (a
    ///        check box), glm::vec4 (a color picker with brightness and alpha), a ValueRange (a slider) and a ValueChoice
    ///        (a combo box)
    template<typename T>
    concept ValueFieldType = NumericValueType<T> || std::same_as<T, std::string> || std::same_as<T, glm::vec4> || IsValueRange<T> || std::same_as<T, ValueChoice>;

    /// @brief One value a Value popup asks for: its label and the value it starts at.
    ///        ValueField { ._Label = "Count", ._Value = 3 } asks for an i32; a string literal asks for a std::string
    template<ValueFieldType T>
    struct ValueField
    {
        std::string_view _Label {};
        T _Value {};
    };

    ValueField(std::string_view, const char*) -> ValueField<std::string>;
    template<ValueFieldType T> ValueField(std::string_view, T) -> ValueField<T>;

    /// @brief The type a field hands back: a range's number, a choice's index, any other field its own type
    template<typename T> struct ValueResult { using Type = T; };
    template<typename T> struct ValueResult<ValueRange<T>> { using Type = T; };
    template<> struct ValueResult<ValueChoice> { using Type = uSize; };
    template<typename T> using ValueResultOf = typename ValueResult<T>::Type;
}