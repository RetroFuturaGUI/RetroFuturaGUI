#include "ITextBox.hpp"

#if defined(TARGET_PLATFORM_LINUX)
    #define GLFW_EXPOSE_NATIVE_X11
#elif defined(TARGET_PLATFORM_WINDOWS)
    #define GLFW_EXPOSE_NATIVE_WIN32
#endif
#include <GLFW/glfw3native.h>

RetroFuturaGUI::ITextBox::ITextBox(std::string_view name, Projection* projection, IHierarchyNode* parentWidget, const WidgetTypeID parentWidgetTypeID, GLFWwindow* parentWindow)
   : IWidget(name, projection, parentWidget, parentWidgetTypeID, parentWindow)
{
    _background = std::make_unique<Rectangle>(projection);
    _border = std::make_unique<Rectangle>(projection);
    _text = std::make_unique<Text>(projection);
    _placeholderText = std::make_unique<Text>(projection);
    initEditVisuals(projection);

    if(!_background)
        return;

    if(!_border)
        return;

    if(!_placeholderText)
        return;

    _background->SetRectangleMode(RectangleMode::Plane);
    _border->SetRectangleMode(RectangleMode::Border);
    _placeholderText->SetColor(_placeholderTextColors[0]);
}

void RetroFuturaGUI::ITextBox::Draw()
{
    if(!_placeholderText)
        return;

    interact();
    drawBackground();
    drawBorder();
    drawSelectedArea();

    if(_text && !_text->GetTextUTF32().empty())
    {
        drawText();
        drawCaret();
        return;
    }

    _placeholderText->Draw();

    drawCaret();
}

void RetroFuturaGUI::ITextBox::SetSize(const glm::vec3& size)
{
    if(!_background)
        return;

    if(!_border)
        return;

    if(!_text)
        return;

    if(!_placeholderText)
        return;

    IWidget::SetSize(size);

    _background->SetSize(size);
    _border->SetSize(size);
    _text->SetParentSize(glm::vec2(size.x, size.y));
    _placeholderText->SetParentSize(glm::vec2(size.x, size.y));
}

void RetroFuturaGUI::ITextBox::SetPosition(const glm::vec3& position)
{
    if(!_background)
        return;

    if(!_selectedArea)
        return;

    if(!_border)
        return;

    if(!_text)
        return;

    if(!_placeholderText)
        return;

    if(!_caret)
        return;

    IWidget::SetPosition(position);

    _background->SetPosition(position);
    _selectedArea->SetPosition(position + glm::vec3(0.0f, 0.0f, 0.1f));
    _border->SetPosition(position + glm::vec3(0.0f, 0.0f, 0.2f));
    _text->SetPosition(position + glm::vec3(0.0f, 0.0f, 0.3f));
    _placeholderText->SetPosition(position + glm::vec3(0.0f, 0.0f, 0.3f));
    _caret->SetPosition(position + glm::vec3(0.0f, 0.0f, 0.5f));
}

void RetroFuturaGUI::ITextBox::SetRotation(const glm::vec3& rotation)
{
    if(!_background)
        return;

    if(!_border)
        return;

    if(!_text)
        return;

    if(!_placeholderText)
        return;

    _rotation = rotation;

    _background->SetRotation(rotation);
    _border->SetRotation(rotation);
    _text->SetRotation(rotation);
    _placeholderText->SetRotation(rotation);
}

void RetroFuturaGUI::ITextBox::interact()
{
    const PointerState pointer { readPointer() };
    glm::vec2 mousePos { pointer._Position };
    bool
        isMouseTextBoxPressed { pointer._LeftButtonPressed },
        isMouseInside { pointer._HasPosition && isPointInside(mousePos) };

    if(_editingEnabled && !_mouseEnteredFlag && !_isMarking && isMouseTextBoxPressed)
    {
        _editingEnabled = false;
        _showCaret = false;
    }

    editText();
    moveCaret();

    if(!_caretNeverBlinks)
        updateCaretBlink();

    if(pointer._IsCaptured)
        return; //ScrollView above has taken the pointer. Typing still works, the keyboard was handled above

    if(_isMarking)
    {
        if(isMouseTextBoxPressed && pointer._HasPosition)
        {
            requestCursorIcon(PlatformBridge::CursorIcon::Text); //also once the drag leaves the box
            //clamp before the hit-test, so the selection doesn't extend outside the visible text area
            //also consider scrolling when marking goes out of bounds
            _selectedPositionLast = _text->GetBoundaryAtPosition(mousePos);
            setCaretFromBoundary(_selectedPositionLast);
            updateSelectedArea();
        }
        else
        {
            _isMarking = false;
        }
    }

    if(!_isEnabledFlag || !isMouseInside) //no action and mouse leave
    {
        if(_mouseEnteredFlag)
        {
            _mouseEnteredFlag = false;
            _onMouseLeaveAsync.EmitAsync();
            _onMouseLeave.Emit();
            setColors(ColorState::Enabled);
        }

        if(!_editingEnabled)
            releaseKeyboardFocus();

        return;
    }

    bool isHovering = _isEnabledFlag && isMouseInside;
    if(isHovering) // hover
    {
        requestCursorIcon(PlatformBridge::CursorIcon::Text);
        _whileHoverAsync.EmitAsync();
        _whileHover.Emit();
    }

    if (isHovering && !_mouseLeftFlag && !_mouseEnteredFlag) //enter
    {
        _mouseEnteredFlag = true;
        _onMouseEnterAsync.EmitAsync();
        _onMouseEnter.Emit();
        setColors(ColorState::Hover);
    }

    if (isMouseTextBoxPressed && !_wasClicked) //click
    {
        _onClickAsync.EmitAsync();
        _onClick.Emit();
        setColors(ColorState::Clicked);
        _editingEnabled = true;
        takeKeyboardFocus();
        _isMarking = true;
        _selectedPositionFirst = _selectedPositionLast = _text->GetBoundaryAtPosition(glm::vec2(clampToTextBounds(mousePos.x), mousePos.y));
        setCaretFromBoundary(_selectedPositionFirst);
        updateSelectedArea();
        _showCaret = true;
    }
    else if(!isMouseTextBoxPressed && _wasClicked) //release
    {
        _onReleaseAsync.EmitAsync();
        _onRelease.Emit();

        if(isHovering)
            setColors(ColorState::Hover);
        else
            setColors(ColorState::Enabled);
    }

    _wasClicked = isMouseTextBoxPressed;
}

void RetroFuturaGUI::ITextBox::TakeKeyboardFocus()
{
    if(!_text)
        return;

    _editingEnabled = true;
    takeKeyboardFocus();
    _isMarking = false;
    _selectedPositionFirst = 0;
    _selectedPositionLast = _text->GetGlyphCount();
    _isSelected = 0 < _selectedPositionLast;
    setCaretFromBoundary(_selectedPositionLast);
    updateSelectedArea();
    _showCaret = true;
    resetCaretBlink();
}

void RetroFuturaGUI::ITextBox::setColors(const ColorState state)
{
    _backgroundColorState = state;
    _borderColorState = state;
    _textColorState = state;
    setBackgroundColors();
    setBorderColors();
    setTextColors();
}

void RetroFuturaGUI::ITextBox::SetCornerRadii(const glm::vec4& radii)
{
    _background->SetCornerRadii(radii);
    _border->SetCornerRadii(radii);
}

RetroFuturaGUI::ITextEditVisuals::TextArea RetroFuturaGUI::ITextBox::getTextArea() const
{
    return { ._Left = _position.x - _size.x * 0.5f, ._Bottom = _position.y - _size.y * 0.5f, ._Right = _position.x + _size.x * 0.5f, ._Top = _position.y + _size.y * 0.5f };
}
