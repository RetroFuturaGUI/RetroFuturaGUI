#pragma once
#include "IWidget.hpp"
#include "Background.hpp"
#include "Border.hpp"
#include "Frame.hpp"
#include "Plane.hpp"
#include "SvgTexture.hpp"
#include "Icon.hpp"
#include "DropDownList.hpp"
#include "ResourceManager.hpp"
#include "Text.hpp"
#include <memory>
#include <vector>
#include "IClickable.hpp"

namespace RetroFuturaGUI
{
    class IDropDown : public IWidget, public IClickable
    {
    public:
        enum class DropDownPosition : u32
        {
            Bottom,
            Right,
            Top,
            Left
        };

        IDropDown(std::string_view name, Projection* projection, IHierarchyNode* parentWidget, const WidgetTypeID parentWidgetTypeID, GLFWwindow* parentWindow);
        ~IDropDown();
        virtual void Draw() = 0;

        /// @brief Draws the lists opened in this frame over everything drawn so far, in a fresh depth buffer.
        ///        Window::drawContent calls it after its scenes, so a list covers the widgets that come after its box.
        static void DrawShown();

        /// @brief Captures the pointer for a widget lying under the open list of another drop-down in its window, so a
        ///        click on the list reaches only the drop-down. IWidget::readPointer calls it for every widget.
        static void CaptureUnderOpenList(const IHierarchyNode* reader, GLFWwindow* readerWindow, PointerState& pointer);

        /// @brief Returns whether the open list of any drop-down in the window lies over a window point (bottom-up pixels). Lists
        ///        are drawn over everything, uncut, so a ScrollView lets the pointer through to one sticking out of it
        static bool IsOpenListAt(GLFWwindow* window, const glm::vec2& windowPoint);

        virtual void RemoveItem(const uSize index) = 0;
        void Connect_OnItemClicked(const typename Signal<>::Slot& slot, const bool async);
        void Connect_OnIndexChanged(const typename Signal<>::Slot& slot, const bool async);
        void Disconnect_OnItemClicked(const typename Signal<>::Slot& slot);
        void Disconnect_OnIndexChanged(const typename Signal<>::Slot& slot);
        uSize GetItemCount() const;
        uSize GetSelectedIndex() const;

        /// @brief Selects an item by code. An index past the last item selects the last one, and with no items nothing changes.
        ///        OnIndexChanged fires when the index changed and emitSignal is true; OnItemClicked is left to the pointer
        void SetIndex(const uSize index, const bool emitSignal);

        /// @brief Opens or closes the list without simulating a pointer click. It opens scrolled so the selected item shows
        void SetDropDownOpen(const bool open);

        /// @brief Sets how many rows the open list shows at most (8 by default, at least 1). The wheel scrolls through the rest
        void SetMaxVisibleItemCount(const uSize count);

        uSize GetMaxVisibleItemCount() const;
        void SetLabelHiddenWhenOpen(const bool hide);
        virtual bool IsPointInsideOpenDropDown(const glm::vec2& point) const;
        /// @brief Returns the arrow at the box's end, turned toward where the list opens
        Icon& GetArrow();

        /// @brief Returns the list the box opens: its background, border and the highlight behind the hovered row
        DropDownList& GetList();

        /// @brief Returns the list the box opens: its background, border and the highlight behind the hovered row
        const DropDownList& GetList() const;

        /// @brief Returns the arrow at the box's end, turned toward where the list opens
        const Icon& GetArrow() const;

        Background& GetBackground();
        const Background& GetBackground() const;
        Border& GetBorder();
        const Border& GetBorder() const;
        virtual void SetFontFamily(std::string_view fontFamily, const f32 fontSize, const FontSizeUnit sizeUnit, const PlatformBridge::Fonts::Slant slant, const PlatformBridge::Fonts::Weight fontWeight) = 0;
        f32 GetFontSize(const FontSizeUnit sizeUnit) const;
        virtual void SetTextAlignment(const TextAlignment alignment) = 0;
        virtual void SetTextPadding(const f32 padding) = 0;
        virtual void SetDropDownPosition(const DropDownPosition position) = 0;

        /// @brief Sets the open list's width, 0 for the box's own. A wider list keeps its near edge on the box's: the left edge below or above the box, so a narrow menu title can open a list of long entries.
        void SetDropDownWidth(const f32 width);

    protected:
        /// @brief The open list's width: the one set, or the box's.
        f32 getDropDownWidth() const;

        /// @brief Places the arrow at the box's right end, turned toward where the list opens. updateDropDownGeometry calls it.
        void updateDropDownArrowGeometry();

        /// @brief Draws the arrow, when its icon loaded.
        void drawDropDownArrow();

        /// @brief Places the open list and the arrow for the box's size, place, rotation and list position.
        void updateDropDownGeometry();

        /// @brief How many rows the open list shows: every item, up to the cap
        uSize getVisibleItemCount() const;

        /// @brief The item shown in the open list's first row; the wheel moves it
        uSize getFirstVisibleItemIndex() const;

        /// @brief Finds the row of the open list an item shows in, when it is among the visible ones
        bool getItemRow(const uSize itemIndex, uSize& outRow) const;

        void setColors(const ColorState state) override;

        /// @brief The center of a row of the open list, counted from its top
        glm::vec3 itemRowCenter(const uSize row) const;

        /// @brief The width the closed box leaves its text: the box without the arrow at its end
        f32 getLabelWidth() const;

        /// @brief The center of the closed box's text, left of the arrow, over the box
        glm::vec3 getLabelCenter() const;

        /// @brief Returns whether a world space point falls on the open drop down panel.
        bool isPointInsideDropDown(const glm::vec2& point) const;

        /// @brief Finds the item under a world space point among the open list's visible rows
        bool hoveredItemIndex(const glm::vec2& point, uSize& outIndex) const;

        /// @brief Opens or closes the list. Opening scrolls it so the selected item shows
        void setListOpen(const bool open);

        /// @brief Scrolls the open list by the wheel while the pointer is over it. Per notch as many rows as the system scrolls
        ///        lines, a list's worth for none. It takes the wheel, so nothing around the list scrolls too
        void scrollListByWheel(const PointerState& pointer, const bool isOverList);

        /// @brief Scrolls the open list so the item shows in one of its rows.
        void scrollItemIntoView(const uSize itemIndex);

        /// @brief Keeps the first visible row where no empty rows show below the last item. After a removal or a smaller cap
        void clampListScroll();

        /// @brief Whether the dropdown's label should be hidden in its current state.
        bool isLabelHidden() const { return _hideLabelWhenOpen && _showDropDown; }

        virtual void selectItem(const uSize index) = 0;

        /// @brief Draws the open list. DrawShown calls it at the end of the frame.
        virtual void drawDropDown() = 0;

        /// @brief Has DrawShown draw the list at the end of this frame while it is open or still fading out, and fades it over
        ///        the Opacity transition. Called from every Draw.
        void showDropDownLater();

    //Elements
        Background _background { &_projection };
        Border _border { &_projection };
        DropDownList _list { &_projection };
        std::shared_ptr<Text> _previewText { nullptr };
        Icon _arrow //its own, so its colors and place are this box's alone
        {
            ResourceManager::CreateDropDownArrowIcon(&_projection),
            "DropDownArrow",
            SvgPathFill { ._FillType = FillType::Solid, ._Colors = {}, ._GradientDegree = 35.0f, ._GradientOffset = 0.0f, ._GradientAnimationSpeed = 0.05f, ._GradientRotationSpeed = 5.0f },
            IconColors
            {
                ._Enabled = { glm::vec4(0.7f, 0.7f, 0.7f, 1.0f) },
                ._Disabled = { glm::vec4(0.3f, 0.3f, 0.3f, 1.0f) },
                ._Clicked = { glm::vec4(1.0f, 1.0f, 1.0f, 1.0f) },
                ._Hover = { glm::vec4(0.85f, 0.85f, 0.85f, 1.0f) },
                ._Selected = {}
            }
        };

    //Design
        static constexpr const f32 _zStep { 0.01f };
        TextAlignment _textAlignment { TextAlignment::Left };
        std::string _fontFamily {};
        f32
            _fontSize { FontManager::ToPixels(11.0f, FontSizeUnit::Points) }, //in pixels, as every stored font size
            _textPadding { 4.0f },
            _dropDownWidth { 0.0f }; //0: the box's width
        PlatformBridge::Fonts::Slant _slant { PlatformBridge::Fonts::Slant::Roman };
        PlatformBridge::Fonts::Weight _fontWeight { PlatformBridge::Fonts::Weight::Normal };

    //Logic
        DropDownPosition _dropDownPosition { DropDownPosition::Bottom };
        uSize
            _currentIndex { 0 },
            _itemCount { 0 },
            _nthItemHovered { 0 }; //item index
        Signal<>
            _onItemClicked,
            _onItemClickedAsync,
            _onIndexChanged,
            _onIndexChangedAsync;
        bool
            _showDropDown { false },
            _dropDownItemHovered { false },
            _holdsPickPress { false }; //the press that picked a row is still down, so the closed list keeps it from the widgets under it
        TransitionValue<f32> _listOpacity {}; //fades the list in when it opens and out when it closes

    private:
        friend class Window; //sets the waiting lists aside while it draws a window docked into one of its tabs
        friend class Environment; //sets them aside while it draws its world into an EnvironmentView's picture

        /// @brief Takes the lists waiting for the end of the frame, so a window drawn in the middle of another one's frame draws only its own.
        static std::vector<IDropDown*> takeShown();

        /// @brief Puts lists taken by takeShown back.
        static void restoreShown(std::vector<IDropDown*>&& shown);

        /// @brief Returns whether the list lies over a window point (bottom-up pixels) in the reader's window, and the reader is neither this drop-down nor inside it
        bool isListOver(const IHierarchyNode* reader, GLFWwindow* readerWindow, const glm::vec2& windowPoint) const;

        glm::vec3 getDropDownCenter() const;

    //Design
        static const constexpr f32
            _zOffset { 0.05f },
            _arrowPadding { 15.0f };

    //Logic
        uSize
            _maxVisibleItemCount { 8 },
            _firstVisibleItemIndex { 0 };
        f32
            _wheelRowRemainder { 0.0f },
            _arrowSpace { 0.0f };
        inline static std::vector<IDropDown*> _shownDropDowns {}; //opened in the frame being drawn, drawn and cleared by DrawShown
        inline static std::vector<const IDropDown*> _dropDowns {}; //every drop-down, for the pointer check
        bool _hideLabelWhenOpen { false };
        glm::mat4 _listFrame { 1.0f }; //the frame the widget drew in, which its open list draws in later
        glm::mat4 _viewListFrame { 1.0f }; //the frame it drew in last in an EnvironmentView's picture, where its open list shows too.
    };
}