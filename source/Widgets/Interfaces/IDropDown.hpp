#pragma once
#include "IWidget.hpp"
#include "IBackground.hpp"
#include "IBorder.hpp"
#include "Rectangle.hpp"
#include "SvgTexture.hpp"
#include "Text.hpp"
#include <memory>
#include <vector>
#include "IClickable.hpp"

namespace RetroFuturaGUI
{
    class IDropDown : public IWidget, public IClickable, public IBackground, public IBorder
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

        virtual void RemoveItem(const uSize index) = 0;
        void Connect_OnItemClicked(const typename Signal<>::Slot& slot, const bool async);
        void Connect_OnIndexChanged(const typename Signal<>::Slot& slot, const bool async);
        void Disconnect_OnItemClicked(const typename Signal<>::Slot& slot);
        void Disconnect_OnIndexChanged(const typename Signal<>::Slot& slot);
        uSize GetItemCount() const;
        uSize GetSelectedIndex() const;
        void SetIndex(const uSize index);
        virtual bool IsPointInsideOpenDropDown(const glm::vec2& point) const;
        void SetDropDownArrowColors(std::span<glm::vec4> colors, const ColorState state);
        void SetDropDownBackgroundColors(std::span<glm::vec4> colors, const ColorState state);
        void SetDropDownBorderColors(std::span<glm::vec4> colors, const ColorState state);
        void SetDropDownCornerRadii(const glm::vec4& radii);
        virtual void SetFontFamily(std::string_view fontFamily, const f32 fontSize, const PlatformBridge::Fonts::Slant slant, const PlatformBridge::Fonts::Weight fontWeight) = 0;
        virtual void SetTextAlignment(const TextAlignment alignment) = 0;
        virtual void SetTextPadding(const f32 padding) = 0;
        virtual void SetDropDownPosition(const DropDownPosition position) = 0;

        /// @brief Sets the open list's width, 0 for the box's own. A wider list keeps its near edge on the box's: the left edge below or above the box, so a narrow menu title can open a list of long entries.
        void SetDropDownWidth(const f32 width);

    protected:
        /// @brief The open list's width: the one set, or the box's.
        f32 dropDownWidth() const;

        void updateDropDownArrowGeometry();
        void updateDropDownGeometry();
        uSize visibleItemCount() const;
        virtual void setColors(const ColorState state);
        glm::vec3 itemRowCenter(const uSize visibleIndex) const;

        /// @brief Returns whether a world space point falls on the open drop down panel.
        bool isPointInsideDropDown(const glm::vec2& point) const;

        /// @brief Finds the visible drop down row under a world space point.
        bool hoveredItemIndex(const glm::vec2& point, uSize& outIndex) const;

        virtual void selectItem(const uSize index) = 0;

        /// @brief Draws the open list. DrawShown calls it at the end of the frame.
        virtual void drawDropDown() = 0;

        /// @brief Has DrawShown draw the open list at the end of this frame. Called from Draw.
        void showDropDownLater();

    //Elements
        std::unique_ptr<Rectangle>
            _dropDownBackground { nullptr },
            _dropDownItemHighlight { nullptr },
            _dropDownBorder { nullptr },
            _dropDownItemBorder { nullptr };
        std::shared_ptr<Text> _previewText { nullptr };
        std::shared_ptr<SvgTexture> _dropDownArrow { nullptr };

    //Design
        static constexpr const f32 _zStep { 0.01f };
        TextAlignment _textAlignment { TextAlignment::Left };
        std::string _fontFamily {};
        f32
            _fontSize { 11.0f },
            _textPadding { 4.0f },
            _dropDownWidth { 0.0f }; //0: the box's width
        PlatformBridge::Fonts::Slant _slant { PlatformBridge::Fonts::Slant::Roman };
        PlatformBridge::Fonts::Weight _fontWeight { PlatformBridge::Fonts::Weight::Normal };

    //Logic
        DropDownPosition _dropDownPosition { DropDownPosition::Bottom };
        uSize
            _currentIndex { 0 },
            _itemCount { 0 },
            _maxItemsToDraw { 8 },
            _nthItemHovered { 0 };
        Signal<>
            _onItemClicked,
            _onItemClickedAsync,
            _onIndexChanged,
            _onIndexChangedAsync;
        bool
            _showDropDown { false },
            _dropDownItemHovered { false },
            _holdsPickPress { false }; //the press that picked a row is still down, so the closed list keeps it from the widgets under it

    private:
        friend class Window; //sets the waiting lists aside while it draws a window docked into one of its tabs

        /// @brief Takes the lists waiting for the end of the frame, so a window drawn in the middle of another one's frame draws only its own.
        static std::vector<IDropDown*> takeShown();

        /// @brief Puts lists taken by takeShown back.
        static void restoreShown(std::vector<IDropDown*>&& shown);

        /// @brief Returns whether the list lies over a point in the reader's window, and the reader is neither this drop-down nor inside it.
        bool isListOver(const IHierarchyNode* reader, GLFWwindow* readerWindow, const glm::vec2& point) const;

        void setDropDownArrowColors();
        void setDropDownBackgroundColors();
        void setDropDownBorderColors();
        glm::vec3 dropDownCenter() const;
        glm::vec2 rotateLocalOffset(const glm::vec2& localOffset) const;

    //Design
        ColorState
            _dropDownArrowColorState { ColorState::Enabled },
            _dropDownBackgroundColorSate { ColorState::Enabled },
            _dropDownBorderColorSate { ColorState::Enabled };
        std::vector<glm::vec4>
            _borderColorsEnabled { { 0.7f, 0.7f, 0.7f, 1.0f } },
            _borderColorsDisabled { { 0.3f, 0.3f, 0.3f, 1.0f } },
            _borderColorsHover { { 0.85f, 0.85f, 0.85f, 1.0f } },
            _borderColorsClicked { { 1.0f, 1.0f, 1.0f, 1.0f } },
            _backgroundColorsEnabled { { 0.25f, 0.25f, 0.8f, 1.0f } },
            _backgroundColorsDisabled { { 0.1f, 0.1f, 0.45f, 1.0f } },
            _backgroundColorsHover { { 0.3f, 0.3f, 0.9f, 1.0f } },
            _backgroundColorsClicked { { 0.4f, 0.4f, 1.0f, 1.0f } },
            _dropDownArrowColorsEnabled { { 0.7f, 0.7f, 0.7f, 1.0f } },
            _dropDownArrowColorsDisabled { { 0.3f, 0.3f, 0.3f, 1.0f } },
            _dropDownArrowColorsHover { { 0.85f, 0.85f, 0.85f, 1.0f } },
            _dropDownArrowColorsClicked { { 1.0f, 1.0f, 1.0f, 1.0f } },
            _dropDownBorderColorsEnabled { { 0.7f, 0.7f, 0.7f, 1.0f } },
            _dropDownBorderColorsDisabled { { 0.3f, 0.3f, 0.3f, 1.0f } },
            _dropDownBorderColorsHover { { 0.85f, 0.85f, 0.85f, 1.0f } },
            _dropDownBorderColorsClicked { { 1.0f, 1.0f, 1.0f, 1.0f } },
            _dropDownBackgroundColorsEnabled { { 0.25f, 0.25f, 0.8f, 1.0f } },
            _dropDownBackgroundColorsDisabled { { 0.1f, 0.1f, 0.45f, 1.0f } },
            _dropDownBackgroundColorsHover { { 0.3f, 0.3f, 0.9f, 1.0f } },
            _dropDownBackgroundColorsClicked { { 0.4f, 0.4f, 1.0f, 1.0f } },
            _highLightColor { { 0.3f, 0.9f, 2.0f, 1.0f } };
        SvgPathFill _dropDownArrowPathFill
        {
            SvgPathFill
            {
                .fillType = FillType::SOLID,
                .colors = { glm::vec4(1.0f) },
                .gradientDegree = 35.0f,
                .gradientOffset = 0.0f,
                .gradientAnimationSpeed = 0.05f,
                .gradientRotationSpeed = 5.0f
            }
        };
        static const constexpr f32
            _zOffset { 0.05f },
            _arrowPadding { 15.0f };

    //Logic
        inline static std::vector<IDropDown*> _shownDropDowns {}; //opened in the frame being drawn, drawn and cleared by DrawShown
        inline static std::vector<const IDropDown*> _dropDowns {}; //every drop-down, for the pointer check
    };
}