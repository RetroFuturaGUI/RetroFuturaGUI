#pragma once
#include "IBackground.hpp"
#include "IBorder.hpp"
#include "ITextProperties.hpp"
#include "Projection.hpp"
#include <chrono>
#include <string>
#include <string_view>

namespace RetroFuturaGUI
{
    class IWidget;

    /// @brief Text in a box that shows above the mouse cursor once it has rested on a widget for a while. The widget owns it
    ///        (IWidget::SetTooltipText), the widget's container updates it every frame, and Window::Draw draws the one
    ///        that is shown at the end of the frame, over every scene.
    class Tooltip final : public IBackground, public IBorder, public ITextProperties
    {
    public:
        /// @brief Constructs a Tooltip for the given widget.
        Tooltip(Projection* projection, IWidget* widget);
        Tooltip() = delete;
        Tooltip(const Tooltip&) = delete;
        Tooltip(Tooltip&&) = delete;
        ~Tooltip();
        auto operator =(const Tooltip&) = delete;
        auto operator =(Tooltip&&) = delete;

        /// @brief Tracks the mouse on the widget. Once it has rested there for the show delay, this tooltip is drawn at the end of the frame.
        void Update();

        /// @brief Draws the tooltip shown this frame over everything drawn so far. Window::Draw calls it last.
        static void DrawShown();

        /// @brief Sets the font of every tooltip, those created before this call included. A tooltip given its own font with SetFontFamily keeps it.
        static void SetDefaultFontFamily(std::string_view fontFamily, const f32 fontSize, const PlatformBridge::Fonts::Slant slant, const PlatformBridge::Fonts::Weight fontWeight);

        void SetBackgroundColors(std::span<glm::vec4> colors);
        void SetBorderColors(std::span<glm::vec4> colors);
        void SetCornerRadii(const glm::vec4& radii);
        void SetText(std::string_view text, bool emitSignal) override;
        void SetFontFamily(std::string_view fontFamily, const f32 fontSize, const PlatformBridge::Fonts::Slant slant, const PlatformBridge::Fonts::Weight fontWeight) override;
        void SetTextPadding(const f32 padding) override;

        /// @brief Sets the border width. The box grows with it, since the border draws inside the box.
        void SetBorderWidth(const f32 borderWidth);

    private:
        friend class Window; //sets its own waiting tooltip aside while it draws a window docked into one of its tabs

        /// @brief Sets the tooltip waiting for the end of the frame, so a window drawn in the middle of another one's frame draws only its own.
        static void setShownAside();

        /// @brief Makes the tooltip set aside the waiting one again.
        static void restoreShownFromAside();

        void draw();
        void setSize();
        void setPosition();
        void interact();

        /// @brief Switches to the default font if it changed since this tooltip last took it, unless the tooltip has its own font.
        void applyDefaultFont();

        Projection* _projection { nullptr };
        IWidget* _widget { nullptr };
        std::vector<glm::vec4>
            _backgroundColors {},
            _borderColors {};
        bool
            _isHovering { false },
            _isShown { false },
            _isDismissed { false },
            _hasOwnFont { false };
        std::chrono::steady_clock::time_point _hoverStart {};
        glm::vec2 _pointerPosition { 0.0f };
        u32 _appliedDefaultFontVersion { 0 };

        inline static Tooltip* _shownTooltip { nullptr }; //drawn and cleared by DrawShown at the end of the frame
        inline static Tooltip* _asideTooltip { nullptr }; //a window's waiting tooltip, while a window docked into its tab draws
        inline static std::string _defaultFontFamily {};
        inline static f32 _defaultFontSize { 16.0f };
        inline static PlatformBridge::Fonts::Slant _defaultFontSlant { PlatformBridge::Fonts::Slant::Roman };
        inline static PlatformBridge::Fonts::Weight _defaultFontWeight { PlatformBridge::Fonts::Weight::Normal };
        inline static u32 _defaultFontVersion { 0 };

        static constexpr const f32
            _zStep { 0.01f },
            _cursorDistance { 4.0f },
            _defaultBorderWidth { 1.0f },
            _defaultTextPadding { 4.0f };
        static constexpr const std::chrono::seconds _showDelay { 1 };
    };
}
