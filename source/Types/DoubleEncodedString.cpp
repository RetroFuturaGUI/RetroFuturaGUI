#include "DoubleEncodedString.hpp"

RetroFuturaGUI::DoubleEncodedString::DoubleEncodedString(const uSize capacity, std::string_view initialString)
    : _capacity(capacity)
{
    if(capacity == 0)
        return;

    if (initialString.empty())
        return;

    _utf8String = std::string(initialString);
    updateUtf32FromUtf8();
}

RetroFuturaGUI::DoubleEncodedString::DoubleEncodedString(const DoubleEncodedString& other)
    : _utf8String(other._utf8String)
    , _utf32String(other._utf32String)
    , _capacity(other._capacity)
{
}

RetroFuturaGUI::DoubleEncodedString::DoubleEncodedString(const DoubleEncodedString&& other)
    : _utf8String(std::move(other._utf8String))
    , _utf32String(std::move(other._utf32String))
    , _capacity(other._capacity)
{
}

void RetroFuturaGUI::DoubleEncodedString::operator=(const DoubleEncodedString& other)
{
    _utf8String = other._utf8String;
    _utf32String = other._utf32String;
    _capacity = other._capacity;
}

void RetroFuturaGUI::DoubleEncodedString::operator=(const DoubleEncodedString&& other)
{
    _utf8String = std::move(other._utf8String);
    _utf32String = std::move(other._utf32String);
    _capacity = other._capacity;
}

void RetroFuturaGUI::DoubleEncodedString::operator=(std::string_view newString)
{
    if(_capacity < newString.length())
    {
        _capacity = newString.length();
        _utf8String = std::string(newString);
    }
    else
        _utf8String = newString;

    updateUtf32FromUtf8();
}

const std::string& RetroFuturaGUI::DoubleEncodedString::GetUtf8() const
{
    return _utf8String;
}

const std::u32string& RetroFuturaGUI::DoubleEncodedString::GetUtf32() const
{
    return _utf32String;
}

uSize RetroFuturaGUI::DoubleEncodedString::GetCapacity() const
{
    return _capacity;
}

uSize RetroFuturaGUI::DoubleEncodedString::GetUtf8Length() const
{
    return _utf8String.length();
}

uSize RetroFuturaGUI::DoubleEncodedString::GetUtf32Length() const
{
    return _utf32String.size();
}

void RetroFuturaGUI::DoubleEncodedString::Clear()
{
    _utf8String.clear();
    _utf32String.clear();
}

void RetroFuturaGUI::DoubleEncodedString::Resize(const uSize capacity)
{
    if(capacity == 0)
        return;

    _capacity = capacity;
    _utf8String.resize(_capacity);
    updateUtf32FromUtf8();
}

void RetroFuturaGUI::DoubleEncodedString::updateUtf32FromUtf8()
{
    constexpr static const u32 maskSingle = 0;
    constexpr static const u32 maskDouble = 0b11000000;
    constexpr static const u32 maskTriple = 0b11100000;
    constexpr static const u32 maskQuadruple = 0b11110000;
    constexpr static const u32 maskSequence = 0b10000000;
    constexpr static const u32 ANDMaskSingle = 0b10000000;
    constexpr static const u32 ANDMaskDouble = 0b11100000;
    constexpr static const u32 ANDMaskTriple = 0b11110000;
    constexpr static const u32 ANDMaskQuadruple = 0b11111000;
    constexpr static const u32 ANDMaskSequence = 0b11000000;
    constexpr static const u32 InvMaskDouble = 0b00011111;
    constexpr static const u32 InvMaskTriple = 0b00001111;
    constexpr static const u32 InvMaskQuadruple = 0b00000111;
    constexpr static const u32 InvMaskSequence = 0b00111111;
    _utf32String.clear();
    _utf32String.resize(_capacity);
    uSize count { 0 };
    
    for(size_t i { 0 }; i < _utf8String.size();)
    {
        if((_utf8String[i] & ANDMaskDouble) == maskDouble)
        {
            if(((_utf8String[i+1] & ANDMaskSequence) != maskSequence))
                break;
            
            _utf32String[count] = ((_utf8String[i] & InvMaskDouble) << 6);
            _utf32String[count] |= (_utf8String[i+1] & InvMaskSequence);
            ++count;
            i += 2;
        }
        else if(((_utf8String[i] & ANDMaskTriple) == maskTriple))
        {
            if(((_utf8String[i+1] & ANDMaskSequence) != maskSequence))
                break;
                
            if(((_utf8String[i+2] & ANDMaskSequence) != maskSequence))
                break;
            
            _utf32String[count] = ((_utf8String[i] & InvMaskTriple) << 12);
            _utf32String[count] |= (_utf8String[i+1] & InvMaskSequence) << 6;
            _utf32String[count] |= (_utf8String[i+2] & InvMaskSequence);
            ++count;
            i += 3;
        }
        else if(((_utf8String[i] & ANDMaskQuadruple) == maskQuadruple))
        {
            if(((_utf8String[i+1] & ANDMaskSequence) != maskSequence))
                break;
                
            if(((_utf8String[i+2] & ANDMaskSequence) != maskSequence))
                break;
                
            if(((_utf8String[i+3] & ANDMaskSequence) != maskSequence))
                break;
            
            _utf32String[count] = ((_utf8String[i] & InvMaskQuadruple) << 18);
            _utf32String[count] |= (_utf8String[i+1] & InvMaskSequence) << 12;
            _utf32String[count] |= (_utf8String[i+2] & InvMaskSequence) << 6;
            _utf32String[count] |= (_utf8String[i+3] & InvMaskSequence);
            ++count;
            i += 4;
        }
        else
        {
            if(((_utf8String[i] & ANDMaskSingle) != maskSingle))
                break;
                
            _utf32String[count] = static_cast<uint32_t>(_utf8String[i]);    
            ++count;    
            ++i;   
        }
    }
}