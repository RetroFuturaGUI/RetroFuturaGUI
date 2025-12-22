#pragma once
#include "IncludeHelper.hpp"
#include "Button.hpp"
#include "PlatformBridge.hpp"

namespace RetroFuturaGUI
{
    enum WindowBarPosition : u32
    {
        Top,
        Left,
        Bottom,
        Right
    };

    enum WindowBorderDecoration : u32
    {
        Border,
        Line
    };

    enum class MaximizeState : u32
    {
        MAXIMIZE,
        RESTORE
    };

    class WindowBar : public IWidget
    {
    public:
        enum class ElementType : u32
        {
            CloseButton,
            MaximizeButton,
            MinimizeButton,
            NoDockingDrag,
            Title,
            Icon
        };

        WindowBar(const IdentityParams& identity, GeometryParams2D& geometry, const glm::vec4& color, const WindowBarPosition wbPosition = WindowBarPosition::Top);
        void Draw();
        bool WindowShouldClose();
        void Resize();
        bool IsPointInside(const f32 pointX, const f32 pointY);
        MaximizeState GetMaximizeState();
        void ConnectMaximizeCallback(const std::function<void()>& callback);
        void SetElementBackgroundColor(const glm::vec4& color, const ColorSetState state, const ElementType elementType);
        void SetElementTextColor(const glm::vec4& color, const ColorSetState state, const ElementType elementType);
        void SetElementBackgroundImageTextureID(const u32 textureID, const ElementType elementType);
        void SetButtonCornerRadii(const glm::vec4& radii, const ElementType buttonType);


    private:
        // Identity
        std::string _name;

        // Geometry
        Projection& _projection;
        WindowBarPosition _windowBarPosition { WindowBarPosition::Top };
        float _windowBarThiccness { 30.0f };

        // Elements
        std::unique_ptr<Rectangle> _background;
        std::unique_ptr<Text> _windowTitle;
        std::unique_ptr<LineBorder> _border;
        std::unique_ptr<Button> 
            _minimize,
            _maximize,
            _close;
        
        //logic
        bool _windowShouldClose { false };
        MaximizeState _maximizeState { MaximizeState::RESTORE };

        glm::vec2 calculateWindowBarPosition(const glm::vec2& position);
        glm::vec2 calculateWindowBarSize(const glm::vec2& size);
        glm::vec2 calculateElementPosition(const glm::vec2& position, const ElementType elementType);
        void windowShouldCloseCallback();
        void minimizeWindowCallback(GLFWwindow* window);
    };
}