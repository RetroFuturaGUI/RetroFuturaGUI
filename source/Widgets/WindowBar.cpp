#include "WindowBar.hpp"
#include "Window.hpp"

RetroFuturaGUI::WindowBar::WindowBar(const std::string& name, Projection* projection, IWidget* parentWidget, const WidgetTypeID parentWidgetTypeID, GLFWwindow* parentWindow)
    : IWidget(name, projection, parentWidget, parentWidgetTypeID, parentWindow), _projection(static_cast<Projection&>(*projection))
{
    _widgetTypeID = WidgetTypeID::WindowBar;
    _position = calculateWindowBarPosition();
    _size = calculateWindowBarSize();
    _background = std::make_unique<Rectangle>(const_cast<Projection*>(&_projection));

    if(_background)
    {
        _background->SetRectangleMode(RectangleMode::Plane);
        _background->SetPosition(_position);
        _background->SetSize(_size);
        _background->SetRotation(0.0f);
    }
}

void RetroFuturaGUI::WindowBar::Draw()
{
    if(_background)
        _background->Draw();

    if(_close)
        _close->Draw();
    
    if(_maximize)
        _maximize->Draw();
    
    if(_minimize)
        _minimize->Draw();
    
    if(_windowTitle)
        _windowTitle->Draw();
}

bool RetroFuturaGUI::WindowBar::WindowShouldClose()
{
    return _windowShouldClose;
}

void RetroFuturaGUI::WindowBar::Resize()
{
    _size = calculateWindowBarSize();
    _position = calculateWindowBarPosition();
    
    if(_background)
    {
        _background->SetSize(_size);
        _background->SetPosition(_position);
    }

    if(_close)
        _close->SetPosition(calculateElementPosition(ElementType::CloseButton));
    
    if(_maximize)
        _maximize->SetPosition(calculateElementPosition(ElementType::MaximizeButton));
    
    if(_minimize)
        _minimize->SetPosition(calculateElementPosition(ElementType::MinimizeButton));

    if(_windowTitle)
    {
        glm::vec2 titleParentSize = calculateTitleParentSize();
        glm::vec3 titleAnchor = calculateElementPosition(ElementType::Title);
        _windowTitle->SetPosition(glm::vec3(titleAnchor.x + titleParentSize.x * 0.5f, titleAnchor.y, titleAnchor.z));
        _windowTitle->SetParentSize(titleParentSize);
    }
}

glm::vec3 RetroFuturaGUI::WindowBar::calculateWindowBarPosition()
{
    f32 
        x { 0.0f }, 
        y { 0.0f };

    if(_windowBarPosition == WindowBarPosition::Top || _windowBarPosition == WindowBarPosition::Bottom)
        x = _projection.GetResolution().x * 0.5f;
    else
        y = _projection.GetResolution().y * 0.5f;

    switch(_windowBarPosition)
    {
    case WindowBarPosition::Bottom:
        y = _windowBarThiccness * 0.5f;
    break;
    /*case WindowBarPosition::Left:
        x = _windowBarThiccness * 0.5f;
    break;
    case WindowBarPosition::Right:
        x = _projection.GetResolution().x - _windowBarThiccness * 0.5f;*/
    default: //Top
        y = _projection.GetResolution().y - _windowBarThiccness * 0.5f;
    }

    return glm::vec3(x, y, _position.z);
}

glm::vec3 RetroFuturaGUI::WindowBar::calculateWindowBarSize()
{
    return glm::vec3(_windowBarPosition == WindowBarPosition::Top || _windowBarPosition == WindowBarPosition::Bottom ? _projection.GetResolution().x : _windowBarThiccness/*,
                     _windowBarPosition == WindowBarPosition::Left || _windowBarPosition == WindowBarPosition::Right ? _projection.GetResolution().y : _windowBarThiccness*/, _windowBarThiccness,
                    _size.z);
}

glm::vec3 RetroFuturaGUI::WindowBar::calculateElementPosition(const ElementType elementType)
{
    f32
        offset { 3.0f }, 
        x { 0.0f }, 
        y { 0.0f };

    switch(elementType)
    {
        case ElementType::NoDockingDrag:
            offset *= 3.5f;
            offset +=_windowBarThiccness * 3.5f;
        break;
        case ElementType::MinimizeButton:
            offset *=  2.5f;
            offset += _windowBarThiccness * 2.5f;
        break;
        case ElementType::MaximizeButton:
            offset *=  1.5f;
            offset += _windowBarThiccness * 1.5f;
        break;
        case ElementType::CloseButton:
            offset += _windowBarThiccness * 0.5f;
        break;
        default: //TITLE
        break;
    }

    switch(elementType)
    {
        case ElementType::Icon:
            if(_windowBarPosition == WindowBarPosition::Top || _windowBarPosition == WindowBarPosition::Bottom)
                x =  _windowBarThiccness * 0.5f + offset;
            else
                y =  _windowBarThiccness * 0.5f + offset;

            switch(_windowBarPosition)
            {
            case WindowBarPosition::Bottom:
                y = _windowBarThiccness * 0.5f;
            break;
            /*case WindowBarPosition::Left:
                x = _windowBarThiccness * 0.5f;
            break;
            case WindowBarPosition::Right:
                x = _projection.GetResolution().x - _windowBarThiccness * 0.5f;*/
            default: //Top
                y = _projection.GetResolution().y - _windowBarThiccness * 0.5f;
            }
        break;
        case ElementType::Title:
            if(_windowBarPosition == WindowBarPosition::Top || _windowBarPosition == WindowBarPosition::Bottom)
                x =  _windowBarThiccness * 1.5f + offset;
            else
                y =  _windowBarThiccness * 1.5f + offset;

            switch(_windowBarPosition)
            {
            case WindowBarPosition::Bottom:
                y = _windowBarThiccness * 0.5f;
            break;
            /*case WindowBarPosition::Left:
                x = _windowBarThiccness * 0.5f;
            break;
            case WindowBarPosition::Right:
                x = _projection.GetResolution().x - _windowBarThiccness * 0.5f;*/
            default: //Top
                y = _projection.GetResolution().y - _windowBarThiccness * 0.5f;
            }
        break;
        default:
        {
            if(_windowBarPosition == WindowBarPosition::Top || _windowBarPosition == WindowBarPosition::Bottom)
                x = _projection.GetResolution().x - offset;
            else
                y = _projection.GetResolution().y - offset;

            switch(_windowBarPosition)
            {
            case WindowBarPosition::Bottom:
                y = _windowBarThiccness * 0.5f;
            break;
            /*case WindowBarPosition::Left:
                x = _windowBarThiccness * 0.5f;
            break;
            case WindowBarPosition::Right:
                x = _projection.GetResolution().x - _windowBarThiccness * 0.5f;*/
            default: //Top
                y = _projection.GetResolution().y - _windowBarThiccness * 0.5f;
            }
        }
    }

    return glm::vec3(x, y, _position.z + 0.1f);
}

void RetroFuturaGUI::WindowBar::windowShouldCloseCallback()
{
    _windowShouldClose = true;
}

RetroFuturaGUI::MaximizeState RetroFuturaGUI::WindowBar::GetMaximizeState()
{
    return _maximizeState;
}

void RetroFuturaGUI::WindowBar::ConnectMaximizeCallback(const std::function<void()> &callback)
{
    _maximizeCallback = callback;

    if(_maximize)
        _maximize->Connect_OnClick(_maximizeCallback, false);
}

void RetroFuturaGUI::WindowBar::SetElementTextColor(const glm::vec4 &color, const ColorState state, const ElementType elementType)
{
    switch(elementType)
    {
        case ElementType::CloseButton:
            if(_close)
                _close->SetTextColor(color, state);
        break;
        case ElementType::MaximizeButton:
            if(_maximize)
                _maximize->SetTextColor(color, state);
        break;
        case ElementType::MinimizeButton:
            if(_minimize)
                _minimize->SetTextColor(color, state);
        break;
        case ElementType::NoDockingDrag:
            //todo
        break;
        case ElementType::Title:
            if(_windowTitle)
                _windowTitle->SetColor(color);
        break;
        case ElementType::Icon:
            //todo
        break;
        default:
        break;
    }
}

void RetroFuturaGUI::WindowBar::SetElementBackgroundImageTextureID(const u32 textureID, const ElementType elementType)
{
    switch(elementType)
    {
        case ElementType::CloseButton:
            if(_close)
                _close->SetWindowBackgroundImageTextureID(textureID);
        break;
        case ElementType::MaximizeButton:
            if(_maximize)
                _maximize->SetWindowBackgroundImageTextureID(textureID);
        break;
        case ElementType::MinimizeButton:
            if(_minimize)
                _minimize->SetWindowBackgroundImageTextureID(textureID);
        break;
        case ElementType::NoDockingDrag:
            //todo
        break;
        case ElementType::Title:
            if(_background)
                _background->SetWindowBackgroundImageTextureID(textureID);
        break;
        case ElementType::Icon:
            //todo
        break;
        default:
        break;
    }
}

void RetroFuturaGUI::WindowBar::SetButtonCornerRadii(const glm::vec4 &radii, const ElementType buttonType)
{
    switch(buttonType)
    {
        case ElementType::CloseButton:
            if(_close)
                _close->SetCornerRadii(radii);
        break;
        case ElementType::MaximizeButton:
            if(_maximize)
                _maximize->SetCornerRadii(radii);
        break;
        case ElementType::MinimizeButton:
            if(_minimize)
                _minimize->SetCornerRadii(radii);
        break;
        default:
        break;
    }
}

void RetroFuturaGUI::WindowBar::SetWindowTitle(std::string_view title, std::string_view fontFamily)
{
    _title = title;

    if(!_windowTitle)
    {
        _windowTitle = std::make_unique<Text>(&_projection);

        if(_windowTitle)
        {
            glm::vec2 titleParentSize = calculateTitleParentSize();
            glm::vec3 titleAnchor = calculateElementPosition(ElementType::Title);

            _windowTitle->SetFontFamily(fontFamily, _windowBarThiccness * 0.5f, PlatformBridge::Fonts::Slant::Roman, PlatformBridge::Fonts::Weight::Normal);
            _windowTitle->SetParentSize(titleParentSize);
            _windowTitle->SetTextUTF8(_title);
            _windowTitle->SetColor(glm::vec4(1.0f));
            _windowTitle->SetSize(glm::vec3(_windowBarThiccness * 0.7f, _windowBarThiccness * 0.7f, 0.01f));
            _windowTitle->SetTextAlignment(TextAlignment::Left);
            _windowTitle->SetTextPadding(3.0f);
            _windowTitle->SetPosition(glm::vec3(titleAnchor.x + titleParentSize.x * 0.5f, titleAnchor.y, titleAnchor.z));
            _windowTitle->SetRotation(0.0f);
        }

        Resize(); //hotfix for dislocation when Text has been initialized with an empty string
        return;
    }

    _windowTitle->SetTextUTF8(_title);
    Resize(); //hotfix for dislocation when Text has been initialized with an empty string
}

glm::vec2 RetroFuturaGUI::WindowBar::calculateTitleParentSize()
{
    /* Text::alignPosition() treats its position as the center of a box this size, but
       calculateElementPosition(Title) hands back a left-edge anchor - so callers must
       shift the x by half of this before handing it to Text::SetPosition.*/
    return glm::vec2(_windowBarThiccness * 10.0f, _windowBarThiccness);
}

void RetroFuturaGUI::WindowBar::minimizeWindowCallback(GLFWwindow *window)
{
    glfwIconifyWindow(window);
}

bool RetroFuturaGUI::WindowBar::IsPointInside(const f32 pointX, const f32 pointY)
{
    f32 buttonSpan = 28.0f * 3.0f + 3.0f * 4.0f; //3 buttons + offsets

    switch(_windowBarPosition)
    {
        case WindowBarPosition::Top:
        case WindowBarPosition::Bottom:
        {
            return (pointX >= _position.x - _size.x * 0.5f &&
                    pointX <= (_position.x + _size.x * 0.5f) - buttonSpan &&
                    _projection.GetResolution().y - pointY >= _position.y - _size.y * 0.5f &&
                    _projection.GetResolution().y - pointY <= _position.y + _size.y * 0.5f);
        }
        /*case WindowBarPosition::Left:
        case WindowBarPosition::Right:
        {
            //todo
        }*/
    }

    return false;
}

void RetroFuturaGUI::WindowBar::EnableElement(const RetroFuturaGUI::WindowBar::ElementType elementType)
{
    *reinterpret_cast<u32*>(&_elements) |= static_cast<u32>(elementType);

    switch(elementType)
    {
        case ElementType::CloseButton:
        {
            if(_close)
                return;

            _close = std::make_unique<Button>("CloseButton", static_cast<RetroFuturaGUI::Projection*>(&_projection), this, RetroFuturaGUI::WidgetTypeID::WindowBar, _parentWindow);
            
            if(!_close)
                return;

            _close->SetPosition(calculateElementPosition(ElementType::CloseButton));
            _close->SetSize(glm::vec3(_windowBarThiccness - 2.0f, _windowBarThiccness - 2.0f, 0.01f));
            _close->SetRotation(0.0f);
            _close->SetTextAlignment(RetroFuturaGUI::TextAlignment::Center);
            _close->SetTextPadding(3.0f);
            _close->SetFontFamily("Noto Sans", _windowBarThiccness * 0.5f, PlatformBridge::Fonts::Slant::Roman, PlatformBridge::Fonts::Weight::Normal);
            _close->SetBorderWidth(2.0f);
            _close->SetText("X");
            _close->Connect_OnClick([this]() { windowShouldCloseCallback(); }, false);
        } return;
        case ElementType::MinimizeButton:
        {
            if(_minimize)
                return;

            _minimize = std::make_unique<Button>("MinimizeButton", static_cast<RetroFuturaGUI::Projection*>(&_projection), this, RetroFuturaGUI::WidgetTypeID::WindowBar, _parentWindow);

            if(!_minimize)
                return;

            _minimize->SetPosition(calculateElementPosition(ElementType::MinimizeButton));
            _minimize->SetSize(glm::vec3(_windowBarThiccness - 2.0f, _windowBarThiccness - 2.0f, 0.01f));
            _minimize->SetRotation(0.0f);
            _minimize->SetTextAlignment(RetroFuturaGUI::TextAlignment::Center);
            _minimize->SetTextPadding(3.0f);
            _minimize->SetFontFamily("Noto Sans", _windowBarThiccness * 0.5f, PlatformBridge::Fonts::Slant::Roman, PlatformBridge::Fonts::Weight::Normal);
            _minimize->SetBorderWidth(2.0f);
            _minimize->SetText("_");
            _minimize->Connect_OnClick([this]() { minimizeWindowCallback(_parentWindow); }, false);
        } return;
        case ElementType::MaximizeButton:
        {
            if(_maximize)
                return;

            _maximize = std::make_unique<Button>("MaximizeButton", static_cast<RetroFuturaGUI::Projection*>(&_projection), this, RetroFuturaGUI::WidgetTypeID::WindowBar, _parentWindow);
                
            if(!_maximize)
                return;

            _maximize->SetPosition(calculateElementPosition(ElementType::MaximizeButton));
            _maximize->SetSize(glm::vec3(_windowBarThiccness - 2.0f, _windowBarThiccness - 2.0f, 0.01f));
            _maximize->SetRotation(0.0f);
            _maximize->SetTextAlignment(RetroFuturaGUI::TextAlignment::Center);
            _maximize->SetTextPadding(3.0f);
            _maximize->SetFontFamily("Noto Sans", _windowBarThiccness * 0.5f, PlatformBridge::Fonts::Slant::Roman, PlatformBridge::Fonts::Weight::Normal);
            _maximize->SetBorderWidth(2.0f);
            _maximize->SetText("M");

            if(_maximizeCallback)
                _maximize->Connect_OnClick(_maximizeCallback, false);
        } return;
        case ElementType::Title:
        {
            if(_windowTitle)
                return;

            _windowTitle = std::make_unique<Text>(static_cast<RetroFuturaGUI::Projection*>(&_projection));
                
            if(!_windowTitle)
                return;

            /* Center alignment measures off _textSpan, not _parentSize, so the anchor from
             calculateElementPosition can be used directly here (unlike the Left-aligned
             case in SetWindowTitle(), which needs the anchor shifted to a box center)*/
            _windowTitle->SetPosition(calculateElementPosition(ElementType::Title));
            _windowTitle->SetSize(glm::vec3(_windowBarThiccness - 2.0f, _windowBarThiccness - 2.0f, 0.01f));
            _windowTitle->SetParentSize(calculateTitleParentSize());
            _windowTitle->SetRotation(0.0f);
            _windowTitle->SetTextAlignment(RetroFuturaGUI::TextAlignment::Center);
            _windowTitle->SetTextPadding(3.0f);
        } return;
        case ElementType::Background:
        {
            if(_background)
                return;

            _background = std::make_unique<Rectangle>(&_projection);

            if(!_background)
                return;
    
            _background->SetRectangleMode(RectangleMode::Plane);
            _background->SetPosition(_position);
            _background->SetSize(_size);
            _background->SetRotation(0.0f);
        } return;
        default:
        return;
    }
}

void RetroFuturaGUI::WindowBar::GiveMaximizeCallback(const std::function<void ()>& callback)
{
    _maximizeCallback = callback;
}


void RetroFuturaGUI::WindowBar::SetButtonBackgroundFillType(const ElementType elementType, const FillType fillType)
{
    switch(elementType)
    {
        case ElementType::CloseButton:
            if(_close)
                _close->SetBackgroundFillType(fillType);
        return;
        case ElementType::MinimizeButton:
            if(_minimize)
                _minimize->SetBackgroundFillType(fillType);
        return;
        case ElementType::MaximizeButton:
            if(_maximize)
                _maximize->SetBackgroundFillType(fillType);
        return;
        default:
        return;
    }
}

void RetroFuturaGUI::WindowBar::SetButtonBorderFillType(const ElementType elementType, const FillType fillType)
{
    switch(elementType)
    {
        case ElementType::CloseButton:
            if(_close)
                _close->SetBorderFillType(fillType);
        return;
        case ElementType::MinimizeButton:
            if(_minimize)
                _minimize->SetBorderFillType(fillType);
        return;
        case ElementType::MaximizeButton:
            if(_maximize)
                _maximize->SetBorderFillType(fillType);
        return;
        default:
        return;
    }
}

void RetroFuturaGUI::WindowBar::SetButtonBorderWidth(const ElementType elementType, const f32 width)
{
    switch(elementType)
    {
        case ElementType::CloseButton:
            if(_close)
                _close->SetBorderWidth(width);
        return;
        case ElementType::MinimizeButton:
            if(_minimize)
                _minimize->SetBorderWidth(width);
        return;
        case ElementType::MaximizeButton:
            if(_maximize)
                _maximize->SetBorderWidth(width);
        return;
        default:
        return;
    }
}

void RetroFuturaGUI::WindowBar::SetButtonCornerRadii(const ElementType elementType, const glm::vec4& radii)
{
    switch(elementType)
    {
        case ElementType::CloseButton:
            if(_close)
                _close->SetCornerRadii(radii);
        return;
        case ElementType::MinimizeButton:
            if(_minimize)
                _minimize->SetCornerRadii(radii);
        return;
        case ElementType::MaximizeButton:
            if(_maximize)
                _maximize->SetCornerRadii(radii);
        return;
        default:
        return;
    }
}

void RetroFuturaGUI::WindowBar::SetWindowTitleFontFamily(const std::string& family, const f32 fontSize, const PlatformBridge::Fonts::Slant fontSlant, const PlatformBridge::Fonts::Weight fontWeight)
{
    if(_windowTitle)
        _windowTitle->SetFontFamily(family, fontSize, fontSlant, fontWeight);
}

void RetroFuturaGUI::WindowBar::SetButtonBackgroundColors(const ElementType elementType, std::span<glm::vec4> colors, const ColorState colorState)
{
    switch(elementType)
    {
        case ElementType::CloseButton:
            if(_close)
                _close->SetBackgroundColors(colors, colorState);
        return;
        case ElementType::MinimizeButton:
            if(_minimize)
                _minimize->SetBackgroundColors(colors, colorState);
        return;
        case ElementType::MaximizeButton:
            if(_maximize)
                _maximize->SetBackgroundColors(colors, colorState);
        return;
        default:
        return;
    }
}

void RetroFuturaGUI::WindowBar::SetButtonBorderColors(const ElementType elementType, std::span<glm::vec4> colors, const ColorState colorState)
{
    switch(elementType)
    {
        case ElementType::CloseButton:
            if(_close)
                _close->SetBorderColors(colors, colorState);
        return;
        case ElementType::MinimizeButton:
            if(_minimize)
                _minimize->SetBorderColors(colors, colorState);
        return;
        case ElementType::MaximizeButton:
            if(_maximize)
                _maximize->SetBorderColors(colors, colorState);
        return;
        default:
        return;
    }
}

void RetroFuturaGUI::WindowBar::SetBackgroundColors(std::span<glm::vec4> colors)
{
    _backgroundColors = std::vector<glm::vec4>(colors.begin(), colors.end());

    if(_background)
        _background->SetColors(std::span<glm::vec4>(_backgroundColors.data(), _backgroundColors.size()));
}