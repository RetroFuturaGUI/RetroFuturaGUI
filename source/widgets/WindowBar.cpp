#include "WindowBar.hpp"
#include "Window.hpp"

RetroFuturaGUI::WindowBar::WindowBar(const IdentityParams &identity, GeometryParams2D &geometry, const glm::vec4 &color, const WindowBarPosition wbPosition)
    : IWidget(identity, geometry), _projection(const_cast<Projection&>(geometry._Projection)), _windowBarPosition(wbPosition)
{   
    _position = calculateWindowBarPosition(geometry._Position);
    _size = calculateWindowBarSize(geometry._Size);

    GeometryParams2D geometryAdv = 
    {
        geometry._Projection,
        _position,
        _size,
        0.0f
    };
    _background = std::make_unique<Rectangle>(geometryAdv, color);

    GeometryParams2D geometryButton{
        geometry._Projection,
        calculateElementPosition(geometry._Position, ElementType::CloseButton),
        glm::vec2(28.0f),
        0.0f
    };

    IdentityParams identityCloseButton
    {
        "testWindowBarClosw",
        this,
        WidgetTypeID::WindowBar
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

    BorderParams borderParams =
    {
        glm::vec4(0.0f),
        2.0f
    };

    _close = std::make_unique<Button>(identityCloseButton, geometryButton, textParams, borderParams);
    _close->SetBackgroundColor(glm::vec4(1.0f, 0.4f, 0.4f, 1.0f), ColorSetState::Enabled);
    _close->Connect_OnClick([this]() { windowShouldCloseCallback(); }, false);

    IdentityParams identityMaximize
    {
        "testMaximize",
        this,
        WidgetTypeID::WindowBar
    };

    GeometryParams2D geometryButtonMaximize
    {
        geometry._Projection,
        calculateElementPosition(geometry._Position, ElementType::MaximizeButton),
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

    _maximize = std::make_unique<Button>(identityMaximize, geometryButtonMaximize, textParamsMaximize, borderParams);

    IdentityParams identityMinimize
    {
        "testMinimize",
        this,
        WidgetTypeID::WindowBar
    };

    GeometryParams2D geometryButtonMinimize
    {
        geometry._Projection,
        calculateElementPosition(geometry._Position, ElementType::MinimizeButton),
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

    _minimize = std::make_unique<Button>(identityMinimize, geometryButtonMinimize, textParamsMinimize, borderParams);

    TextParams textParamsTitel = 
    {
        "RetroFuturaGUI Test",
        font,
        glm::vec4(1.0f),
        glm::vec2(25.0f),
        TextAlignment::LEFT,
        3.0f
    };

    GeometryParams2D geometryButtonTitle
    {
        geometry._Projection,
        calculateElementPosition(geometry._Position, ElementType::Titel),
        glm::vec2(28.0f),
        0.0f
    };

    _windowTitle = std::make_unique<Text>(geometryButtonTitle, textParamsTitel);
}

void RetroFuturaGUI::WindowBar::Draw()
{
    _background->Draw();
    _close->Draw();
    _maximize->Draw();
    _minimize->Draw();
    _windowTitle->Draw();
}

bool RetroFuturaGUI::WindowBar::WindowShouldClose()
{
    return _windowShouldClose;
}

void RetroFuturaGUI::WindowBar::Resize()
{
    _size = calculateWindowBarSize(_size);
    _position = calculateWindowBarPosition(_position);
    _background->Resize(_size);
    _background->Move(_position);
    _close->SetPosition(calculateElementPosition(_position, ElementType::CloseButton));
    _maximize->SetPosition(calculateElementPosition(_position, ElementType::MaximizeButton));
    _minimize->SetPosition(calculateElementPosition(_position, ElementType::MinimizeButton));
    glm::vec2 titlePosition = calculateElementPosition(_position, ElementType::Titel);
    _windowTitle->SetPosition(titlePosition);
}

glm::vec2 RetroFuturaGUI::WindowBar::calculateWindowBarPosition(const glm::vec2 &position)
{
    f32 x, y;

    if(_windowBarPosition == WindowBarPosition::Top || _windowBarPosition == WindowBarPosition::Bottom)
        x = _projection.GetResolution().x * 0.5f;
    else
        y = _projection.GetResolution().y * 0.5f;

    switch(_windowBarPosition)
    {
    case WindowBarPosition::Bottom:
        y = _windowBarThiccness * 0.5f;
    break;
    case WindowBarPosition::Left:
        x = _windowBarThiccness * 0.5f;
    break;
    case WindowBarPosition::Right:
        x = _projection.GetResolution().x - _windowBarThiccness * 0.5f;
    default: //Top
        y = _projection.GetResolution().y - _windowBarThiccness * 0.5f;
    }

    return glm::vec2(x, y);
}

glm::vec2 RetroFuturaGUI::WindowBar::calculateWindowBarSize(const glm::vec2 &size)
{
    return glm::vec2(_windowBarPosition == WindowBarPosition::Top || _windowBarPosition == WindowBarPosition::Bottom ? _projection.GetResolution().x : _windowBarThiccness,
                     _windowBarPosition == WindowBarPosition::Left || _windowBarPosition == WindowBarPosition::Right ? _projection.GetResolution().y : _windowBarThiccness);
}

glm::vec2 RetroFuturaGUI::WindowBar::calculateElementPosition(const glm::vec2 &position, const ElementType elementType)
{
    f32 offset = 3.0f, x, y;

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
            case WindowBarPosition::Left:
                x = _windowBarThiccness * 0.5f;
            break;
            case WindowBarPosition::Right:
                x = _projection.GetResolution().x - _windowBarThiccness * 0.5f;
            default: //Top
                y = _projection.GetResolution().y - _windowBarThiccness * 0.5f;
            }
        break;
        case ElementType::Titel:
            if(_windowBarPosition == WindowBarPosition::Top || _windowBarPosition == WindowBarPosition::Bottom)
                x =  _windowBarThiccness * 1.5f + offset;
            else
                y =  _windowBarThiccness * 1.5f + offset;

            switch(_windowBarPosition)
            {
            case WindowBarPosition::Bottom:
                y = _windowBarThiccness * 0.5f;
            break;
            case WindowBarPosition::Left:
                x = _windowBarThiccness * 0.5f;
            break;
            case WindowBarPosition::Right:
                x = _projection.GetResolution().x - _windowBarThiccness * 0.5f;
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
            case WindowBarPosition::Left:
                x = _windowBarThiccness * 0.5f;
            break;
            case WindowBarPosition::Right:
                x = _projection.GetResolution().x - _windowBarThiccness * 0.5f;
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
        case WindowBarPosition::Left:
        case WindowBarPosition::Right:
        {
            //todo
        }
    }
}
