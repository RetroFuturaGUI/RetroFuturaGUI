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

        /// @brief Constructs a WindowBar under the given parent widget/window.
        WindowBar(const std::string& name, Projection* projection, IWidget* parentWidget, const WidgetTypeID parentWidgetTypeID, GLFWwindow* parentWindow);
        WindowBar(const WindowBar&) = delete;
        WindowBar(WindowBar&&) = delete;
        ~WindowBar() = default;
        auto operator =(const WindowBar&) = delete;
        auto operator =(WindowBar&&) = delete;

        /// @brief Draws the window bar and its enabled elements (title, icon, buttons, background).
        void Draw();

        /// @brief Returns whether the close button was clicked.
        bool WindowShouldClose();

        /// @brief Recomputes the size and position of the window bar and its elements to match the parent window.
        void Resize();

        /// @brief Returns whether the given point lies within the window bar's bounds.
        bool IsPointInside(const f32 pointX, const f32 pointY);

        /// @brief Returns the current maximize/restore state.
        MaximizeState GetMaximizeState();

        /// @brief Connects a callback to be invoked when the maximize button is clicked.
        void ConnectMaximizeCallback(const std::function<void()>& callback);

        /// @brief Sets the text color of the given element, for the given color state.
        void SetElementTextColor(const glm::vec4& color, const ColorState state, const ElementType elementType);

        /// @brief Sets the background image texture ID of the given element.
        void SetElementBackgroundImageTextureID(const u32 textureID, const ElementType elementType);

        /// @brief Sets the corner rounding radii of the given button element.
        void SetButtonCornerRadii(const glm::vec4& radii, const ElementType buttonType);

        /// @brief Sets the window bar's title text and font family.
        void SetWindowTitle(std::string_view title, std::string_view fontFamily);

        /// @brief Enables the given element(s), ORing them into the currently enabled set.
        void EnableElement(const ElementType elementType);
        void SetEnabled(const bool enable) = delete;

        /// @brief Sets the background fill type of the given button element.
        void SetButtonBackgroundFillType(const ElementType elementType, const FillType fillType);

        /// @brief Sets the border fill type of the given button element.
        void SetButtonBorderFillType(const ElementType elementType, const FillType fillType);

        /// @brief Sets the border width of the given button element.
        void SetButtonBorderWidth(const ElementType elementType, const f32 width);

        /// @brief Sets the corner rounding radii of the given button element.
        void SetButtonCornerRadii(const ElementType elementType, const glm::vec4& radii);

        /// @brief Sets the font family, size and style used to render the window title.
        void SetWindowTitleFontFamily(const std::string& family, const f32 fontSize, const PlatformBridge::Fonts::Slant fontSlant, const PlatformBridge::Fonts::Weight fontWeight);

        /// @brief Sets the background colors of the given button element, for the given color state.
        void SetButtonBackgroundColors(const ElementType elementType, std::span<glm::vec4> colors, const ColorState colorState);

        /// @brief Sets the border colors of the given button element, for the given color state.
        void SetButtonBorderColors(const ElementType elementType, std::span<glm::vec4> colors, const ColorState colorState);

        /// @brief Sets the callback to be invoked when the maximize button is clicked, replacing any previous callback.
        void GiveMaximizeCallback(const std::function<void ()>& callback);

        /// @brief Sets the window bar's background color(s).
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
        glm::vec2 calculateTitleParentSize();
        void windowShouldCloseCallback();
        void minimizeWindowCallback(GLFWwindow* window);
    };
}