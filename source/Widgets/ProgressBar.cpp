#include "ProgressBar.hpp"
#include "Input.hpp"
#include "PlatformBridge.hpp"
#include <memory>

#if defined(TARGET_PLATFORM_LINUX)
    #define GLFW_EXPOSE_NATIVE_X11
#elif defined(TARGET_PLATFORM_WINDOWS)
    #define GLFW_EXPOSE_NATIVE_WIN32
#endif
#include <GLFW/glfw3native.h>

RetroFuturaGUI::ProgressBar::ProgressBar(const std::string& name, Projection* projection, IWidget* parentWidget, const WidgetTypeID parentWidgetTypeID, GLFWwindow* parentWindow)
   : IRangedSingleValue(name, projection, parentWidget, parentWidgetTypeID, parentWindow)
{
    _widgetTypeID = WidgetTypeID::ProgressBar;
}

void  RetroFuturaGUI::ProgressBar::Draw()
{
    if(!_background)
        return;

    if(!_border)
        return;

    interact();

    _background->Draw();
    _border->Draw();

    drawGraph();
    drawIndicator();
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
    if(!_graph)
        return false;

    return isPointInsideRect(mousePos, glm::vec3(_graph->GetSize(), 0.0f), _graph->GetPosition(), _graph->GetRotation());
}

void RetroFuturaGUI::ProgressBar::interact()
{
    const PointerState pointer { readPointer() };

    if(pointer._IsCaptured)
        return; //ScrollView above has taken the pointer

    glm::vec2 mousePos { pointer._Position };
    bool
        isMouseButtonPressed { pointer._LeftButtonPressed },
        isMouseInside { pointer._HasPosition && isPointInsideRect(mousePos, _size, _position, _rotation) },
        isMouseInsideGraph { isInsideGraph(mousePos) };

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