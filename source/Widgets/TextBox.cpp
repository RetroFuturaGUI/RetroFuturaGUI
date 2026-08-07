#include "TextBox.hpp"

#if defined(TARGET_PLATFORM_LINUX)
    #define GLFW_EXPOSE_NATIVE_X11
#elif defined(TARGET_PLATFORM_WINDOWS)
    #define GLFW_EXPOSE_NATIVE_WIN32
#endif
#include <GLFW/glfw3native.h>

RetroFuturaGUI::TextBox::TextBox(const std::string& name, Projection* projection, IWidget* parentWidget, const WidgetTypeID parentWidgetTypeID, GLFWwindow* parentWindow)
   : IWidget(name, projection, parentWidget, parentWidgetTypeID, parentWindow)
{
    _widgetTypeID = WidgetTypeID::Button;
    _background = std::make_unique<Rectangle>(projection);
    _border = std::make_unique<Rectangle>(projection);
    _text = std::make_unique<Text>(projection);
    _caret = std::make_unique<Rectangle>(projection);
    _selectedArea = std::make_unique<Rectangle>(projection);

    if (_background)
        _background->SetRectangleMode(RectangleMode::Plane);

    if (_border)
        _border->SetRectangleMode(RectangleMode::Border);

    if(_caret)
    {
        _caret->SetRectangleMode(RectangleMode::Plane);
        _caret->SetFillType(FillType::SOLID);
        _caret->SetColors(std::span<glm::vec4>(_caretColors.data(), _caretColors.size()));
    }

    if(_selectedArea)
    {
        _selectedArea->SetRectangleMode(RectangleMode::Plane);
        _selectedArea->SetFillType(FillType::SOLID);
        _selectedArea->SetColors(_selectedAreaColors);
    }
}

void RetroFuturaGUI::TextBox::Draw()
{
    interact();
    drawBackground();
    drawBorder();
    drawSelectedArea();
    drawText();
    drawCaret();
}

void RetroFuturaGUI::TextBox::SetEnabled(const bool enable, const bool emitSignal)
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

void RetroFuturaGUI::TextBox::SetSize(const glm::vec3& size)
{
    IWidget::SetSize(size);

    if(_background)
        _background->SetSize(size);

    if(_border)
        _border->SetSize(size);

    if(_text)
        _text->SetParentSize(glm::vec2(size.x, size.y));

    if(_caret)
        _caret->SetSize(glm::vec2(2.0f, _text->GetGlyphSize() * 1.6f));
}

void RetroFuturaGUI::TextBox::SetPosition(const glm::vec3& position)
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

    if(_caret)
        _caret->SetPosition(position + glm::vec3(0.0f, 0.0f, 0.5f));
}

void RetroFuturaGUI::TextBox::SetRotation(const glm::vec3& rotation)
{
    _rotation = rotation;

    if(_background)
        _background->SetRotation(rotation);

    if(_border)
        _border->SetRotation(rotation);

    if(_text)
        _text->SetRotation(rotation);
}

void RetroFuturaGUI::TextBox::interact()
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

    if(_editingEnabled && !_mouseEnteredFlag && isMouseTextBoxPressed)
    {
        _editingEnabled = false;
        _showCaret = false;
    }

    editText();
    moveCaret();

    if(_isMarking) //keep marking going even if the cursor drifts outside the widget bounds mid-drag
    {
        if(isMouseTextBoxPressed && hasMousePosition)
        {
            _markedPositionLast = _text->GetBoundaryAtPosition(mousePos.x);
            setCaretFromBoundary(_markedPositionLast);
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

        PlatformBridge::Input::SetActiveDisplay(nullptr);
        PlatformBridge::Input::SetActiveWindow(0);
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
        _markedPositionFirst = _markedPositionLast = _text->GetBoundaryAtPosition(mousePos.x);
        setCaretFromBoundary(_markedPositionFirst);
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

void RetroFuturaGUI::TextBox::setColors(const ColorState state)
{
    _backgroundColorState = state;
    _borderColorState = state;
    _textColorState = state;
    setBackgroundColors();
    setBorderColors();
    setTextColors();
}

void RetroFuturaGUI::TextBox::drawCaret()
{
    if(!_caret)
        return;

    if(_showCaret)
        _caret->Draw();
}

void RetroFuturaGUI::TextBox::drawSelectedArea()
{
    if(!_selectedArea)
        return;

    if(_isSelected)
        _selectedArea->Draw();
}

void RetroFuturaGUI::TextBox::updateSelectedArea()
{
    const uSize
        left { _markedPositionFirst < _markedPositionLast ? _markedPositionFirst : _markedPositionLast },
        right { _markedPositionFirst < _markedPositionLast ? _markedPositionLast : _markedPositionFirst };

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
    _selectedArea->SetPosition(glm::vec3(clippedLeftX + width * 0.5f, leftPosition.y, _position.z + 0.1f));
    _isSelected = true;
}

void RetroFuturaGUI::TextBox::setCaretFromBoundary(const uSize boundary)
{
    if(boundary == 0)
    {
        _caretPosition = 0;
        _caretRelativePosition = CaretRelativePosition::Left;
    }
    else
    {
        _caretPosition = boundary - 1;
        _caretRelativePosition = CaretRelativePosition::Right;
    }

    glm::vec3 caretPosition { _text->GetGlyphPosition(_caretPosition, _caretRelativePosition, _caret->GetSize().y) };
    caretPosition.x = clampToTextBounds(caretPosition.x, _caret->GetSize().x * 0.5f);
    _caret->SetPosition(caretPosition);
}

f32 RetroFuturaGUI::TextBox::clampToTextBounds(const f32 worldX, const f32 halfExtent) const
{
    const f32
        left { _position.x - _size.x * 0.5f + halfExtent },
        right { _position.x + _size.x * 0.5f - halfExtent };

    if(left > right) //requested extent is wider than the box itself
        return _position.x;

    if(worldX < left)
        return left;

    if(worldX > right)
        return right;

    return worldX;
}


void RetroFuturaGUI::TextBox::SetFontFamily(std::string_view fontFamily, const f32 fontSize, const PlatformBridge::Fonts::Slant slant, const PlatformBridge::Fonts::Weight fontWeight)
{
    ITextProperties::SetFontFamily(fontFamily, fontSize, slant, fontWeight);
    _caret->SetSize(glm::vec2(2.0f, fontSize * 1.6f));
}

void RetroFuturaGUI::TextBox::SetCornerRadii(const glm::vec4& radii)
{
    _background->SetCornerRadii(radii);
    _border->SetCornerRadii(radii);
}

void RetroFuturaGUI::TextBox::SetSelectedAreaColors(std::span<glm::vec4> colors)
{
    _selectedAreaColors.assign(colors.begin(), colors.end());

    if(_selectedArea)
        _selectedArea->SetColors(_selectedAreaColors);
}

void RetroFuturaGUI::TextBox::SetSelectedAreaFillType(const FillType fillType)
{
    if(_selectedArea)
        _selectedArea->SetFillType(fillType);
}
void RetroFuturaGUI::TextBox::SetSelectedAreaGradientAnimationSpeed(const f32 speed)
{
    if(_selectedArea)
        _selectedArea->SetGradientAnimationSpeed(speed);
}

void RetroFuturaGUI::TextBox::SetSelectedAreaGradientOffset(const f32 gradientOffset)
{
    if(_selectedArea)
        _selectedArea->SetGradientOffset(gradientOffset);
}

void RetroFuturaGUI::TextBox::SetSelectedAreaGradientDegree(const f32 degree)
{
    if(_selectedArea)
        _selectedArea->SetGradientDegree(degree);
}

void RetroFuturaGUI::TextBox::SetSelectedAreaGradientRotationSpeed(const f32 rotationSpeed)
{
    if(_selectedArea)
        _selectedArea->SetGradientRotationSpeed(rotationSpeed);
}