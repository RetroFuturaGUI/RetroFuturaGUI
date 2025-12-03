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

    class WindowBar : public IWidget
    {
    private:
        enum class ElementType : u32
        {
            CloseButton,
            MaximizeButton,
            MinimizeButton,
            NoDockingDrag,
            Titel,
            Icon
        };

    public:
        WindowBar(const IdentityParams& identity, GeometryParams2D& geometry, const glm::vec4& color, const WindowBarPosition wbPosition = WindowBarPosition::Top);
        void Draw();
        bool WindowShouldClose();

    private:
        glm::vec2 calculateWindowBarPosition(const glm::vec2& position);
        glm::vec2 calculateWindowBarSize(const glm::vec2& size);
        glm::vec2 calculateElementPosition(const glm::vec2& position, const ElementType elementType);

        // Identity
        std::string _name;
        // Geometry
        Projection& _projection;
        WindowBarPosition _windowBarPosition = WindowBarPosition::Top;
        float _windowBarthiccness = 30.0f;
        // Elements
        std::unique_ptr<Rectangle> _background;
        std::unique_ptr<Text> _windowTitle;
        std::unique_ptr<LineBorder> _border;
        std::unique_ptr<Button> _minimize;
        std::unique_ptr<Button> _maximize;
        std::unique_ptr<Button> _close;
        


        //logic
        bool _windowShouldClose = false;

    };
}