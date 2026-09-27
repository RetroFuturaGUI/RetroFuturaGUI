#pragma once
#include "IncludeHelper.hpp"
#include "IWidget.hpp"
#include "IBackground.hpp"
#include "IBorder.hpp"
#include "IClickable.hpp"
#include "Label.hpp"
#include "Button.hpp"
#include "Rectangle.hpp"
#include "TextBox.hpp"
#include "SvgImage.hpp"
#include "Image.hpp"
#include "CheckBox.hpp"
#include "Slider.hpp"
#include "ProgressBar.hpp"
#include "Prefab.hpp"
#include "ComboBox.hpp"
#include <memory>

namespace RetroFuturaGUI
{
    template<typename T>
    concept MenuBarWidgetTypes =
       std::same_as<T, Label>  || std::same_as<T, Button> || std::same_as<T, TextBox>
    || std::same_as<T, Image> || std::same_as<T, SvgImage> || std::same_as<T, CheckBox>
    || std::same_as<T, Slider> || std::same_as<T, ProgressBar> || std::same_as<T, Prefab>
    || std::same_as<T, ComboBox>;

    class MenuBar final : public IWidget, public IBackground, public IBorder, public IClickable
    {
    public:
        enum class DockingEdge : u32
        {
            Left,
            Right,
            Top,
            Bottom
        };

        enum class EdgeAlignment : u32
        {
            Start,
            Center,
            End
        };

        MenuBar(const std::string& name, Projection* projection, IWidget* parentWidget, const WidgetTypeID parentWidgetTypeID, GLFWwindow* parentWindow);
        MenuBar() = delete;
        MenuBar(const Button&) = delete;
        MenuBar(MenuBar&&) = delete;
        auto operator =(const MenuBar&) = delete;
        auto operator =(MenuBar&&) = delete;
        ~MenuBar() = default;

        void Draw() override;
        void SetPosition(const glm::vec3& position) override;
        void SetSize(const glm::vec3& size) override;
        void SetRotation(const glm::vec3& rotation) override;
        void SetMargin(const f32 margin);
        void SetDockingEdge(const DockingEdge dockingEdge);
        void SetEdgeAlignment(const EdgeAlignment edgeAlignment);
        void SetFlexCount(const uSize count);

        /// @brief Takes ownership of a widget and puts it in the given flex slot
        template<MenuBarWidgetTypes T> void AddWidget(std::unique_ptr<T> widget, const uSize index)
        {
            if(!widget)
                return;

            if(index >= _itemFlex.size())
                return;

            *std::next(_itemFlex.begin(), static_cast<i64>(index)) = std::move(widget);
            placeItems();
        }

        /// @brief Returns the widget in the given flex slot, or nullptr when the slot is empty, out of range, or holds a widget of another type.
        template<MenuBarWidgetTypes T> T* GetWidget(const uSize index) const
        {
            if(index >= _itemFlex.size())
                return nullptr;

            return dynamic_cast<T*>(std::next(_itemFlex.begin(), static_cast<i64>(index))->get());
        }

        bool RemoveWidget(const uSize index);
        void EnableSeparatorLines(const bool enable);
        void SetSeparatorLinesThickness(const f32 thickness);

    //Separator lines
        /// @brief Sets a single separator line color for the given color state.
        void SetSeparatorLinesColor(const glm::vec4& color, const ColorState state);

        /// @brief Sets multiple separator line colors (e.g. gradient stops) for the given color state.
        void SetSeparatorLinesColors(std::span<glm::vec4> colors, const ColorState state);

        /// @brief Sets the separator lines' fill type (solid, linear/radial/huestar gradient).
        void SetSeparatorLinesFillType(const FillType fillType);

        /// @brief Sets the offset applied to the separator lines' gradient start position.
        void SetSeparatorLinesGradientOffset(const f32 gradientOffset);

        /// @brief Sets the speed at which the separator lines' gradient animates over time.
        void SetSeparatorLinesGradientAnimationSpeed(const f32 animationSpeed);

        /// @brief Sets the angle of the separator lines' linear gradient, in degrees.
        void SetSeparatorLinesGradientDegree(const f32 degree);

        /// @brief Sets the speed at which the separator lines' gradient rotates over time.
        void SetSeparatorLinesGradientRotationSpeed(const f32 rotationSpeed);

        /// @brief Sets sections of the separator lines to skip drawing, which is what turns them into dashed lines; see Rectangle::SetBackgroundGaps.
        void SetSeparatorLinesGaps(const BackgroundGap& gap);

    //Item background
        /// @brief Sets a single item background color for the given color state.
        void SetItemBackgroundColor(const glm::vec4& color, const ColorState state);

        /// @brief Sets multiple item background colors (e.g. gradient stops) for the given color state.
        void SetItemBackgroundColors(std::span<glm::vec4> colors, const ColorState state);

        /// @brief Sets the corner rounding radii of the item background and item border.
        void SetItemCornerRadii(const glm::vec4& radii);

        /// @brief Sets the item background fill type (solid, linear/radial/huestar gradient).
        void SetItemBackgroundFillType(const FillType fillType);

        /// @brief Sets the offset applied to the item background gradient's start position.
        void SetItemBackgroundGradientOffset(const f32 gradientOffset);

        /// @brief Sets the speed at which the item background gradient animates over time.
        void SetItemBackgroundGradientAnimationSpeed(const f32 animationSpeed);

        /// @brief Sets the angle of the item background's linear gradient, in degrees.
        void SetItemBackgroundGradientDegree(const f32 degree);

        /// @brief Sets the speed at which the item background gradient rotates over time.
        void SetItemBackgroundGradientRotationSpeed(const f32 rotationSpeed);

        /// @brief Sets the texture ID sampled for the item background's glass-effect-with-image shader feature.
        void SetItemWindowBackgroundImageTextureID(const u32 textureID);

        /// @brief Sets the dot color used for the item background's DottedPattern shader feature.
        void SetItemBackgroundDotColor(const glm::vec4& color);

        /// @brief Sets the spacing between dot centers, in pixels, for the item background's DottedPattern shader feature.
        void SetItemBackgroundDotDistance(const f32 distance);

        /// @brief Sets the direction, in degrees, along which the item background's dot radii/animation transfer.
        void SetItemBackgroundDotSizeTransferDegree(const f32 degree);

        /// @brief Sets the per-position dot radius curve, in pixels, for the item background's DottedPattern shader feature. Enables the feature when non-empty.
        void SetItemBackgroundDotRadiusTransfer(std::span<f32> radiusTransfer);

        /// @brief Sets how far each item background dot's opacity reaches from its center before fading to transparent.
        void SetItemBackgroundDotTransparencyTransfer(const f32 transparencyTransfer);

        /// @brief Sets the speed at which the item background's dotted pattern animates.
        void SetItemBackgroundDotAnimationSpeed(const f32 animationSpeed);

        /// @brief Sets the overall opacity of the item background's FogEffect shader feature.
        void SetItemBackgroundFogAlpha(const f32 alpha);

        /// @brief Sets the speed at which the item background's fog drifts over time.
        void SetItemBackgroundFogSpeed(const f32 speed);

        /// @brief Sets the per-octave density/weight curve driving the item background's fog. Enables the FogEffect shader feature when non-empty.
        void SetItemBackgroundFogDensity(std::span<f32> density);

        /// @brief Sets the coverage threshold above which the item background's fog appears; higher values carve larger clear gaps out of the cloud.
        void SetItemBackgroundFogClearing(const f32 clearing);

        /// @brief Sets sections of the item background to skip drawing; see Rectangle::SetBackgroundGaps.
        void SetItemBackgroundGaps(const BackgroundGap& gap);

    //Item border
        /// @brief Sets a single item border color for the given color state.
        void SetItemBorderColor(const glm::vec4& color, const ColorState state);

        /// @brief Sets multiple item border colors (e.g. gradient stops) for the given color state.
        void SetItemBorderColors(std::span<glm::vec4> colors, const ColorState state);

        /// @brief Sets the item border fill type (solid, linear/radial/huestar gradient).
        void SetItemBorderFillType(const FillType fillType);

        /// @brief Sets the offset applied to the item border gradient's start position.
        void SetItemBorderGradientOffset(const f32 gradientOffset);

        /// @brief Sets the speed at which the item border gradient animates over time.
        void SetItemBorderGradientAnimationSpeed(const f32 animationSpeed);

        /// @brief Sets the angle of the item border's linear gradient, in degrees.
        void SetItemBorderGradientDegree(const f32 degree);

        /// @brief Sets the speed at which the item border gradient rotates over time.
        void SetItemBorderGradientRotationSpeed(const f32 rotationSpeed);

        /// @brief Sets the texture ID sampled for the item border's glass-effect-with-image shader feature.
        void SetItemWindowBorderImageTextureID(const u32 textureID);

        /// @brief Sets the dot color used for the item border's DottedPattern shader feature.
        void SetItemBorderDotColor(const glm::vec4& color);

        /// @brief Sets the spacing between dot centers, in pixels, for the item border's DottedPattern shader feature.
        void SetItemBorderDotDistance(const f32 distance);

        /// @brief Sets the direction, in degrees, along which the item border's dot radii/animation transfer.
        void SetItemBorderDotSizeTransferDegree(const f32 degree);

        /// @brief Sets the per-position dot radius curve, in pixels, for the item border's DottedPattern shader feature. Enables the feature when non-empty.
        void SetItemBorderDotRadiusTransfer(std::span<f32> radiusTransfer);

        /// @brief Sets how far each item border dot's opacity reaches from its center before fading to transparent.
        void SetItemBorderDotTransparencyTransfer(const f32 transparencyTransfer);

        /// @brief Sets the speed at which the item border's dotted pattern animates.
        void SetItemBorderDotAnimationSpeed(const f32 animationSpeed);

        /// @brief Sets the item border's width.
        void SetItemBorderWidth(const f32 borderWidth);

        /// @brief Sets sections of the item border to skip drawing; see Rectangle::SetBorderGaps.
        void SetItemBorderGaps(std::span<BorderGap> gaps);



    private:
        /// @brief Emits the hover/click signals and works out which flex slot the cursor sits in.
        void interact();

        /// @brief Re-docks the bar against its edge and places the items in their slots.
        void updateLayout();

        /// @brief Puts every item in the center of its slot. Slot geometry follows the bar, items keep their own size.
        void placeItems();

        void drawItems();
        void drawSeparatorLines();

        /// @brief Returns the center of the bar along the edge it is docked to, honouring the alignment.
        f32 alignAlongEdge(const f32 edgeLength, const f32 barLength) const;

        /// @brief Returns where the bar sits once its docking edge, margin and alignment are applied.
        glm::vec3 calculateDockedPosition() const;

        /// @brief Whether the bar runs along the X axis, i.e. is docked to the top or the bottom edge.
        bool isHorizontal() const;

        /// @brief Returns the size of one flex slot. Zero on both axes while the flex is empty.
        glm::vec2 slotSize() const;

        /// @brief Returns the center of the given flex slot.
        glm::vec3 slotPosition(const uSize index) const;

        /// @brief The state the item in the given slot draws in, given what the cursor is doing.
        ColorState itemState(const uSize index) const;

        /// @brief Makes the given state the one the items draw in, and pushes the matching colors.
        void setItemColors(const ColorState state);

        /// @brief Hands the colors to the element, unless the state they came from was never set.
        void pushElementColors(Rectangle* element, std::vector<glm::vec4>* colors);

        void setSeparatorLinesColors();
        void setItemBackgroundColors();
        void setItemBorderColors();
        void setItemBackgroundCornerRadii(const glm::vec4& radii);
        void setItemBorderCornerRadii(const glm::vec4& radii);

    //Elements
        std::unique_ptr<Rectangle>
            _separatorLine { nullptr },
            _itemBackground { nullptr },
            _itemBorder { nullptr };
        std::list<std::unique_ptr<IWidget>> _itemFlex {};

    //Design
        f32 _margin { 0.0f }; //margin from window edge
        DockingEdge _dockingEdge { DockingEdge::Top };
        EdgeAlignment _edgeAlignment { EdgeAlignment::Center };
        std::vector<glm::vec4>
            _separatorLinesColorsEnabled {},
            _separatorLinesColorsDisabled {},
            _itemBackgroundColorsEnabled {},
            _itemBackgroundColorsDisabled {},
            _itemBackgroundColorsHover {},
            _itemBackgroundColorsClick {},
            _itemBorderColorsEnabled {},
            _itemBorderColorsDisabled {},
            _itemBorderColorsHover {},
            _itemBorderColorsClick {};
        std::vector<f32>
            _itemBackgroundDotRadiusTransfer {},
            _itemBackgroundFogDensity {},
            _itemBorderDotRadiusTransfer {};

        f32 _separatorLinesThickness { 1.0f };

    //Logic
        ColorState
            _separatorLinesColorState { ColorState::Enabled },
            _itemBackgroundColorState { ColorState::Enabled },
            _itemBorderColorState { ColorState::Enabled };
        bool _useSeparatorLines { false };

        /// @brief Stands in for "the cursor is on no slot at all" in _hoveredItemIndex.
        static constexpr uSize _kNoItem { ~uSize(0) };
        uSize _hoveredItemIndex { _kNoItem };

    };
}