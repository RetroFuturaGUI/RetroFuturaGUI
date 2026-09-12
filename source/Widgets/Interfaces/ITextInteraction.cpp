#include "ITextInteraction.hpp"
#include "PlatformBridge.hpp"

#if defined(TARGET_PLATFORM_LINUX)
    #define GLFW_EXPOSE_NATIVE_X11
#elif defined(TARGET_PLATFORM_WINDOWS)
    #define GLFW_EXPOSE_NATIVE_WIN32
#endif
#include <GLFW/glfw3native.h>

bool RetroFuturaGUI::ITextInteraction::hasInputFocus() const
{
    const uint64_t activeWindowId { PlatformBridge::Input::GetActiveWindowID() };

#if defined(TARGET_PLATFORM_LINUX)
    return activeWindowId == static_cast<uint64_t>(glfwGetX11Window(_parentWindow));
#elif defined(TARGET_PLATFORM_WINDOWS)
    return activeWindowId == reinterpret_cast<uint64_t>(glfwGetWin32Window(_parentWindow));
#else
    return false;
#endif
}

void RetroFuturaGUI::ITextInteraction::updateCaretBlink()
{
    if(!_showCaret)
    {
        resetCaretBlink();
        return;
    }

    const f64 elapsedMilliseconds { std::chrono::duration<f64, std::milli>(std::chrono::high_resolution_clock::now() - _millisecondsPassed).count() };

    if(elapsedMilliseconds < _blinkForMilliseconds)
        return;

    _caretBlinkState = !_caretBlinkState;
    _millisecondsPassed = std::chrono::high_resolution_clock::now();
}

void RetroFuturaGUI::ITextInteraction::resetCaretBlink()
{
    _caretBlinkState = true;
    _millisecondsPassed = std::chrono::high_resolution_clock::now();
}

void RetroFuturaGUI::ITextInteraction::deselect()
{
    _isMarking = false;
    _isSelected = false;
}

uSize RetroFuturaGUI::ITextInteraction::markedStart() const
{
    return _selectedPositionFirst < _selectedPositionLast ? _selectedPositionFirst : _selectedPositionLast;
}

uSize RetroFuturaGUI::ITextInteraction::markedEnd() const
{
    return _selectedPositionFirst < _selectedPositionLast ? _selectedPositionLast : _selectedPositionFirst;
}

bool RetroFuturaGUI::ITextInteraction::shouldRepeat(const u32 holdFrames)
{
    return holdFrames >= _keyRepeatInitialDelay && (holdFrames - _keyRepeatInitialDelay) % _keyRepeatInterval == 0;
}
