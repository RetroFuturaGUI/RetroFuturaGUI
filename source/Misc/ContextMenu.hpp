#pragma once
#include "ExtendedComboBox.hpp"
#include "Border.hpp"
#include "WidgetText.hpp"
#include "Projection.hpp"
#include "SvgImage.hpp"
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>
#include "Background.hpp"

namespace RetroFuturaGUI
{
    class IWidget;

    /// @brief Nested dropdown items shown near the cursor after right-clicking the owning widget
    class ContextMenu final
    {
    public:
        /// @brief Constructs a ContextMenu for the given widget
        ContextMenu(Projection* projection, IWidget* widget);
        ContextMenu() = delete;
        ContextMenu(const ContextMenu&) = delete;
        ContextMenu(ContextMenu&&) = delete;
        ~ContextMenu();
        auto operator =(const ContextMenu&) = delete;
        auto operator =(ContextMenu&&) = delete;

        /// @brief Tracks right-clicks on the owning widget, passes the input on to the items while shown, and queues this ContextMenu to be drawn last in this frame
        void Update();

        /// @brief Draws the ContextMenu shown this frame in the given window over everything drawn so far. Window::Draw calls it last
        static void DrawShown(GLFWwindow* window);

        /// @brief Sets the font of every ContextMenu, those created before this call included. A ContextMenu given its own font with SetFontFamily keeps it
        static void SetDefaultFontFamily(std::string_view fontFamily, const f32 fontSize, const FontSizeUnit sizeUnit, const PlatformBridge::Fonts::Slant slant, const PlatformBridge::Fonts::Weight fontWeight);

        void SetFontFamily(std::string_view fontFamily, const f32 fontSize, const FontSizeUnit sizeUnit, const PlatformBridge::Fonts::Slant slant, const PlatformBridge::Fonts::Weight fontWeight);

        /// @brief Adds an item with an optional SVG image and text/note labels to the named dropdown node. Use "Root" for the root dropdown
        /// @param imagePath The SVG file path. Each item creates and owns its own SvgImage through ResourceManager
        void AddItem(std::string_view itemName, std::string_view node, std::string_view imagePath, std::string_view text, std::string_view note);

        /// @brief Adds a dropdown node (a submenu) beneath the named dropdown node. Use "Root" for the root dropdown. Clicking its row
        ///        opens its list; only clicks on items report
        void AddNode(std::string_view newNodeName, std::string_view node);

        /// @brief Connects a slot called with the index path of each clicked item: the indices from the root list down to the
        ///        item, {2} for the third root item, {1, 0} for the first item in the submenu that is root row 1. The menu closes first
        void Connect_OnItemClicked(const typename Signal<std::vector<uSize>>::Slot& slot, const bool async);
        void Disconnect_OnItemClicked(const typename Signal<std::vector<uSize>>::Slot& slot);

        /// @brief Returns the index path of the item clicked last (see Connect_OnItemClicked), empty before any click
        const std::vector<uSize>& GetClickedPath() const;

        /// @brief Returns the widget's text: its string, font, colors and effects
        WidgetText& GetText();
        const WidgetText& GetText() const;

        /// @brief Returns the context menu's background
        Background& GetBackground();

        /// @brief Returns the context menu's background
        const Background& GetBackground() const;

        /// @brief Returns the context menu's border. The box grows with its width, since the border draws inside the box
        Border& GetBorder();
        
        /// @brief Returns the context menu's border
        const Border& GetBorder() const;

    private:
        void draw();
        void setSize();
        void setPosition();
        void interact();
        void setItemFont(ExtendedComboBox* comboBox) const;

        /// @brief Switches to the default font if it changed since this ContextMenu last took it, unless the ContextMenu has its own font
        void applyDefaultFont();

        /// @brief Has the box and the text follow the widget's transitions
        void useWidgetTransitions();

        ExtendedComboBox* iterateNode(ExtendedComboBox* comboBox, std::string_view node);

        /// @brief Has the node report the items picked in its list: a row that is an item closes the menu and emits its path, a
        ///        row that is a node only opens its own list
        void followNode(ExtendedComboBox& node);

        /// @brief Reports the item the node's list picked, unless it is a node's row
        void takeItemClick(ExtendedComboBox& node);

        /// @brief The indices from the given list down to the node, in path; false when the node isn't beneath it
        static bool findPath(const ExtendedComboBox& list, const ExtendedComboBox& node, std::vector<uSize>& path);

        /// @brief Closes the node's list and every list beneath it, so the menu opens again with only the root list open
        static void closeLists(ExtendedComboBox& node);

        /// @brief Hides the menu and closes its lists
        void hide();

        Projection& _projection;
        IWidget* _widget { nullptr };
        std::unique_ptr<ExtendedComboBox> _items { nullptr };
        Background _background { &_projection };
        Border _border { &_projection, { ._OnWidthChange = [this]() { setSize(); }, ._OnCornerRadiiChange = {} } };
        WidgetText _text { &_projection, [this](const WidgetTextChange) { setSize(); } };
        std::string _fontFamily {};
        f32 _fontSize { FontManager::ToPixels(16.0f, FontSizeUnit::Points) };
        PlatformBridge::Fonts::Slant _fontSlant { PlatformBridge::Fonts::Slant::Roman };
        PlatformBridge::Fonts::Weight _fontWeight { PlatformBridge::Fonts::Weight::Normal };
        bool
            _isShown { false },
            _hasOwnFont { false },
            _wasRightButtonPressed { false },
            _hasPendingItemClose { false };
        glm::vec2 _pointerPosition { 0.0f };
        std::vector<uSize> _clickedPath {};
        Signal<std::vector<uSize>>
            _onItemClicked,
            _onItemClickedAsync;
        TransitionValue<f32> _opacity {};
        u32 _appliedDefaultFontVersion { 0 };

        //Per window, since every window interacts before any draws: drawn and cleared by DrawShown at the end of its frame
        inline static std::unordered_map<GLFWwindow*, ContextMenu*> _shownContextMenus {};
        inline static std::string _defaultFontFamily {};
        inline static f32 _defaultFontSize { FontManager::ToPixels(16.0f, FontSizeUnit::Points) };
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