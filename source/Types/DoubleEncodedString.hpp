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

    class DoubleEncodedString
    {
    public:
        DoubleEncodedString() = default;
        DoubleEncodedString(const uSize maxLength, std::string_view initialString = "");
        DoubleEncodedString(const uSize maxLength, std::u32string_view initialString = U"");
        DoubleEncodedString(const DoubleEncodedString& other);
        DoubleEncodedString(const DoubleEncodedString&& other);
        ~DoubleEncodedString() = default;
        void operator=(const DoubleEncodedString& other);
        void operator=(const DoubleEncodedString&& other);
        void operator=(std::string_view newString);
        void operator=(std::u32string_view newString);

        const std::string& GetUtf8() const;
        const std::u32string& GetUtf32() const;
        uSize GetCapacity() const;
        uSize GetUtf8Length() const;
        uSize GetUtf32Length() const;
        void Clear();
        void Resize(const uSize newMaxLength);
        static std::u32string Utf8ToUtf32(std::string_view input);
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