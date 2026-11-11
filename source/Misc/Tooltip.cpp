#include "Tooltip.hpp"
#include "Background.hpp"
#include "FontManager.hpp"
#include "WidgetText.hpp"
#include "IWidget.hpp"
#include "IncludeHelper.hpp"
#include "PlatformBridge.hpp"
#include "EnvironmentView.hpp"
#include "Window.hpp"
#include <algorithm>
#include <memory>
#include <string_view>

RetroFuturaGUI::Tooltip::Tooltip(Projection* projection, IWidget* widget)
    : _hostProjection(*projection)
{
    _widget = widget;

    if(!_widget)
        return;

    _hostWindow = _widget->GetParentWindow();
    useWidgetTransitions();

    _border.SetWidth(_defaultBorderWidth);
    _text.SetPadding(_defaultTextPadding);
    _text.SetColorState(ColorState::Enabled);
    applyDefaultFont();
}

RetroFuturaGUI::Tooltip::~Tooltip()
{
    std::erase_if(_shownTooltips, [this](const auto& shown) { return this == shown.second; });
}

void RetroFuturaGUI::Tooltip::Update()
{
    if(!_widget)
        return;

    interact();

    //Fades in once shown and out once hidden, over the widget's Opacity transition, and draws until it has faded out
    const f32 target { _isShown ? 1.0f : 0.0f };

    if(target != _opacity.GetTarget())
        _opacity.Set(target, Transitions::Resolve(_widget->_transitionSettings, TransitionProperty::Opacity));

    if(0.0f >= _opacity.Get())
        return;

    _shownTooltips[_hostWindow] = this; //only where the cursor is
}

void RetroFuturaGUI::Tooltip::DrawShown(GLFWwindow* window)
{
    const auto shown { _shownTooltips.find(window) };

    if(_shownTooltips.end() == shown)
        return;

    Tooltip* tooltip { shown->second };
    _shownTooltips.erase(shown);

    if(!tooltip)
        return;

    Shader::ClearDepth();
    const Shader::OpacityScope opacity { tooltip->_opacity.Get() };
    tooltip->draw();
}

void RetroFuturaGUI::Tooltip::SetDefaultFontFamily(std::string_view fontFamily, const f32 fontSize, const FontSizeUnit sizeUnit, const PlatformBridge::Fonts::Slant slant, const PlatformBridge::Fonts::Weight fontWeight)
{
    _defaultFontFamily = fontFamily;
    _defaultFontSize = FontManager::ToPixels(fontSize, sizeUnit);
    _defaultFontSlant = slant;
    _defaultFontWeight = fontWeight;
    ++_defaultFontVersion;

    FontManager::GetFontInfo(fontFamily, fontSize, sizeUnit, slant, fontWeight);
}

void RetroFuturaGUI::Tooltip::draw()
{
    followHostWindow();
    applyDefaultFont();
    setPosition();
    _background.Draw();
    _border.Draw();
    _text.Draw();
}

void RetroFuturaGUI::Tooltip::SetFontFamily(std::string_view fontFamily, const f32 fontSize, const FontSizeUnit sizeUnit, const PlatformBridge::Fonts::Slant slant, const PlatformBridge::Fonts::Weight fontWeight)
{
    _hasOwnFont = true;
    _text.SetFontFamily(fontFamily, fontSize, sizeUnit, slant, fontWeight);
    setSize();
}

void RetroFuturaGUI::Tooltip::useWidgetTransitions()
{
    if(!_widget)
        return;

    _background.SetTransitionSettings(&_widget->_transitionSettings);
    _border.SetTransitionSettings(&_widget->_transitionSettings);
    _text.SetTransitionSettings(&_widget->_transitionSettings);
}

void RetroFuturaGUI::Tooltip::applyDefaultFont()
{
    if(_hasOwnFont)
        return;

    if(_defaultFontVersion == _appliedDefaultFontVersion)
        return;

     const std::string family { _defaultFontFamily.empty() ? FontManager::GetDefaultFontFamily() : _defaultFontFamily };
    _text.SetFontFamily(family, _defaultFontSize, FontSizeUnit::Pixels, _defaultFontSlant, _defaultFontWeight);
    setSize();
    _appliedDefaultFontVersion = _defaultFontVersion;
}

void RetroFuturaGUI::Tooltip::followHostWindow()
{
    const Window* host { Window::FromGlfwWindow(_hostWindow) };

    if(!host)
        return;

    const Projection* projection { host->GetProjection() };

    if(!projection)
        return;

    _hostProjection = *projection;
}

void RetroFuturaGUI::Tooltip::setSize()
{
    const f32 padding { _text.GetPadding() };
    const glm::vec2
        textArea { _text.GetWidth() + padding * 2.0f, _text.GetElement().GetHeight() + padding * 2.0f }, //exactly the text plus its padding, so any alignment centers it
        size { textArea + glm::vec2(_border.GetWidth() * 2.0f) }; //the border draws inside the box
    _text.SetSize(textArea);
    _background.SetSize(size);
    _border.SetSize(size);
}

void RetroFuturaGUI::Tooltip::setPosition()
{
    if(!_widget)
        return;

    const glm::vec2
        halfSize { _background.GetSize() * 0.5f },
        resolution { _hostProjection.GetResolution() };

    // Centered above the cursor's tip and kept inside the window. It slides sideways at the left and right edges
    // and down at the top one. World space runs bottom-up, so above is +Y
    // Not std::clamp: a tooltip larger than the window would make it undefined
    const glm::vec3 center {
        std::max(halfSize.x, std::min(_pointerPosition.x, resolution.x - halfSize.x)),
        std::max(halfSize.y, std::min(_pointerPosition.y + _cursorDistance + halfSize.y, resolution.y - halfSize.y)),
        _depth };

    _background.SetPosition(center);
    _border.SetPosition(center + glm::vec3(0.0f, 0.0f, _zStep));
    _text.SetPosition(center + glm::vec3(0.0f, 0.0f, _zStep * 2.0f));
}

void RetroFuturaGUI::Tooltip::interact()
{
    if(!_widget)
        return;

    const PointerState pointer { _widget->readPointer() };
    const bool isHovering { pointer._HasPosition && !pointer._IsCaptured && _widget->isPointInside(pointer._Position) };

    if(!isHovering)
    {
        _isHovering = false;
        _isShown = false;
        _isDismissed = false;
        return;
    }

    //At the cursor in the window it is in: through a view, the view's window, not the world's
    const std::optional<EnvironmentView::ViewPointer>& viewPointer { EnvironmentView::getViewPointer() };
    const bool isThroughView { viewPointer && _widget->isInside(viewPointer->_World) };
    _hostWindow = isThroughView ? viewPointer->_Window : _widget->GetParentWindow();
    _pointerPosition = isThroughView ? viewPointer->_ViewWindowPoint : pointer._WindowPosition; //stays always flat!
    _depth = isThroughView ? 0.0f : _widget->GetPosition().z;

    if(!_isHovering) //mouse entered, the delay starts
    {
        _isHovering = true;
        _hoverStart = std::chrono::steady_clock::now();
    }

    if(pointer._LeftButtonPressed)
        _isDismissed = true;

    _isShown = !_isDismissed && _showDelay <= std::chrono::steady_clock::now() - _hoverStart;
}

RetroFuturaGUI::Background& RetroFuturaGUI::Tooltip::GetBackground()
{
    return _background;
}

const RetroFuturaGUI::Background& RetroFuturaGUI::Tooltip::GetBackground() const
{
    return _background;
}

RetroFuturaGUI::Border& RetroFuturaGUI::Tooltip::GetBorder()
{
    return _border;
}

const RetroFuturaGUI::Border& RetroFuturaGUI::Tooltip::GetBorder() const
{
    return _border;
}

RetroFuturaGUI::WidgetText& RetroFuturaGUI::Tooltip::GetText()
{
    return _text;
}

const RetroFuturaGUI::WidgetText& RetroFuturaGUI::Tooltip::GetText() const
{
    return _text;
}