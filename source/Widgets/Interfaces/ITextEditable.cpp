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
    if(!_text)
        return;

    if(!_caret)
        return;

    const f32 fullHeight { caretHeight() };
    glm::vec3 caretPosition { _text->GetBoundaryPosition(_caretPosition, fullHeight) };
    caretPosition.x = keepCaretVisible(caretPosition.x, _caret->GetSize().x * 0.5f);

    const f32
        clippedTopY { clampToTextBoundsY(caretPosition.y + fullHeight * 0.5f) },
        clippedBottomY { clampToTextBoundsY(caretPosition.y - fullHeight * 0.5f) },
        clippedHeight { clippedTopY - clippedBottomY };

    _isCaretInView = 0.0f < clippedHeight; //false while the caret's line is entirely outside the text area
    caretPosition.y = clippedBottomY + clippedHeight * 0.5f;
    _caret->SetSize(glm::vec2(_caret->GetSize().x, clippedHeight));
    _caret->SetPosition(caretPosition);
    resetCaretBlink();
}

void RetroFuturaGUI::ITextEditable::drawSelectedArea()
{
    if(!_selectedArea)
        return;

    if(!_isSelected)
        return;

    //Rectangle::Draw() advances the gradient animation on every call - start each line from the same point
    const f32
        gradientOffset { _selectedArea->GetGradientOffset() },
        gradientDegree { _selectedArea->GetGradientDegree() },
        z { _selectedArea->GetPosition().z };

    for(const SelectedLineArea& area : _selectedLineAreas)
    {
        _selectedArea->SetGradientOffset(gradientOffset);
        _selectedArea->SetGradientDegree(gradientDegree);
        _selectedArea->SetSize(area._Size);
        _selectedArea->SetPosition(glm::vec3(area._Center, z));
        _selectedArea->Draw();
    }
}

void RetroFuturaGUI::ITextEditable::updateSelectedArea()
{
    _isSelected = false;
    _selectedLineAreas.clear();

    if(!_text)
        return;

    if(!_selectedArea)
        return;

    if(!_caret)
        return;

    const uSize
        left { markedStart() },
        right { markedEnd() };

    if(left == right) //nothing selected
        return;

    const uSize
        firstLine { _text->GetBoundaryLine(left) },
        lastLine { _text->GetBoundaryLine(right) };
    const f32 lineHeight { _text->GetLineHeight() };
    uSize start { left };

    for(uSize line = firstLine; line <= lastLine; ++line)
    {
        const uSize end { line == lastLine ? right : _text->GetLineLastBoundary(line) };
        const f32 lineBreakWidth { line == lastLine ? 0.0f : _text->GetGlyphSize() * 0.3f }; //a split for the selected '\n', so selected empty lines still show
        const glm::vec3
            startPosition { _text->GetBoundaryPosition(start, _caret->GetSize().y) },
            endPosition { _text->GetBoundaryPosition(end, _caret->GetSize().y) };
        const f32
            clippedLeftX { clampToTextBounds(startPosition.x) },
            clippedRightX { clampToTextBounds(endPosition.x + lineBreakWidth) },
            width { clippedRightX - clippedLeftX },
            clippedTopY { clampToTextBoundsY(startPosition.y + lineHeight * 0.5f) },
            clippedBottomY { clampToTextBoundsY(startPosition.y - lineHeight * 0.5f) },
            height { clippedTopY - clippedBottomY };

        start = end + 1; //+1 starts the next line after '\n'

        if(0.0f >= width || 0.0f >= height) //skip what sits outside the visible text area
            continue;

        _selectedLineAreas.push_back({ ._Center = glm::vec2(clippedLeftX + width * 0.5f, clippedBottomY + height * 0.5f), ._Size = glm::vec2(width, height) });
    }

    _isSelected = !_selectedLineAreas.empty();
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

f32 RetroFuturaGUI::ITextEditable::caretHeight() const
{
    if(!_text)
        return 0.0f;

    return _text->GetGlyphSize() * _caretHeightFactor;
}