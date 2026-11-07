#include "ITextEditable.hpp"
#include <array>
#include "Clipboard.hpp"
#include "PlatformBridge.hpp"

#if defined(TARGET_PLATFORM_LINUX)
    #define GLFW_EXPOSE_NATIVE_X11
#elif defined(TARGET_PLATFORM_WINDOWS)
    #define GLFW_EXPOSE_NATIVE_WIN32
#endif
#include <GLFW/glfw3native.h>

RetroFuturaGUI::ITextEditable::ITextEditable(Projection* projection)
    : ITextEditVisuals(projection), _placeholder(projection), _text(projection, [this](const WidgetTextChange change) { textChanged(change); }, &_placeholder)
{
    _placeholder.SetColors(std::array { glm::vec4(0.5f, 0.5f, 0.5f, 1.0f) }, ColorState::Enabled);
}

void RetroFuturaGUI::ITextEditable::EnableReadOnly(const bool enable)
{
    _readOnly = enable;
}

bool RetroFuturaGUI::ITextEditable::IsReadOnly() const
{
    return _readOnly;
}

void RetroFuturaGUI::ITextEditable::emitChange()
{
    if(_enterPressed)
        return;

    _text.EmitChange();
}

RetroFuturaGUI::WidgetText& RetroFuturaGUI::ITextEditable::GetText()
{
    return _text;
}

const RetroFuturaGUI::WidgetText& RetroFuturaGUI::ITextEditable::GetText() const
{
    return _text;
}

RetroFuturaGUI::WidgetText& RetroFuturaGUI::ITextEditable::GetPlaceholder()
{
    return _placeholder;
}

const RetroFuturaGUI::WidgetText& RetroFuturaGUI::ITextEditable::GetPlaceholder() const
{
    return _placeholder;
}

RetroFuturaGUI::Text* RetroFuturaGUI::ITextEditable::getActiveText() const
{
    return &_text.GetElement();
}

bool RetroFuturaGUI::ITextEditable::isTextReadOnly() const
{
    return _readOnly;
}