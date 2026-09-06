#include "ProgressBar.hpp"
#include "IWidget.hpp"
#include "Input.hpp"
#include "PlatformBridge.hpp"
#include "Rectangle.hpp"
#include <memory>

#if defined(TARGET_PLATFORM_LINUX)
    #define GLFW_EXPOSE_NATIVE_X11
#elif defined(TARGET_PLATFORM_WINDOWS)
    #define GLFW_EXPOSE_NATIVE_WIN32
#endif
#include <GLFW/glfw3native.h>

RetroFuturaGUI::ProgressBar::ProgressBar(const std::string& name, Projection* projection, IWidget* parentWidget, const WidgetTypeID parentWidgetTypeID, GLFWwindow* parentWindow)
   : IWidget(name, projection, parentWidget, parentWidgetTypeID, parentWindow)
{
    _widgetTypeID = WidgetTypeID::ProgressBar;
    _background = std::make_unique<Rectangle>(projection);
    _border = std::make_unique<Rectangle>(projection);
    _graph = std::make_unique<Rectangle>(projection);

    if (_background)
        _background->SetRectangleMode(RectangleMode::Plane);

    if (_border)
        _border->SetRectangleMode(RectangleMode::Border);

    if (_graph)
        _graph->SetRectangleMode(RectangleMode::Plane);

    _track = _background.get();
    _elementProjection = projection;
    _useGraph = true;
}

void  RetroFuturaGUI::ProgressBar::Draw()
{
    interact();

    if (_background)
        _background->Draw();

    if (_border)
        _border->Draw();

    drawGraph();
    drawIndicator();
}

void RetroFuturaGUI::ProgressBar::SetPosition(const glm::vec3& position)
{
    IWidget::SetPosition(position);

    if (_background)
        _background->SetPosition(position);

    if (_border)
        _border->SetPosition(position + glm::vec3(0.0f, 0.0f, 0.01f));

    setGraphPosition();
    setIndicatorPosition();
}

void RetroFuturaGUI::ProgressBar::SetSize(const glm::vec3& size)
{
    IWidget::SetSize(size);

    if (_background)
        _background->SetSize(size);

    if (_border)
        _border->SetSize(size);

    setGraphPosition(); // internally calls setGraphSize() first
    setIndicatorPosition();
}

void RetroFuturaGUI::ProgressBar::SetRotation(const glm::vec3& rotation)
{
    IWidget::SetRotation(rotation);
    const glm::vec3 trackRotation { orientedRotation(rotation) };

    if (_background)
        _background->SetRotation(trackRotation);

    if (_border)
        _border->SetRotation(trackRotation);

    setGraphPosition();
    setIndicatorPosition();
}

void RetroFuturaGUI::ProgressBar::SetOrientation(const Orientation orientation)
{
    IRangedValue::SetOrientation(orientation);
    SetRotation(_rotation);
}

void RetroFuturaGUI::ProgressBar::SetCornerRadii(const glm::vec4& radii)
{
    if(_background)
        _background->SetCornerRadii(radii);

    if(_border)
        _border->SetCornerRadii(radii);
}

void RetroFuturaGUI::ProgressBar::SetIndicatorSize(const glm::vec2& size)
{
    if(_indicatorBackground)
        _indicatorBackground->SetSize(size);

    SetIndicatorType(_indicatorType); // reapply rounding (if Circle) at the new size
    setIndicatorPosition();
}

void RetroFuturaGUI::ProgressBar::setColors(const ColorState state)
{
    _backgroundColorState = state;
    _borderColorState = state;
    setBackgroundColors();
    setBorderColors();
}

bool RetroFuturaGUI::ProgressBar::isInsideGraph(const glm::vec2& mousePos)
{
    return isPointInsideRect(mousePos, glm::vec3(_graph->GetSize(), 0.0f), _graph->GetPosition(), _graph->GetRotation());
}

void RetroFuturaGUI::ProgressBar::interact()
{
    i32
        mouseX { 0 },
        mouseY { 0 };
    bool hasMousePosition { false };

#if defined(TARGET_PLATFORM_LINUX)
    hasMousePosition = PlatformBridge::Input::GetMouseWindowPosition(glfwGetX11Window(_parentWindow), mouseX, mouseY);
#elif defined(TARGET_PLATFORM_WINDOWS)
    hasMousePosition = PlatformBridge::Input::GetMouseWindowPosition(glfwGetWin32Window(_parentWindow), mouseX, mouseY);
#endif

    glm::vec2 mousePos { static_cast<f32>(mouseX), _projection.GetResolution().y - static_cast<f32>(mouseY) };
    bool isMouseButtonPressed = PlatformBridge::Input::IsMouseButtonDown(PlatformBridge::MouseButton::Left);
    bool isMouseInside = hasMousePosition && isPointInsideRect(mousePos, _size, _position, orientedRotation(_rotation));
    bool isMouseInsideGraph = isPointInsideRect(mousePos, glm::vec3(_graph->GetSize(), 0.0f), _graph->GetPosition(), _graph->GetRotation());

    if(!_isEnabledFlag || !isMouseInside) //no action and mouse leave
    {
        if(_mouseEnteredFlag)
        {
            _mouseEnteredFlag = false;
            _onMouseLeaveAsync.EmitAsync();
            _onMouseLeave.Emit();
            setColors(ColorState::Enabled);
            setGraphColors(ColorState::Enabled);
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

        if(isMouseInsideGraph)
            setGraphColors(ColorState::Hover);
    }

    if (isMouseButtonPressed && !_wasClicked) //click
    {
        _onClickAsync.EmitAsync();
        _onClick.Emit();
        setColors(ColorState::Clicked);
        setGraphColors(ColorState::Clicked);
    }
    else if(!isMouseButtonPressed && _wasClicked) //release
    {
        _onReleaseAsync.EmitAsync();
        _onRelease.Emit();

        if(isHovering)
        {
            setColors(ColorState::Hover);

            if(isMouseInsideGraph)
                setGraphColors(ColorState::Hover);
        }
        else
        {
            setColors(ColorState::Enabled);

            if(isMouseInsideGraph)
                setGraphColors(ColorState::Enabled);
        }
    }

    _wasClicked = isMouseButtonPressed;
}