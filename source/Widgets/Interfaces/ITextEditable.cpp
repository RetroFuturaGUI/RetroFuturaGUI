#include "ITextEditable.hpp"
#include "Clipboard.hpp"
#include "PlatformBridge.hpp"

#if defined(TARGET_PLATFORM_LINUX)
    #define GLFW_EXPOSE_NATIVE_X11
#elif defined(TARGET_PLATFORM_WINDOWS)
    #define GLFW_EXPOSE_NATIVE_WIN32
#endif
#include <GLFW/glfw3native.h>

void RetroFuturaGUI::ITextEditable::SetReadOnly(const bool readOnly)
{
    _readOnly = readOnly;
}

bool RetroFuturaGUI::ITextEditable::IsReadOnly() const
{
    return _readOnly;
}

void RetroFuturaGUI::ITextEditable::emitChange()
{
    if(_enterPressed)
        return;

    _onTextChangeAsync.EmitAsync();
    _onTextChange.Emit();
}

void RetroFuturaGUI::ITextEditable::SetPlaceholderTextColor(const glm::vec4& color)
{
    if(!_placeholderText)
        return;

    _placeholderText->SetColor(color);
}

void RetroFuturaGUI::ITextEditable::SetPlaceholderText(std::string_view text)
{
    if(!_placeholderText)
        return;

    if(text.empty())
    {
        _placeholderText->SetTextUTF8("");
        return;
    }

    _placeholderText->SetTextUTF8(text);
}

const std::string& RetroFuturaGUI::ITextEditable::GetPlaceholderText() const
{
    if(!_placeholderText)
        return _dummy;

    return _placeholderText->GetTextUTF8();
}

void RetroFuturaGUI::ITextEditable::SetFontFamily(std::string_view fontFamily, const f32 fontSize, const PlatformBridge::Fonts::Slant slant, const PlatformBridge::Fonts::Weight fontWeight)
{
    if(_text)
        _text->SetFontFamily(fontFamily, fontSize, slant, fontWeight);

    if(_placeholderText)
        _placeholderText->SetFontFamily(fontFamily, fontSize, slant, fontWeight);
}

void RetroFuturaGUI::ITextEditable::SetTextAlignment(const TextAlignment alignment)
{
    if(_text)
        _text->SetTextAlignment(alignment);

    if(_placeholderText)
        _placeholderText->SetTextAlignment(alignment);
}
        
void RetroFuturaGUI::ITextEditable::SetTextPadding(const f32 padding)
{
    if(_text)
        _text->SetTextPadding(padding);

    if(_placeholderText)
        _placeholderText->SetTextPadding(padding);
}


RetroFuturaGUI::Text* RetroFuturaGUI::ITextEditable::activeText() const
{
    return _text.get();
}

bool RetroFuturaGUI::ITextEditable::isTextReadOnly() const
{
    return _readOnly;
}