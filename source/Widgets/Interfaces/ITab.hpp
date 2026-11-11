#pragma once
#include "Background.hpp"
#include "IncludeHelper.hpp"
#include "IWidget.hpp"
#include "IContainer.hpp"
#include "Border.hpp"
#include "Box.hpp"
#include "IconButton.hpp"
#include "ResourceManager.hpp"
#include "Text.hpp"
#include "Transition.hpp"
#include <memory>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace RetroFuturaGUI
{
    /// @brief What Tab and WindowTab share. A strip of fields along the top, one per item (a page, a docked window), each with
    ///        its title and a close button, and the item of the selected field shown in the content area below. Pressing a field
    ///        selects it, dragging it sideways reorders the fields, its cross closes it. What an item is, what closing it does
    ///        and how the selected one shows come from the derived class
    class ITab : public IWidget, public IContainer
    {
    public:
        ITab(std::string_view name, Projection* projection, IHierarchyNode* parentWidget, const WidgetTypeID parentWidgetTypeID, GLFWwindow* parentWindow);
        ITab() = delete;
        ITab(const ITab&) = delete;
        ITab(ITab&&) = delete;
        auto operator =(const ITab&) = delete;
        auto operator =(ITab&&) = delete;
        ~ITab() override = default;

    //Strip
        /// @brief Sets the height of the strip the fields sit in
        void SetStripHeight(const f32 height);

        /// @brief Sets how narrow and how wide a field may get. Fields share the strip evenly within that range. Once they no
        ///        longer fit at the minimum width, the strip scrolls. By the wheel over it, by the arrows at its ends, and to
        ///        a field as it is selected or dragged past an end
        void SetFieldWidthRange(const f32 minimumWidth, const f32 maximumWidth);

        /// @brief Sets the gap between fields, and above them
        void SetFieldSpacing(const f32 spacing);

    //Titles
        /// @brief Sets the font of the titles. Without one they don't show
        void SetTitleFontFamily(std::string_view fontFamily, const f32 fontSize, const FontSizeUnit sizeUnit, const PlatformBridge::Fonts::Slant slant, const PlatformBridge::Fonts::Weight weight);

        /// @brief Returns the title font size in the given unit
        f32 GetTitleFontSize(const FontSizeUnit sizeUnit) const;

        /// @brief Sets a title's colors for Enabled, Disabled, Hover or Clicked. The selected field's title shows the same colors as
        ///        the others, so Selected is ignored
        void SetTitleColors(std::span<const glm::vec4> colors, const ColorState state);

        /// @brief Sets the space between a field's left edge and its title
        void SetTitlePadding(const f32 padding);

    //Close buttons
        /// @brief Sets the width and height of the close buttons
        void SetCloseButtonSize(const f32 size);

        /// @brief Returns the close button every field draws. Its background, transparent until hovered by default, and its
        ///        cross. Each field's button shows that field's state. Its corners round with a quarter of SetCloseButtonSize
        IconButton& GetCloseButton();

        /// @brief Returns the close button every field draws (see the other overload)
        const IconButton& GetCloseButton() const;

        /// @brief Returns the scroll arrows at the strip's ends, which show while fields are hidden on their side. Their
        ///        background and their arrow, as large as the close buttons. One button draws both, the right one turned
        IconButton& GetScrollButton();

        /// @brief Returns the scroll arrows (see the other overload).
        const IconButton& GetScrollButton() const;

        /// @brief Returns the look of the fields. Enabled, Hover, Clicked and Disabled, and Selected for the selected field, which
        ///        usually matches the content so the two join. Rounded at the top, without a border by default
        Box& GetFieldStyle();

        /// @brief Returns the look of the fields; see the other overload
        const Box& GetFieldStyle() const;

    //Content
        /// @brief Sets the corner radii of the content area, clockwise from the top-left. A WindowTab's docked window rounds its
        ///        own corners to them; a Tab rounds its background and border at the bottom to them
        void SetContentCornerRadii(const glm::vec4& radii);

        /// @brief Returns the corner radii of the content area
        const glm::vec4& GetContentCornerRadii() const;

        /// @brief Returns the background, under the strip and the content area
        Background& GetBackground();

        /// @brief Returns the background, under the strip and the content area
        const Background& GetBackground() const;

        /// @brief Returns the border around the strip and the content area
        Border& GetBorder();

        /// @brief Returns the border around the strip and the content area
        const Border& GetBorder() const;

    protected:
        /// @brief One field. Its item and the title it shows
        struct Field
        {
            IHierarchyNode* _Item { nullptr }; //a Tab's page or a WindowTab's window
            std::string _Title {}; //what _TitleText shows
            std::unique_ptr<Text> _TitleText { nullptr };
            TransitionValue<glm::vec2> _Shown {}; //the field's left edge and width as drawn, on their way to its slot
            bool _HasShown { false }; //a field not shown yet stands in its slot at once
        };

        /// @brief The colors for every state a field and its parts can be in
        struct FieldColors
        {
            std::vector<glm::vec4>
                _Enabled { glm::vec4(0.0f) },
                _Hover { glm::vec4(0.0f) },
                _Clicked { glm::vec4(0.0f) },
                _Disabled { glm::vec4(0.0f) },
                _Selected { glm::vec4(0.0f) };
        };

        /// @brief Sets the center of the whole strip and content area together
        void applyPosition(const glm::vec3& position) override;

        /// @brief Sets the size of the whole strip and content area together
        void applySize(const glm::vec3& size) override;

    //Hooks for the derived class
        /// @brief The field was selected. Emits the selection signal when emitSignal is set
        virtual void fieldSelected(IHierarchyNode* item, const bool emitSignal) = 0;

        /// @brief The last field went, so nothing is selected anymore. Emits the selection signal with nullptr
        virtual void selectionCleared() = 0;

        /// @brief What the field's cross does. A Tab removes the page, a WindowTab asks the window to close
        virtual void closeField(const uSize index) = 0;

        /// @brief Whether the fields show their cross. True unless overridden
        virtual bool showsCloseButtons() const;

        /// @brief Called while a field is dragged, past the drag threshold. True when the drag took the field off the strip, so it
        ///        isn't reordered. False unless overridden
        virtual bool dragsOutOfStrip(const uSize index, const glm::vec2& point);

        /// @brief The fields' order changed by a drag. Nothing unless overridden
        virtual void fieldsReordered();

        /// @brief How many slots the fields share the strip with. One per field, unless overridden (a drop preview takes one too)
        virtual uSize getSlotCount() const;

        /// @brief The strip or the content area changed size or place. Nothing unless overridden
        virtual void contentAreaChanged();

        /// @brief SetContentCornerRadii changed them. Nothing unless overridden
        virtual void contentCornerRadiiChanged();

        /// @brief SetTitleFontFamily gave the titles another font, for titles of the derived class's own. Nothing unless overridden
        virtual void titleFontChanged();

    //Fields
        /// @brief Adds a field for the item at the given place among the fields, with its title, and returns its index
        uSize insertField(IHierarchyNode* item, std::string_view title, const uSize index);

        /// @brief Selects the field's item, telling the derived class when it changed.
        void selectField(const uSize index, const bool emitSignal);

        /// @brief Sets the title a field shows
        void setFieldTitle(const uSize index, std::string_view title);

        /// @brief Takes the field off the strip and lets go of the pointer's hold on it.
        /// @return Whether its item was the selected one
        bool eraseField(const uSize index);

        /// @brief After the selected field was erased from the given place. The field now there is selected, or the one before it
        ///        when it was the last, or the selection is cleared when none is left
        void reselectAfterErase(const uSize index);

        uSize indexOf(const IHierarchyNode* item) const;

    //Interaction
        /// @brief Selects or closes on a press, and drags the pressed field along the strip until the button is let go
        void followPointer();

        /// @brief Selects the field and remembers where it was grabbed, in case the press turns into a drag
        void pressField(const uSize index, const glm::vec2& point);

        /// @brief Moves the pressed field with the cursor along the strip, reordering the fields
        void dragField(const glm::vec2& point);

        /// @brief The field under the point, or _kNoField
        uSize fieldAt(const glm::vec2& point) const;

        /// @brief The state a field draws in, given what the pointer is doing
        ColorState fieldState(const uSize index) const;

    //Layout, in window coordinates with y running upward. A rectangle is (left, bottom, width, height)
        glm::vec4 getStripRect() const;
        glm::vec4 getContentRect() const;
        f32 getFieldWidth() const;

        /// @brief Where the field at the given place sits, ignoring a drag
        glm::vec4 slotRect(const uSize slot) const;

        /// @brief Where the field is drawn: on its way to its slot, or under the cursor while it is dragged
        glm::vec4 fieldRect(const uSize index) const;

        /// @brief Returns the left edge and width a field is moving toward: its slot's, or where the cursor drags it
        glm::vec2 getFieldTarget(const uSize index) const;

        /// @brief Moves every field toward its slot over the Position transition. The dragged field follows the cursor at once
        void syncFieldRects();

        /// @brief The place along the strip whose slot the given x lies over
        uSize slotAt(const f32 x) const;

        /// @brief Whether any of the field lies in the field area. A field partly out of it is cut at its edge
        bool isInFieldArea(const glm::vec4& field) const;

        /// @brief Where the fields show. The whole strip, or between the scroll arrows while they don't fit
        glm::vec4 getFieldArea() const;

        /// @brief Whether the fields need more room than the strip has, at their minimum width
        bool isOverflowing() const;

        /// @brief How far the strip can scroll. How much wider the fields are than the field area
        f32 getMaxStripScroll() const;

        /// @brief Scrolls the strip by the given distance, kept within its range
        void scrollStrip(const f32 distance);

        /// @brief Scrolls the strip just enough to show the slot whole
        void scrollSlotIntoView(const uSize slot);

        /// @brief The scroll arrow on the given side, (left, bottom, width, height)
        glm::vec4 scrollButtonRect(const bool isRight) const;

        /// @brief Whether fields are hidden on the given side, so its arrow shows
        bool hasHiddenFields(const bool isRight) const;

        /// @brief Scrolls by the wheel over the strip, a field per notch, horizontally or vertically
        void scrollByWheel(const PointerState& pointer);

        glm::vec4 closeButtonRect(const glm::vec4& field) const;
        glm::vec4 titleRect(const glm::vec4& field) const;

    //Drawing
        /// @brief Draws every field, the dragged one over the others
        void drawFields();
        void drawField(const uSize index, const f32 z);
        void drawFieldShape(const glm::vec4& rect, const f32 z, Box& style, const ColorState state);
        void drawCloseButton(const glm::vec4& field, const f32 z, const ColorState state);

        /// @brief Draws the scroll arrows of the sides with hidden fields
        void drawScrollButtons(const f32 z);
        void drawScrollButton(const bool isRight, const f32 z);
        void drawTitle(Text* title, const glm::vec4& field, const f32 z, const std::vector<glm::vec4>& colors);
        void applyTitleFont(Text* title) const;

        /// @brief The state a field's look shows. Disabled, Selected for the selected field, or the field's own
        static ColorState fieldStyleState(const ColorState state, const bool selected);

        /// @brief The colors to draw with. The selected ones for the selected field unless it is disabled, else the state's
        static const std::vector<glm::vec4>& colorFor(const FieldColors& colors, const ColorState state, const bool selected);

        /// @brief The colors stored for the state
        static std::vector<glm::vec4>& colorOf(FieldColors& colors, const ColorState state);

        /// @brief Whether any of the colors can be seen, so a fully transparent part isn't drawn at all
        static bool isVisible(std::span<const glm::vec4> colors);

        static bool contains(const glm::vec4& rect, const glm::vec2& point);

        /// @brief The UTF-8 path of an image in the Resources/img folder next to the executable.
        static std::string getImagePath(std::string_view fileName);

    //Elements
        std::vector<Field> _fields {};
        Background _background { &_projection };
        Border _border { &_projection };
        Box _fieldStyle { &_projection };
        IconButton
            _closeButton { &_projection, ResourceManager::CreateSvgTexture(getImagePath("CloseIcon.svg"), &_projection, glm::i32vec2(32)), "CloseIcon" }, //its own, so its colors and size are this tab's alone.
            _scrollButton { &_projection, ResourceManager::CreateSvgTexture(getImagePath("ScrollArrow.svg"), &_projection, glm::i32vec2(32)), "ScrollArrow" };

    //Design
        f32
            _stripHeight { 36.0f },
            _minimumFieldWidth { 80.0f },
            _maximumFieldWidth { 240.0f },
            _fieldSpacing { 4.0f },
            _titlePadding { 12.0f },
            _closeButtonSize { 24.0f };
        glm::vec4 _contentCornerRadii { 0.0f };
        FieldColors
            _titleColors
            {
                ._Enabled = { glm::vec4(0.7f, 0.7f, 0.75f, 1.0f) },
                ._Hover = { glm::vec4(0.9f, 0.9f, 0.95f, 1.0f) },
                ._Clicked = { glm::vec4(1.0f) },
                ._Disabled = { glm::vec4(0.4f, 0.4f, 0.45f, 1.0f) },
                ._Selected = { glm::vec4(1.0f) }
            };
        std::string _titleFontFamily {};
        f32 _titleFontSize { FontManager::ToPixels(18.0f, FontSizeUnit::Points) }; //in pixels, as every stored font size
        PlatformBridge::Fonts::Slant _titleFontSlant { PlatformBridge::Fonts::Slant::Roman };
        PlatformBridge::Fonts::Weight _titleFontWeight { PlatformBridge::Fonts::Weight::Normal };

    //Logic
        static constexpr uSize _kNoField { ~uSize(0) };
        static constexpr f32
            _zStep { 0.01f },
            _closeIconScale { 0.4f },
            _dragThreshold { 5.0f },
            _dragScrollSpeed { 600.0f };

        /// @brief Which scroll arrow the pointer is over or pressed
        enum class ScrollSide : u32
        {
            None,
            Left,
            Right
        };

        IHierarchyNode
            *_selectedItem { nullptr },
            *_pressedItem { nullptr };
        uSize
            _hoveredField { _kNoField },
            _hoveredCloseButton { _kNoField },
            _pressedCloseButton { _kNoField };
        glm::vec2
            _pressPoint { 0.0f },
            _grabOffset { 0.0f };
        f32
            _dragLeft { 0.0f }, //the dragged field's left edge, which follows the cursor
            _sidewaysTravel { 0.0f }, //how far the drag went sideways at most
            _stripScroll { 0.0f }; //how far the fields are scrolled to the left while they don't fit
        ScrollSide
            _hoveredScrollButton { ScrollSide::None },
            _pressedScrollButton { ScrollSide::None };
        StepClock _dragScrollClock {}; //the time between frames while a dragged field scrolls the strip
        bool
            _isDraggingField { false },
            _wasPressed { false };

    private:
        /// @brief Gives the field look its default colors, corners and border width
        void setFieldStyleDefaults();
    };
}