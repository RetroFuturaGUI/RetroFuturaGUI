#include "ITextEditable.hpp"
#include "Clipboard.hpp"
#include "PlatformBridge.hpp"

#if defined(TARGET_PLATFORM_LINUX)
    #define GLFW_EXPOSE_NATIVE_X11
#elif defined(TARGET_PLATFORM_WINDOWS)
    #define GLFW_EXPOSE_NATIVE_WIN32
#endif
#include <GLFW/glfw3native.h>

void RetroFuturaGUI::ITextEditable::updateCaretPosition()
{
    glm::vec3 caretPosition { _text->GetBoundaryPosition(_caretPosition, _caret->GetSize().y) };
    caretPosition.x = keepCaretVisible(caretPosition.x, _caret->GetSize().x * 0.5f);
    _caret->SetPosition(caretPosition);
    resetCaretBlink();
}

void RetroFuturaGUI::ITextEditable::drawSelectedArea()
{
    if(!_selectedArea)
        return;

    if(_isSelected)
        _selectedArea->Draw();
}

void RetroFuturaGUI::ITextEditable::updateSelectedArea()
{
    const uSize
        left { markedStart() },
        right { markedEnd() };

    if(!_text || !_selectedArea || left == right) //nothing selected
    {
        _isSelected = false;
        return;
    }

    const glm::vec3
        leftPosition { _text->GetBoundaryPosition(left, _caret->GetSize().y) },
        rightPosition { _text->GetBoundaryPosition(right, _caret->GetSize().y) };
    const f32
        clippedLeftX { clampToTextBounds(leftPosition.x) },
        clippedRightX { clampToTextBounds(rightPosition.x) },
        width { clippedRightX - clippedLeftX };

    if(width <= 0.0f) //selection sits entirely outside the visible text area
    {
        _isSelected = false;
        return;
    }

    _selectedArea->SetSize(glm::vec2(width, _caret->GetSize().y));
    _selectedArea->SetPosition(glm::vec3(clippedLeftX + width * 0.5f, leftPosition.y, _selectedArea->GetPosition().z));
    _isSelected = true;
}


void RetroFuturaGUI::ITextEditable::SetReadOnly(const bool readOnly)
{
    _readOnly = readOnly;
}

bool RetroFuturaGUI::ITextEditable::IsReadOnly() const
{
    return _readOnly;
}

void RetroFuturaGUI::ITextEditable::SetCaretColors(std::span<glm::vec4> colors)
{
    _caretColors.assign(colors.begin(), colors.end());

    if(_caret)
        _caret->SetColors(_caretColors);
}

void RetroFuturaGUI::ITextEditable::SetCaretFillType(const FillType fillType)
{
    if(_caret)
        _caret->SetFillType(fillType);
}

void RetroFuturaGUI::ITextEditable::SetCaretGradientAnimationSpeed(const f32 speed)
{
    if(_caret)
        _caret->SetGradientAnimationSpeed(speed);
}

void RetroFuturaGUI::ITextEditable::SetSelectedAreaColors(std::span<glm::vec4> colors)
{
    _selectedAreaColors.assign(colors.begin(), colors.end());

    if(_selectedArea)
        _selectedArea->SetColors(_selectedAreaColors);
}

void RetroFuturaGUI::ITextEditable::SetSelectedAreaFillType(const FillType fillType)
{
    if(_selectedArea)
        _selectedArea->SetFillType(fillType);
}

void RetroFuturaGUI::ITextEditable::SetSelectedAreaGradientAnimationSpeed(const f32 speed)
{
    if(_selectedArea)
        _selectedArea->SetGradientAnimationSpeed(speed);
}

void RetroFuturaGUI::ITextEditable::SetSelectedAreaGradientOffset(const f32 gradientOffset)
{
    if(_selectedArea)
        _selectedArea->SetGradientOffset(gradientOffset);
}

void RetroFuturaGUI::ITextEditable::SetSelectedAreaGradientDegree(const f32 degree)
{
    if(_selectedArea)
        _selectedArea->SetGradientDegree(degree);
}

void RetroFuturaGUI::ITextEditable::SetSelectedAreaGradientRotationSpeed(const f32 rotationSpeed)
{
    if(_selectedArea)
        _selectedArea->SetGradientRotationSpeed(rotationSpeed);
}

void RetroFuturaGUI::ITextEditable::SetSelectedAreaCornerRadii(const glm::vec4& radii)
{
    if(_selectedArea)
        _selectedArea->SetCornerRadii(radii);
}

void RetroFuturaGUI::ITextEditable::emitChange()
{
    syncValueFromText();

    if(_enterPressed)
        return;

    _onTextChangeAsync.EmitAsync();
    _onTextChange.Emit();
}

void RetroFuturaGUI::ITextEditable::syncValueFromText()
{
    if(!_text)
        return;

    _valueText = _text->GetTextUTF8();
}

void RetroFuturaGUI::ITextEditable::SetCaretBlinkTime(const f64 milliseconds)
{
    if(milliseconds <= 16.0)
    {
        _caretNeverBlinks = true;
        return;
    }

    _blinkForMilliseconds = milliseconds;
    _caretNeverBlinks = false;
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