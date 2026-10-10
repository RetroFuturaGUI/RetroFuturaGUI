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

    if (_background)
        _background->SetRectangleMode(RectangleMode::Plane);

    if (_border)
        _border->SetRectangleMode(RectangleMode::Border);

    if(_placeholderText)
        _placeholderText->SetColor(_placeholderTextColors[0]);
}

void RetroFuturaGUI::ITextBox::Draw()
{
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

    if(_placeholderText)
        _placeholderText->Draw();

    drawCaret();
}

void RetroFuturaGUI::ITextBox::SetEnabled(const bool enable, const bool emitSignal)
{
    _isEnabledFlag = enable;

    if(_isEnabledFlag)
    {
        if(emitSignal)
        {
            _onEnableAsync.EmitAsync();
            _onEnable.Emit();
        }

        setColors(ColorState::Enabled);
        return;
    }

    if(emitSignal)
    {
        _onDisableAsync.EmitAsync();
        _onDisable.Emit();
    }

    setColors(ColorState::Disabled);
}

void RetroFuturaGUI::ITextBox::SetSize(const glm::vec3& size)
{
    IWidget::SetSize(size);

    if(_background)
        _background->SetSize(size);

    if(_border)
        _border->SetSize(size);

    if(_text)
        _text->SetParentSize(glm::vec2(size.x, size.y));

    if(_placeholderText)
        _placeholderText->SetParentSize(glm::vec2(size.x, size.y));
}

void RetroFuturaGUI::ITextBox::SetPosition(const glm::vec3& position)
{
    IWidget::SetPosition(position);

    if(_background)
        _background->SetPosition(position);

    if(_selectedArea)
        _selectedArea->SetPosition(position + glm::vec3(0.0f, 0.0f, 0.1f));

    if(_border)
        _border->SetPosition(position + glm::vec3(0.0f, 0.0f, 0.2f));

    if(_text)
        _text->SetPosition(position + glm::vec3(0.0f, 0.0f, 0.3f));

    if(_placeholderText)
        _placeholderText->SetPosition(position + glm::vec3(0.0f, 0.0f, 0.3f));

    if(_caret)
        _caret->SetPosition(position + glm::vec3(0.0f, 0.0f, 0.5f));
}

void RetroFuturaGUI::ITextBox::SetRotation(const glm::vec3& rotation)
{
    _rotation = rotation;

    if(_background)
        _background->SetRotation(rotation);

    if(_border)
        _border->SetRotation(rotation);

    if(_text)
        _text->SetRotation(rotation);

    if(_placeholderText)
        _placeholderText->SetRotation(rotation);
}

void RetroFuturaGUI::ITextBox::interact()
{
    i32 mouseX { 0 }, mouseY { 0 };
    bool hasMousePosition { false };

#if defined(TARGET_PLATFORM_LINUX)
    hasMousePosition = PlatformBridge::Input::GetMouseWindowPosition(glfwGetX11Window(_parentWindow), mouseX, mouseY);
#elif defined(TARGET_PLATFORM_WINDOWS)
    hasMousePosition = PlatformBridge::Input::GetMouseWindowPosition(glfwGetWin32Window(_parentWindow), mouseX, mouseY);
#endif

    //PlatformBridge reports native (top-down) window coordinates; flip to this library's bottom-up world space here
    glm::vec2 mousePos { static_cast<f32>(mouseX), _projection.GetResolution().y - static_cast<f32>(mouseY) };
    bool isMouseTextBoxPressed = PlatformBridge::Input::IsMouseButtonDown(PlatformBridge::MouseButton::Left);
    bool isMouseInside = hasMousePosition && isPointInside(mousePos);

    if(_editingEnabled && !_mouseEnteredFlag && !_isMarking && isMouseTextBoxPressed)
    {
        _editingEnabled = false;
        _showCaret = false;
    }

    editText();
    moveCaret();

    if(!_caretNeverBlinks)
        updateCaretBlink();

    if(_isMarking)
    {
        if(isMouseTextBoxPressed && hasMousePosition)
        {
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
        {
            PlatformBridge::Input::SetActiveDisplay(nullptr);
            PlatformBridge::Input::SetActiveWindow(0);
        }

        return;
    }

    bool isHovering = _isEnabledFlag && isMouseInside;
    if(isHovering) // hover
    {
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
#if defined(TARGET_PLATFORM_LINUX)
        PlatformBridge::Input::SetActiveDisplay(glfwGetX11Display());
        PlatformBridge::Input::SetActiveWindow(glfwGetX11Window(_parentWindow));
#elif defined(TARGET_PLATFORM_WINDOWS)
        PlatformBridge::Input::SetActiveWindow(glfwGetWin32Window(_parentWindow));
#endif
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

RetroFuturaGUI::ITextEditVisuals::TextArea RetroFuturaGUI::ITextBox::textArea() const
{
    return { ._Left = _position.x - _size.x * 0.5f, ._Bottom = _position.y - _size.y * 0.5f, ._Right = _position.x + _size.x * 0.5f, ._Top = _position.y + _size.y * 0.5f };
}
