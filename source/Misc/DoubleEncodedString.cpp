#include "DoubleEncodedString.hpp"
#include <algorithm>

RetroFuturaGUI::DoubleEncodedString::DoubleEncodedString(const uSize maxLength, std::string_view initialString)
    : _capacity(maxLength)
{
    if(_capacity == 0)
        return;

    if (initialString.empty())
        return;

    _utf8String = std::string(initialString);
    updateUtf32FromUtf8();
}

RetroFuturaGUI::DoubleEncodedString::DoubleEncodedString(const uSize maxLength, std::u32string_view initialString)
    : _capacity(maxLength)
{
    if(_capacity == 0)
        return;

    if (initialString.empty())
        return;

    _utf32String = std::u32string(initialString);
    updateUtf8FromUtf32();
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

void RetroFuturaGUI::DoubleEncodedString::operator=(std::u32string_view newString)
{
    if(_capacity < newString.length())
    {
        _capacity = newString.length();
        _utf32String = std::u32string(newString);
    }
    else
        _utf32String = std::u32string(newString);

    updateUtf8FromUtf32();
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
    _utf32String = Utf8ToUtf32(_utf8String);
}

void RetroFuturaGUI::DoubleEncodedString::updateUtf8FromUtf32()
{
    _utf8String = Utf32ToUtf8(_utf32String);
}

std::u32string RetroFuturaGUI::DoubleEncodedString::Utf8ToUtf32(std::string_view input)
{
    std::u32string utf32;
    utf32.resize(input.size());
    uSize count { 0 };
    
    for(uSize i { 0 }; i < input.size();)
    {
        if((input[i] & _andMaskDouble) == _maskDouble)
        {
            if(((input[i+1] & _andMaskSequence) != _maskSequence))
                break;
            
            utf32[count] = ((input[i] & _invMaskDouble) << 6);
            utf32[count] |= (input[i+1] & _invMaskSequence);
            ++count;
            i += 2;
        }
        else if(((input[i] & _andMaskTriple) == _maskTriple))
        {
            if(((input[i+1] & _andMaskSequence) != _maskSequence))
                break;
                
            if(((input[i+2] & _andMaskSequence) != _maskSequence))
                break;
            
            utf32[count] = ((input[i] & _invMaskTriple) << 12);
            utf32[count] |= (input[i+1] & _invMaskSequence) << 6;
            utf32[count] |= (input[i+2] & _invMaskSequence);
            ++count;
            i += 3;
        }
        else if(((input[i] & _andMaskQuadruple) == _maskQuadruple))
        {
            if(((input[i+1] & _andMaskSequence) != _maskSequence))
                break;
                
            if(((input[i+2] & _andMaskSequence) != _maskSequence))
                break;
                
            if(((input[i+3] & _andMaskSequence) != _maskSequence))
                break;
            
            utf32[count] = ((input[i] & _invMaskQuadruple) << 18);
            utf32[count] |= (input[i+1] & _invMaskSequence) << 12;
            utf32[count] |= (input[i+2] & _invMaskSequence) << 6;
            utf32[count] |= (input[i+3] & _invMaskSequence);
            ++count;
            i += 4;
        }
        else
        {
            if(((input[i] & _andMaskSingle) != _maskSingle))
                break;
                
            utf32[count] = static_cast<u32>(input[i]);    
            ++count;    
            ++i;   
        }
    }

    uSize end { utf32.find_last_not_of(U'\0') };
    
    if (end != std::string::npos)
        utf32.erase(end + 1);
    else
        utf32.clear();

    return utf32;
}

std::string RetroFuturaGUI::DoubleEncodedString::Utf32ToUtf8(std::u32string_view input)
{
    std::string utf8;
    utf8.resize(input.size() * sizeof(u32));
    size_t count { 0 };
    
    for(size_t i { 0 }; i < input.size(); ++i)
    {
        if(input[i] == 0)
            break;

        if(input[i] < 0x80)
        {
            utf8[count] = static_cast<char>(input[i]);
            ++count;
        }
        else if(input[i] >= 0x80 && input[i] < 0x800)
        {
            utf8[count] = static_cast<char>((input[i] >> 6) | _maskDouble);
            utf8[count+1] = static_cast<char>((input[i] & _invMaskSequence) | _maskSequence);
            count += 2;
        }
        else if(input[i] >= 0x800 && input[i] < 0x10000)
        {
            utf8[count] = static_cast<char>((input[i] >> 12) | _maskTriple);
            utf8[count+1] = static_cast<char>(((input[i] >> 6) & _invMaskSequence) | _maskSequence);
            utf8[count+2] = static_cast<char>((input[i] & _invMaskSequence) | _maskSequence);
            count += 3;
        }
        else if(input[i] >= 0x10000 && input[i] < 0x110000)
        {
            utf8[count] = static_cast<char>((input[i] >> 18) | _maskQuadruple);
            utf8[count+1] = static_cast<char>(((input[i] >> 12) & _invMaskSequence) | _maskSequence);
            utf8[count+2] = static_cast<char>(((input[i] >> 6) & _invMaskSequence) | _maskSequence);
            utf8[count+3] = static_cast<char>((input[i] & _invMaskSequence) | _maskSequence);
            count += 4;
        }
        else
            break;
    }

    uSize end { utf8.find_last_not_of('\0') };
    
    if (end != std::string::npos)
        utf8.erase(end + 1);
    else
        utf8.clear();

    return utf8;
}