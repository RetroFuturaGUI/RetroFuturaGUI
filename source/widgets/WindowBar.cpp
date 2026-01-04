#include "WindowBar.hpp"
#include "Window.hpp"

RetroFuturaGUI::WindowBar::WindowBar(const IdentityParams &identity, GeometryParams2D &geometry, const glm::vec4& color, const WindowBarPosition wbPosition)
    : IWidget(identity, geometry), _projection(const_cast<Projection&>(geometry._Projection)), _windowBarPosition(wbPosition)
{   
    _position = calculateWindowBarPosition();
    _size = calculateWindowBarSize();

    GeometryParams2D geometryAdv = 
    {
        geometry._Projection,
        _position,
        _size,
        0.0f
    };

    _backgroundColors.resize(1, color);
    _background = std::make_unique<Rectangle>(geometryAdv, _backgroundColors, RectangleMode::PLANE);

    GeometryParams2D geometryButton{
        geometry._Projection,
        calculateElementPosition(ElementType::CloseButton),
        glm::vec2(28.0f),
        0.0f
    };

    IdentityParams identityCloseButton
    {
        "testWindowBarClose",
        this,
        WidgetTypeID::WindowBar,
        _parentWindow
    };

    std::string font = PlatformBridge::Fonts::GetFontsInformation().front().second;
    TextParams textParams = 
    {
        "X",
        font,
        glm::vec4(1.0f),
        glm::vec2(25.0f),
        TextAlignment::CENTER,
        3.0f
    };

    _close = std::make_unique<Button>(identityCloseButton, geometryButton, textParams, 2.0f);
    
    if(_close)
    {
        _close->SetBackgroundColor(glm::vec4(1.0f, 0.4f, 0.4f, 1.0f), ColorState::Enabled);
        _close->Connect_OnClick([this]() { windowShouldCloseCallback(); }, false);
    }

    IdentityParams identityMaximize
    {
        "testMaximize",
        this,
        WidgetTypeID::WindowBar,
        _parentWindow
    };

    GeometryParams2D geometryButtonMaximize
    {
        geometry._Projection,
        calculateElementPosition(ElementType::MaximizeButton),
        glm::vec2(28.0f),
        0.0f
    };

    TextParams textParamsMaximize = 
    {
        "M",
        font,
        glm::vec4(1.0f),
        glm::vec2(25.0f),
        TextAlignment::CENTER,
        3.0f
    };

    _maximize = std::make_unique<Button>(identityMaximize, geometryButtonMaximize, textParamsMaximize, 0.2f);

    IdentityParams identityMinimize
    {
        "testMinimize",
        this,
        WidgetTypeID::WindowBar,
        _parentWindow
    };

    GeometryParams2D geometryButtonMinimize
    {
        geometry._Projection,
        calculateElementPosition(ElementType::MinimizeButton),
        glm::vec2(28.0f),
        0.0f
    };

    TextParams textParamsMinimize = 
    {
        "_",
        font,
        glm::vec4(1.0f),
        glm::vec2(25.0f),
        TextAlignment::CENTER,
        3.0f
    };

    _minimize = std::make_unique<Button>(identityMinimize, geometryButtonMinimize, textParamsMinimize, 0.2f);
   
    if(_minimize)
        _minimize->Connect_OnClick([this]() { minimizeWindowCallback(_parentWindow); }, false);

    TextParams textParamsTitle = 
    {
        _title,
        font,
        glm::vec4(1.0f),
        glm::vec2(25.0f),
        TextAlignment::LEFT,
        3.0f
    };

    GeometryParams2D geometryButtonTitle
    {
        geometry._Projection,
        calculateElementPosition(ElementType::Title),
        glm::vec2(28.0f),
        0.0f
    };

    _windowTitle = std::make_unique<Text>(geometryButtonTitle, textParamsTitle);
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
        glm::vec2 titlePosition = calculateElementPosition(ElementType::Title);
        _windowTitle->SetPosition(titlePosition);
    }
}

glm::vec2 RetroFuturaGUI::WindowBar::calculateWindowBarPosition()
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

    return glm::vec2(x, y);
}

glm::vec2 RetroFuturaGUI::WindowBar::calculateWindowBarSize()
{
    return glm::vec2(_windowBarPosition == WindowBarPosition::Top || _windowBarPosition == WindowBarPosition::Bottom ? _projection.GetResolution().x : _windowBarThiccness/*,
                     _windowBarPosition == WindowBarPosition::Left || _windowBarPosition == WindowBarPosition::Right ? _projection.GetResolution().y : _windowBarThiccness*/, _windowBarThiccness);
}

glm::vec2 RetroFuturaGUI::WindowBar::calculateElementPosition(const ElementType elementType)
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

    return glm::vec2(x, y);
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
    if(_maximize)
        _maximize->Connect_OnClick(callback, false);
}

void RetroFuturaGUI::WindowBar::SetElementBackgroundColor(const glm::vec4& color, const ColorState state, const ElementType elementType)
{
    switch(elementType)
    {
        case ElementType::CloseButton:
            if(_close)
                _close->SetBackgroundColor(color, state);
        break;
        case ElementType::MaximizeButton:
            if(_maximize)
                _maximize->SetBackgroundColor(color, state);
        break;
        case ElementType::MinimizeButton:
            if(_minimize)
                _minimize->SetBackgroundColor(color, state);
        break;
        case ElementType::NoDockingDrag:
            //todo
        break;
        case ElementType::Title:
            if(!_background)
                break;

            _backgroundColors.clear();
            _backgroundColors.resize(1, color);
            _background->SetColor(_backgroundColors);
        break;
        case ElementType::Icon:
            //todo
        break;
    }
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
    }
}

void RetroFuturaGUI::WindowBar::SetWindowTitle(std::string_view title)
{
    if(!_windowTitle)
        return;

    _title = title;
    _windowTitle->SetText(_title);
    Resize(); //hotfix for dislocation when Text has been initialized with an empty string
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