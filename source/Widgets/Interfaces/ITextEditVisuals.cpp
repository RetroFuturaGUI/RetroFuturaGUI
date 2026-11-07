#include "ITextEditVisuals.hpp"

RetroFuturaGUI::ITextEditVisuals::ITextEditVisuals(Projection* projection)
    : _caret(projection), _selectedArea(projection)
{
    _caret.SetSize(glm::vec2(_caretThickness, 0.0f)); //updateCaretPosition sets caret size by text size
}

void RetroFuturaGUI::ITextEditVisuals::updateCaretBlink()
{
    if(!_showCaret)
    {
        resetCaretBlink();
        return;
    }

    const f64 elapsedMilliseconds { std::chrono::duration<f64, std::milli>(std::chrono::high_resolution_clock::now() - _millisecondsPassed).count() };

    if(elapsedMilliseconds < _caret.GetBlinkTime())
        return;

    _caretBlinkState = !_caretBlinkState;
    _millisecondsPassed = std::chrono::high_resolution_clock::now();
}

void RetroFuturaGUI::ITextEditVisuals::updateCaretPosition()
{
    Text* text { getActiveText() };

    if(!text)
        return;

    const f32 fullHeight { getCaretHeight() };
    glm::vec3 caretPosition { text->GetBoundaryPosition(_caretPosition, fullHeight) };

    //In a column the caret lies across it, as wide as the column, between the characters above and below
    if(TextFlow::Horizontal != text->GetTextFlow())
    {
        const f32
            halfLength { text->GetLineHeight() * 0.5f },
            clippedLeftX { clampToTextBounds(caretPosition.x - halfLength) },
            clippedRightX { clampToTextBounds(caretPosition.x + halfLength) },
            clippedWidth { clippedRightX - clippedLeftX };

        _isCaretInView = 0.0f < clippedWidth && clampToTextBoundsY(caretPosition.y) == caretPosition.y;
        _caret.SetSize(glm::vec2(clippedWidth, _caretThickness));
        _caret.SetPosition(glm::vec3(clippedLeftX + clippedWidth * 0.5f, caretPosition.y, caretPosition.z));
        resetCaretBlink();
        return;
    }

    caretPosition.x = keepCaretVisible(caretPosition.x, _caretThickness * 0.5f);

    const f32
        clippedTopY { clampToTextBoundsY(caretPosition.y + fullHeight * 0.5f) },
        clippedBottomY { clampToTextBoundsY(caretPosition.y - fullHeight * 0.5f) },
        clippedHeight { clippedTopY - clippedBottomY };

    _isCaretInView = 0.0f < clippedHeight; //false while the caret's line is outside the text area
    caretPosition.y = clippedBottomY + clippedHeight * 0.5f;
    _caret.SetSize(glm::vec2(_caretThickness, clippedHeight));
    _caret.SetPosition(caretPosition);
    resetCaretBlink();
}

void RetroFuturaGUI::ITextEditVisuals::updateSelectedArea()
{
    _isSelected = false;
    _selectedLineAreas.clear();

    Text* text { getActiveText() };

    if(!text)
        return;

    const uSize
        left { getMarkedStart() },
        right { getMarkedEnd() };

    if(left == right) //nothing selected
        return;

    //One box per stretch drawn without a break, where the caret sits (it may be clipped shorter): mixed right-to-left and
    //left-to-right text splits a line's selection, and in columns the boxes run down them
    for(const TextSelectionArea& area : text->GetSelectionAreas(left, right, getCaretHeight()))
    {
        const f32
            clippedLeftX { clampToTextBounds(area._Center.x - area._Size.x * 0.5f) },
            clippedRightX { clampToTextBounds(area._Center.x + area._Size.x * 0.5f) },
            width { clippedRightX - clippedLeftX },
            clippedTopY { clampToTextBoundsY(area._Center.y + area._Size.y * 0.5f) },
            clippedBottomY { clampToTextBoundsY(area._Center.y - area._Size.y * 0.5f) },
            height { clippedTopY - clippedBottomY };

        if(0.0f >= width || 0.0f >= height) //skip whatever is outside the visible text area
            continue;

        _selectedLineAreas.push_back(SelectedArea::LineArea { ._Center = glm::vec2(clippedLeftX + width * 0.5f, clippedBottomY + height * 0.5f), ._Size = glm::vec2(width, height) });
    }

    _isSelected = !_selectedLineAreas.empty();
}

void RetroFuturaGUI::ITextEditVisuals::drawSelectedArea()
{
    if(!_isSelected)
        return;

    _selectedArea.Draw(_selectedLineAreas);
}

void RetroFuturaGUI::ITextEditVisuals::drawCaret()
{
    if(!_showCaret)
        return;

    if(!_isCaretInView)
        return;

    if(_caret.NeverBlinks() || _caretBlinkState)
        _caret.Draw();
}

f32 RetroFuturaGUI::ITextEditVisuals::clampToTextBounds(const f32 worldX, const f32 halfExtent) const
{
    const TextArea area { getTextArea() };
    const f32
        left { area._Left + halfExtent },
        right { area._Right - halfExtent };

    if(left > right) //the requested extent is wider than the text area itself
        return (area._Left + area._Right) * 0.5f;

    if(worldX < left)
        return left;

    if(worldX > right)
        return right;

    return worldX;
}

f32 RetroFuturaGUI::ITextEditVisuals::clampToTextBoundsY(const f32 worldY) const
{
    const TextArea area { getTextArea() };

    if(worldY < area._Bottom)
        return area._Bottom;

    if(worldY > area._Top)
        return area._Top;

    return worldY;
}

f32 RetroFuturaGUI::ITextEditVisuals::keepCaretVisible(const f32 worldX, const f32 halfExtent)
{
    Text* text { getActiveText() };

    if(!text)
        return worldX;

    const TextArea area { getTextArea() };
    const f32
        left { area._Left + halfExtent },
        right { area._Right - halfExtent },
        scrollOffset { text->GetScrollOffset() };

    if(left > right) //the requested extent is wider than the text area itself
        return (area._Left + area._Right) * 0.5f;

    if(worldX > right) //scroll the text left so the caret lands exactly on the edge
    {
        text->SetScrollOffset(scrollOffset + worldX - right);
        return right;
    }

    if(worldX < left && 0.0f < scrollOffset) //scroll back right, as far as there's room to
    {
        const f32
            wantedScrollOffset { scrollOffset + worldX - left },
            newScrollOffset { 0.0f < wantedScrollOffset ? wantedScrollOffset : 0.0f };

        text->SetScrollOffset(newScrollOffset);
        return worldX + scrollOffset - newScrollOffset; //shift the caret by as much as the text scrolled
    }

    return clampToTextBounds(worldX, halfExtent); //nothing left to scroll - fall back to a hard clamp
}

f32 RetroFuturaGUI::ITextEditVisuals::getCaretHeight() const
{
    Text* text { getActiveText() };

    if(!text)
        return 0.0f;

    return text->GetGlyphSize() * _caretHeightFactor;
}

RetroFuturaGUI::Caret& RetroFuturaGUI::ITextEditVisuals::GetCaret()
{
    return _caret;
}

const RetroFuturaGUI::Caret& RetroFuturaGUI::ITextEditVisuals::GetCaret() const
{
    return _caret;
}

RetroFuturaGUI::SelectedArea& RetroFuturaGUI::ITextEditVisuals::GetSelectedArea()
{
    return _selectedArea;
}

const RetroFuturaGUI::SelectedArea& RetroFuturaGUI::ITextEditVisuals::GetSelectedArea() const
{
    return _selectedArea;
}