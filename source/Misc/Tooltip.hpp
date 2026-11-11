#pragma once
#include "Background.hpp"
#include "Border.hpp"
#include "WidgetText.hpp"
#include "Projection.hpp"
#include <chrono>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>

namespace RetroFuturaGUI
{
    class IWidget;

    /// @brief Text in a box that shows above the mouse cursor once it has rested on a widget for a while
    class Tooltip final
    {
    public:
        /// @brief Constructs a Tooltip for the given widget
        Tooltip(Projection* projection, IWidget* widget);
        Tooltip() = delete;
        Tooltip(const Tooltip&) = delete;
        Tooltip(Tooltip&&) = delete;
        ~Tooltip();
        auto operator =(const Tooltip&) = delete;
        auto operator =(Tooltip&&) = delete;

        /// @brief Tracks the mouse on the widget. Once it has rested there for the show delay, this tooltip is drawn at the end of the frame
        void Update();

        /// @brief Draws the tooltip shown this frame in the given window over everything drawn so far. Window::Draw calls it last
        static void DrawShown(GLFWwindow* window);

        /// @brief Sets the font of every tooltip, those created before this call included. A tooltip given its own font with SetFontFamily keeps it
        static void SetDefaultFontFamily(std::string_view fontFamily, const f32 fontSize, const FontSizeUnit sizeUnit, const PlatformBridge::Fonts::Slant slant, const PlatformBridge::Fonts::Weight fontWeight);

        void SetFontFamily(std::string_view fontFamily, const f32 fontSize, const FontSizeUnit sizeUnit, const PlatformBridge::Fonts::Slant slant, const PlatformBridge::Fonts::Weight fontWeight);

        /// @brief Returns the widget's text: its string, font, colors and effects
        WidgetText& GetText();
        const WidgetText& GetText() const;

        /// @brief Returns the tooltip's background
        Background& GetBackground();
        
        /// @brief Returns the tooltip's background
        const Background& GetBackground() const;

        /// @brief Returns the tooltip's border. The box grows with its width, since the border draws inside the box
        Border& GetBorder();
        
        /// @brief Returns the tooltip's border
        const Border& GetBorder() const;

    private:
        void draw();
        void setSize();
        void setPosition();
        void interact();

        /// @brief Switches to the default font if it changed since this tooltip last took it, unless the tooltip has its own font.
        void applyDefaultFont();

        /// @brief Has the box and the text follow the widget's transitions.
        void useWidgetTransitions();

        /// @brief Copies the host window's projection, so the tooltip draws flat in whichever window it opened in
        void followHostWindow();

        Projection _hostProjection; //its host window's, copied when it draws. Declared before what draws through it
        GLFWwindow* _hostWindow { nullptr }; //where the cursor is over the widget: the widget's window, or an EnvironmentView's seeing it
        IWidget* _widget { nullptr };
        Background _background { &_hostProjection };
        Border _border { &_hostProjection, { ._OnWidthChange = [this]() { setSize(); }, ._OnCornerRadiiChange = {} } };
        WidgetText _text { &_hostProjection, [this](const WidgetTextChange) { setSize(); } };
        bool
            _isHovering { false },
            _isShown { false },
            _isDismissed { false },
            _hasOwnFont { false };
        std::chrono::steady_clock::time_point _hoverStart {};
        glm::vec2 _pointerPosition { 0.0f }; //in the host window
        f32 _depth { 0.0f }; //the widget's, or the window's front when it is seen through a view
        TransitionValue<f32> _opacity {};
        u32 _appliedDefaultFontVersion { 0 };

        //Per window, since every window interacts before any draws: drawn and cleared by DrawShown at the end of its frame
        inline static std::unordered_map<GLFWwindow*, Tooltip*> _shownTooltips {};
        inline static std::string _defaultFontFamily {};
        inline static f32 _defaultFontSize { FontManager::ToPixels(16.0f, FontSizeUnit::Points) }; //in pixels, as every stored font size
        inline static PlatformBridge::Fonts::Slant _defaultFontSlant { PlatformBridge::Fonts::Slant::Roman };
        inline static PlatformBridge::Fonts::Weight _defaultFontWeight { PlatformBridge::Fonts::Weight::Normal };
        inline static u32 _defaultFontVersion { 1 }; //above a new tooltip's 0, so it takes a font even when no default was set

        static constexpr const f32
            _zStep { 0.01f },
            _cursorDistance { 4.0f },
            _defaultBorderWidth { 1.0f },
            _defaultTextPadding { 4.0f };
        static constexpr const std::chrono::seconds _showDelay { 1 };
    };
}