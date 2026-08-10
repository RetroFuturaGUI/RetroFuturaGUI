#pragma once
#include <string>
#include <string_view>
#include <type_traits>
#include <concepts>
#include "IncludeHelper.hpp"

namespace RetroFuturaGUI
{
    //optimize to prevent unnecessary allocations
    //template<typename T>
    //concept StringType = std::same_as<T, std::string> || std::same_as<T, std::u32string> || std::same_as<T, std::string_view> || std::same_as<T, std::u32string_view>;

    /// @brief Holds a string simultaneously as UTF-8 and UTF-32, keeping both encodings in sync so callers can read whichever is convenient without re-converting.
    class DoubleEncodedString
    {
    public:
        DoubleEncodedString() = default;

        /// @brief Constructs from a UTF-8 string, reserving capacity for maxLength codepoints.
        DoubleEncodedString(const uSize maxLength, std::string_view initialString = "");

        /// @brief Constructs from a UTF-32 string, reserving capacity for maxLength codepoints.
        DoubleEncodedString(const uSize maxLength, std::u32string_view initialString = U"");
        DoubleEncodedString(const DoubleEncodedString& other);
        DoubleEncodedString(const DoubleEncodedString&& other);
        ~DoubleEncodedString() = default;
        void operator=(const DoubleEncodedString& other);
        void operator=(const DoubleEncodedString&& other);

        /// @brief Assigns from a UTF-8 string, re-deriving the UTF-32 encoding.
        void operator=(std::string_view newString);

        /// @brief Assigns from a UTF-32 string, re-deriving the UTF-8 encoding.
        void operator=(std::u32string_view newString);

        /// @brief Returns the string in UTF-8 encoding.
        const std::string& GetUtf8() const;

        /// @brief Returns the string in UTF-32 encoding.
        const std::u32string& GetUtf32() const;

        /// @brief Returns the reserved capacity in codepoints.
        uSize GetCapacity() const;

        /// @brief Returns the length of the UTF-8 encoding in bytes.
        uSize GetUtf8Length() const;

        /// @brief Returns the length of the UTF-32 encoding in codepoints.
        uSize GetUtf32Length() const;

        /// @brief Clears both encodings.
        void Clear();

        /// @brief Changes the reserved capacity in codepoints, truncating the string if it no longer fits.
        void Resize(const uSize newMaxLength);

        /// @brief Converts a UTF-8 encoded string to UTF-32.
        static std::u32string Utf8ToUtf32(std::string_view input);

        /// @brief Converts a UTF-32 encoded string to UTF-8.
        static std::string Utf32ToUtf8(std::u32string_view input);

    private:
        std::string _utf8String {};
        std::u32string _utf32String {};
        uSize _capacity { 0 };

        void updateUtf32FromUtf8();
        void updateUtf8FromUtf32();

        /*template<StringType strIn, StringType strOut>
        requires (!std::same_as<strIn, strOut>)
        strOut removeTrailingZeros(const strIn& str)
        {
            strOut result { str };
        
            if constexpr (std::same_as<strIn, std::string_view>)
            {
                size_t end = result.find_last_not_of('\0');

                if (end != std::string::npos)
                    result.erase(end + 1);
                else
                    result.clear();
            }
            else if constexpr (std::same_as<strIn, std::u32string_view>)
            {
                size_t end = result.find_last_not_of(U'\0');

                if (end != std::u32string::npos)
                    result.erase(end + 1);
                else
                    result.clear();
            }
            
        return result;
    }*/

    constexpr static inline const u32 _maskSingle = 0;
    constexpr static inline const u32 _maskDouble = 0b11000000;
    constexpr static inline const u32 _maskTriple = 0b11100000;
    constexpr static inline const u32 _maskQuadruple = 0b11110000;
    constexpr static inline const u32 _maskSequence = 0b10000000;
    constexpr static inline const u32 _andMaskSingle = 0b10000000;
    constexpr static inline const u32 _andMaskDouble = 0b11100000;
    constexpr static inline const u32 _andMaskTriple = 0b11110000;
    constexpr static inline const u32 _andMaskQuadruple = 0b11111000;
    constexpr static inline const u32 _andMaskSequence = 0b11000000;
    constexpr static inline const u32 _invMaskDouble = 0b00011111;
    constexpr static inline const u32 _invMaskTriple = 0b00001111;
    constexpr static inline const u32 _invMaskQuadruple = 0b00000111;
    constexpr static inline const u32 _invMaskSequence = 0b00111111;
    };
}