#include "Tooltip.hpp"
#include "FontManager.hpp"
#include "ITextProperties.hpp"
#include "IWidget.hpp"
#include "PlatformBridge.hpp"
#include <algorithm>
#include <memory>
#include <string_view>

RetroFuturaGUI::Tooltip::Tooltip(Projection* projection, IWidget* widget)
{
    _projection = projection;
    _widget = widget;
    _background = std::make_unique<Plane>(_projection);
    _border = std::make_unique<Frame>(_projection);
    _text = std::make_unique<Text>(projection);

    if(!_background)
        return;

    if(!_border)
        return;

    if(!_text)
        return;

    _border->SetBorderWidth(_defaultBorderWidth);
    _text->SetTextPadding(_defaultTextPadding);
    setBackgroundColors();
    setBorderColors();
    setTextColors();
    applyDefaultFont();
}

RetroFuturaGUI::Tooltip::~Tooltip()
{
    if(this == _shownTooltip)
        _shownTooltip = nullptr;

    if(this == _asideTooltip)
        _asideTooltip = nullptr;
}

void RetroFuturaGUI::Tooltip::Update()
{
    interact();

    if(!_isShown)
        return;

    _shownTooltip = this;
}

void RetroFuturaGUI::Tooltip::DrawShown()
{
    if(!_shownTooltip)
        return;

    glClear(GL_DEPTH_BUFFER_BIT);
    _shownTooltip->draw();
    _shownTooltip = nullptr;
}

void RetroFuturaGUI::Tooltip::setShownAside()
{
    _asideTooltip = _shownTooltip;
    _shownTooltip = nullptr;
}

void RetroFuturaGUI::Tooltip::restoreShownFromAside()
{
    _shownTooltip = _asideTooltip;
    _asideTooltip = nullptr;
}

void RetroFuturaGUI::Tooltip::SetDefaultFontFamily(std::string_view fontFamily, const f32 fontSize, const PlatformBridge::Fonts::Slant slant, const PlatformBridge::Fonts::Weight fontWeight)
{
    _defaultFontFamily = fontFamily;
    _defaultFontSize = fontSize;
    _defaultFontSlant = slant;
    _defaultFontWeight = fontWeight;
    ++_defaultFontVersion;

    FontManager::GetFontInfo(fontFamily, fontSize, slant, fontWeight);
}

void RetroFuturaGUI::Tooltip::draw()
{
    applyDefaultFont();
    setPosition();
    drawBackground();
    drawBorder();
    drawText();
}

void RetroFuturaGUI::Tooltip::SetBackgroundColors(std::span<const glm::vec4> colors)
{
    if(!_background)
        return;

    _backgroundColors.assign(colors.begin(), colors.end());
    _background->SetColors(_backgroundColors);
}

void RetroFuturaGUI::Tooltip::SetBorderColors(std::span<const glm::vec4> colors)
{
    if(!_border)
        return;
    
    _borderColors.assign(colors.begin(), colors.end());
    _border->SetColors(_borderColors);
}

void RetroFuturaGUI::Tooltip::SetCornerRadii(const glm::vec4& radii)
{
    if(!_background)
        return;
    
    if(!_border)
        return;
    
    _background->SetCornerRadii(radii);
    _border->SetCornerRadii(radii);
}

void RetroFuturaGUI::Tooltip::SetText(std::string_view text, const bool emitSignal)
{
    ITextProperties::SetText(text, emitSignal);
    setSize();
}

void RetroFuturaGUI::Tooltip::SetFontFamily(std::string_view fontFamily, const f32 fontSize, const PlatformBridge::Fonts::Slant slant, const PlatformBridge::Fonts::Weight fontWeight)
{
    _hasOwnFont = true;
    ITextProperties::SetFontFamily(fontFamily, fontSize, slant, fontWeight);
    setSize();
}

void RetroFuturaGUI::Tooltip::applyDefaultFont()
{
    if(_hasOwnFont)
        return;

    if(_defaultFontVersion == _appliedDefaultFontVersion)
        return;

    ITextProperties::SetFontFamily(_defaultFontFamily, _defaultFontSize, _defaultFontSlant, _defaultFontWeight);
    setSize();
    _appliedDefaultFontVersion = _defaultFontVersion;
}

void RetroFuturaGUI::Tooltip::SetTextPadding(const f32 padding)
{
    ITextProperties::SetTextPadding(padding);
    setSize(); //the padding is part of the box
}

void RetroFuturaGUI::Tooltip::SetBorderWidth(const f32 borderWidth)
{
    IBorder::SetBorderWidth(borderWidth);
    setSize();
}

void RetroFuturaGUI::Tooltip::setSize()
{
    if(!_text)
        return;

    if(!_background)
        return;

    if(!_border)
        return;

    const f32 padding { _text->GetPadding() };
    const glm::vec2
        textArea { _text->GetWidth() + padding * 2.0f, _text->GetHeight() + padding * 2.0f }, //exactly the text plus its padding, so any alignment centers it
        size { textArea + glm::vec2(_border->GetBorderWidth() * 2.0f) }; //the border draws inside the box
    _text->SetParentSize(textArea);
    _background->SetSize(size);
    _border->SetSize(size);
}

void RetroFuturaGUI::Tooltip::setPosition()
{
    if(!_widget)
        return;

    if(!_projection)
        return;

    if(!_background)
        return;

    if(!_border)
        return;

    if(!_text)
        return;

    const glm::vec2
        halfSize { _background->GetSize() * 0.5f },
        resolution { _projection->GetResolution() };

    // Centered above the cursor's tip and kept inside the window: it slides sideways at the left and right edges
    // and down at the top one. World space runs bottom-up, so above is +Y.
    // Not std::clamp: a tooltip larger than the window would make it undefined.
    const glm::vec3 center {
        std::max(halfSize.x, std::min(_pointerPosition.x, resolution.x - halfSize.x)),
        std::max(halfSize.y, std::min(_pointerPosition.y + _cursorDistance + halfSize.y, resolution.y - halfSize.y)),
        _widget->GetPosition().z };

    _background->SetPosition(center);
    _border->SetPosition(center + glm::vec3(0.0f, 0.0f, _zStep));
    _text->SetPosition(center + glm::vec3(0.0f, 0.0f, _zStep * 2.0f));
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

    _pointerPosition = pointer._Position;

    if(!_isHovering) //mouse entered, the delay starts
    {
        _isHovering = true;
        _hoverStart = std::chrono::steady_clock::now();
    }

    if(pointer._LeftButtonPressed)
        _isDismissed = true;

    _isShown = !_isDismissed && _showDelay <= std::chrono::steady_clock::now() - _hoverStart;
}