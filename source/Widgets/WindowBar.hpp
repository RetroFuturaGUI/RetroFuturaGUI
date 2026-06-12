#pragma once
#include "IncludeHelper.hpp"
#include "Button.hpp"
#include "PlatformBridge.hpp"
#include <string_view>

namespace RetroFuturaGUI
{
    enum WindowBarPosition : u32
    {
        Top,
        //Left,
        Bottom
        //Right
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

    //A Window Bar with window sizing and termination options
    class WindowBar : public IWidget
    {
    public:
        enum class ElementType : u32
        {
            None = 0,
            CloseButton = 1,
            MaximizeButton = 1 << 1,
            MinimizeButton = 1 << 2,
            NoDockingDrag = 1 << 3,
            Title = 1 << 4,
            Icon = 1 << 5,
            Background = 1 << 6
        };

        WindowBar(const std::string& name, Projection* projection, IWidget* parentWidget, const WidgetTypeID parentWidgetTypeID, GLFWwindow* parentWindow);
        WindowBar(const WindowBar&) = delete;
        WindowBar(WindowBar&&) = delete;
        ~WindowBar() = default;
        auto operator =(const WindowBar&) = delete;
        auto operator =(WindowBar&&) = delete;
        void Draw();
        bool WindowShouldClose();
        void Resize();
        bool IsPointInside(const f32 pointX, const f32 pointY);
        MaximizeState GetMaximizeState();
        void ConnectMaximizeCallback(const std::function<void()>& callback);
        void SetElementTextColor(const glm::vec4& color, const ColorState state, const ElementType elementType);
        void SetElementBackgroundImageTextureID(const u32 textureID, const ElementType elementType);
        void SetButtonCornerRadii(const glm::vec4& radii, const ElementType buttonType);
        void SetWindowTitle(std::string_view title, std::string_view fontFamily);
        void EnableElement(const ElementType elementType);
        void SetEnabled(const bool enable) = delete;
        void SetButtonBackgroundFillType(const ElementType elementType, const FillType fillType);
        void SetButtonBorderFillType(const ElementType elementType, const FillType fillType);
        void SetButtonBorderWidth(const ElementType elementType, const f32 width);
        void SetButtonCornerRadii(const ElementType elementType, const glm::vec4& radii);
        void SetWindowTitleFontFamily(const std::string& family, const f32 fontSize, const PlatformBridge::Fonts::Slant fontSlant, const PlatformBridge::Fonts::Weight fontWeight);
        void SetButtonBackgroundColors(const ElementType elementType, std::span<glm::vec4> colors, const ColorState colorState);
        void SetButtonBorderColors(const ElementType elementType, std::span<glm::vec4> colors, const ColorState colorState);
        void GiveMaximizeCallback(const std::function<void ()>& callback);
        void SetBackgroundColors(std::span<glm::vec4> colors);

    private:
        // Identity
        std::string _name;
        std::string _title;

        // Geometry
        Projection& _projection;
        WindowBarPosition _windowBarPosition { WindowBarPosition::Top };
        float _windowBarThiccness { 30.0f };

        // Elements
        std::unique_ptr<Rectangle> _background;
        std::unique_ptr<Text> _windowTitle;
        //std::string _fontFamily;
        std::unique_ptr<Button> 
            _minimize,
            _maximize,
            _close;
        std::vector<glm::vec4> _backgroundColors { glm::vec4(1.0f) };
        
        //logic
        bool _windowShouldClose { false };
        MaximizeState _maximizeState { MaximizeState::RESTORE };
        ElementType _elements { ElementType::None };
        std::function<void ()> _maximizeCallback;
        
        glm::vec3 calculateWindowBarPosition();
        glm::vec3 calculateWindowBarSize();
        glm::vec3 calculateElementPosition(const ElementType elementType);
        void windowShouldCloseCallback();
        void minimizeWindowCallback(GLFWwindow* window);
    };
}