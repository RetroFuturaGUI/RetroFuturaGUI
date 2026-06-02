#pragma once
#include <string>
#include <string_view>
#include "IncludeHelper.hpp"

namespace RetroFuturaGUI
{
    class DoubleEncodedString
    {
    public:
        DoubleEncodedString() = default;
        DoubleEncodedString(const uSize maxLength, std::string_view initialString = "");
        DoubleEncodedString(const DoubleEncodedString& other);
        DoubleEncodedString(const DoubleEncodedString&& other);
        ~DoubleEncodedString() = default;
        void operator=(const DoubleEncodedString& other);
        void operator=(const DoubleEncodedString&& other);
        void operator=(std::string_view newString);

        const std::string& GetUtf8() const;
        const std::u32string& GetUtf32() const;
        uSize GetCapacity() const;
        uSize GetUtf8Length() const;
        uSize GetUtf32Length() const;
        void Clear();
        void Resize(const uSize newMaxLength);

    private:
        std::string _utf8String {};
        std::u32string _utf32String {};
        uSize _capacity { 0 };

        void updateUtf32FromUtf8();
    };
}