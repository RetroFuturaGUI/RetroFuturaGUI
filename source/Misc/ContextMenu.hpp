#pragma once
#include "ExtendedComboBox.hpp"
#include "IBackground.hpp"
#include "IBorder.hpp"
#include "ITextProperties.hpp"
#include "Projection.hpp"
#include "SvgImage.hpp"
#include <memory>
#include <string>
#include <string_view>

namespace RetroFuturaGUI
{
    class IWidget;

    /// @brief Nested dropdown items shown near the cursor after right-clicking the owning widget. The widget's container
    ///        updates it every frame, and Window::Draw draws the one shown at the end of the frame, over every scene.
    class ContextMenu final : public IBackground, public IBorder, public ITextProperties
    {
    public:
        /// @brief Constructs a ContextMenu for the given widget.
        ContextMenu(Projection* projection, IWidget* widget);
        ContextMenu() = delete;
        ContextMenu(const ContextMenu&) = delete;
        ContextMenu(ContextMenu&&) = delete;
        ~ContextMenu();
        auto operator =(const ContextMenu&) = delete;
        auto operator =(ContextMenu&&) = delete;

        /// @brief Tracks right-clicks on the owning widget and queues this ContextMenu for the end of the frame.
        void Update();

        /// @brief Draws the ContextMenu shown this frame over everything drawn so far. Window::Draw calls it last.
        static void DrawShown();

        /// @brief Sets the font of every ContextMenu, those created before this call included. A ContextMenu given its own font with SetFontFamily keeps it.
        static void SetDefaultFontFamily(std::string_view fontFamily, const f32 fontSize, const PlatformBridge::Fonts::Slant slant, const PlatformBridge::Fonts::Weight fontWeight);

        void SetBackgroundColors(std::span<const glm::vec4> colors);
        void SetBorderColors(std::span<const glm::vec4> colors);
        void SetCornerRadii(const glm::vec4& radii);
        void SetText(std::string_view text, const bool emitSignal) override;
        void SetFontFamily(std::string_view fontFamily, const f32 fontSize, const PlatformBridge::Fonts::Slant slant, const PlatformBridge::Fonts::Weight fontWeight) override;
        void SetTextPadding(const f32 padding) override;

        /// @brief Sets the border width. The box grows with it, since the border draws inside the box.
        void SetBorderWidth(const f32 borderWidth);

        /// @brief Adds an item with an optional SVG image and text/note labels to the named dropdown node. Use "Root" for the root dropdown.
        /// @param imagePath The SVG file path. Each item creates and owns its own SvgImage through ResourceManager.
        void AddItem(std::string_view itemName, std::string_view node, std::string_view imagePath, std::string_view text, std::string_view note);
        /// @brief Adds a dropdown node beneath the named dropdown node. Use "Root" for the root dropdown.
        void AddNode(std::string_view newNodeName, std::string_view node);
        /// @brief Connects to clicks on items in the root dropdown.
        void Connect_OnItemClicked(const typename Signal<>::Slot& slot, const bool async);
        /// @brief Returns the selected root dropdown item index.
        uSize GetSelectedIndex() const;

    private:
        friend class Window; //sets its own waiting ContextMenu aside while it draws a window docked into one of its tabs

        /// @brief Sets the ContextMenu waiting for the end of the frame, so a window drawn in the middle of another one's frame draws only its own.
        static void setShownAside();

        /// @brief Makes the ContextMenu set aside the waiting one again.
        static void restoreShownFromAside();

        void draw();
        void setSize();
        void setPosition();
        void interact();
        void setItemFont(ExtendedComboBox* comboBox) const;

        /// @brief Switches to the default font if it changed since this ContextMenu last took it, unless the ContextMenu has its own font.
        void applyDefaultFont();

        ExtendedComboBox* iterateNode(ExtendedComboBox* comboBox, std::string_view node);

        Projection* _projection { nullptr };
        IWidget* _widget { nullptr };
        std::unique_ptr<ExtendedComboBox> _items { nullptr };
        std::vector<glm::vec4>
            _backgroundColors {},
            _borderColors {};
        std::string _fontFamily {};
        f32 _fontSize { 16.0f };
        PlatformBridge::Fonts::Slant _fontSlant { PlatformBridge::Fonts::Slant::Roman };
        PlatformBridge::Fonts::Weight _fontWeight { PlatformBridge::Fonts::Weight::Normal };
        bool
            _isShown { false },
            _hasOwnFont { false },
            _wasRightButtonPressed { false },
            _hasPendingItemClose { false };
        glm::vec2 _pointerPosition { 0.0f };
        u32 _appliedDefaultFontVersion { 0 };

        inline static ContextMenu* _shownContextMenu { nullptr }; //drawn and cleared by DrawShown at the end of the frame
        inline static ContextMenu* _asideContextMenu { nullptr }; //a window's waiting ContextMenu, while a window docked into its tab draws
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
        static constexpr std::string_view _rootNodeName { "Root" };
    };
}