#pragma once
#include "IBackground.hpp"
#include "IBorder.hpp"
#include "IncludeHelper.hpp"
#include "IWidget.hpp"
#include "IClickable.hpp"
#include <memory>
#include <string>
#include <string_view>
#include <list>
#include "SvgTexture.hpp"
#include "Text.hpp"
#include "Rectangle.hpp"

namespace RetroFuturaGUI
{
    class ComboBox final : public IWidget, public IClickable, public IBackground, public IBorder
    {
    public:
        ComboBox(std::string_view name, Projection* projection, IHierarchyNode* parentWidget, const WidgetTypeID parentWidgetTypeID, GLFWwindow* parentWindow);
        ComboBox() = delete;
        ComboBox(const ComboBox&) = delete;
        ComboBox(ComboBox&&) = delete;
        ~ComboBox() = default;
        auto operator =(const ComboBox&) = delete;
        auto operator =(ComboBox&&) = delete;

        void Draw() override;
        void SetSize(const glm::vec3& size) override;
        void SetPosition(const glm::vec3& position) override;
        void SetRotation(const glm::vec3& rotation) override;
        void AddItem(std::string_view name);
        void RemoveItem(const uSize index);
        void Connect_OnItemClicked(const typename Signal<>::Slot& slot, const bool async);
        void Connect_OnIndexChanged(const typename Signal<>::Slot& slot, const bool async);
        void Disconnect_OnItemClicked(const typename Signal<>::Slot& slot);
        void Disconnect_OnIndexChanged(const typename Signal<>::Slot& slot);
        uSize GetItemCount() const;
        uSize GetSelectedIndex() const;
        void SetIndex(const uSize index);
        const std::string& GetSelectedText() const;
        void SetArrowDownColors(std::span<glm::vec4> colors, const ColorState state);
        void SetDropDownBackgroundColors(std::span<glm::vec4> colors, const ColorState state);
        void SetDropDownBorderColors(std::span<glm::vec4> colors, const ColorState state);
        void SetDropDownCornerRadii(const glm::vec4& radii);
        void SetFontFamily(std::string_view fontFamily, const f32 fontSize, const PlatformBridge::Fonts::Slant slant, const PlatformBridge::Fonts::Weight fontWeight);
        void SetTextAlignment(const TextAlignment alignment);
        void SetTextPadding(const f32 padding);

    private:
        void interact();

        /// @brief Returns whether a world space point falls on the open drop down panel.
        bool isPointInsideDropDown(const glm::vec2& point) const;

        /// @brief Finds the visible drop down row under a world space point.
        bool hoveredItemIndex(const glm::vec2& point, uSize& outIndex) const;

        /// @brief Makes the given item the selected one and emits what changed.
        void selectItem(const uSize index);

        void setArrowDownColors();
        void setDropDownBackgroundColors();
        void setDropDownBorderColors();
        void setColors(const ColorState state);
        void updateArrowDownGeometry();
        void updateDropDownGeometry();
        void updateItemGeometry();
        void placeItemText(Text& item, const glm::vec3& center) const;
        uSize visibleItemCount() const;
        glm::vec3 itemRowCenter(const uSize visibleIndex) const;
        glm::vec2 rotateLocalOffset(const glm::vec2& localOffset) const;

    //Elements
        std::unique_ptr<Rectangle>
            _dropDownBackground { nullptr },
            _dropDownItemHighlight { nullptr },
            _dropDownBorder { nullptr },
            _dropDownItemBorder { nullptr };
        std::list<std::unique_ptr<Text>> _itemTexts {};
        uSize _currentIndex { 0 };
        inline static const std::string _invalidIndex { "" };
        std::shared_ptr<SvgTexture> _arrowDown { nullptr };
        uSize
            _maxItemsToDraw { 8 },
            _nthItemHovered { 0 };
        static const constexpr f32
            _zOffset { 0.05f },
            _zStep { 0.01f },
            _arrowPadding { 15.0f };
        ColorState
            _arrowDownColorState { ColorState::Enabled },
            _dropDownBackgroundColorSate { ColorState::Enabled },
            _dropDownBorderColorSate { ColorState::Enabled };
        bool
            _showDropDown { false },
            _dropDownItemHovered { false };

    //Design
         std::vector<glm::vec4>
            _arrowDownColorsEnabled { { 0.7f, 0.7f, 0.7f, 1.0f } },
            _arrowDownColorsDisabled { { 0.3f, 0.3f, 0.3f, 1.0f } },
            _arrowDownColorsHover { { 0.85f, 0.85f, 0.85f, 1.0f } },
            _arrowDownColorsClicked { { 1.0f, 1.0f, 1.0f, 1.0f } },
            _borderColorsEnabled { { 0.7f, 0.7f, 0.7f, 1.0f } },
            _borderColorsDisabled { { 0.3f, 0.3f, 0.3f, 1.0f } },
            _borderColorsHover { { 0.85f, 0.85f, 0.85f, 1.0f } },
            _borderColorsClicked { { 1.0f, 1.0f, 1.0f, 1.0f } },
            _backgroundColorsEnabled { { 0.25f, 0.25f, 0.8f, 1.0f } },
            _backgroundColorsDisabled { { 0.1f, 0.1f, 0.45f, 1.0f } },
            _backgroundColorsHover { { 0.3f, 0.3f, 0.9f, 1.0f } },
            _backgroundColorsClicked { { 0.4f, 0.4f, 1.0f, 1.0f } },
            _dropDownBorderColorsEnabled { { 0.7f, 0.7f, 0.7f, 1.0f } },
            _dropDownBorderColorsDisabled { { 0.3f, 0.3f, 0.3f, 1.0f } },
            _dropDownBorderColorsHover { { 0.85f, 0.85f, 0.85f, 1.0f } },
            _dropDownBorderColorsClicked { { 1.0f, 1.0f, 1.0f, 1.0f } },
            _dropDownBackgroundColorsEnabled { { 0.25f, 0.25f, 0.8f, 1.0f } },
            _dropDownBackgroundColorsDisabled { { 0.1f, 0.1f, 0.45f, 1.0f } },
            _dropDownBackgroundColorsHover { { 0.3f, 0.3f, 0.9f, 1.0f } },
            _dropDownBackgroundColorsClicked { { 0.4f, 0.4f, 1.0f, 1.0f } },
            _highLightColor { { 0.3f, 0.9f, 2.0f, 1.0f } };
        SvgPathFill _arrowDownPathFill
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
        TextAlignment _textAlignment { TextAlignment::Left };
        std::string _fontFamily {};
        f32 
            _fontSize { 11.0f },
            _textPadding { 4.0f };
        PlatformBridge::Fonts::Slant _slant { PlatformBridge::Fonts::Slant::Roman };
        PlatformBridge::Fonts::Weight _fontWeight { PlatformBridge::Fonts::Weight::Normal };

    //Signals
        Signal<>
            _onItemClicked,
            _onItemClickedAsync,
            _onIndexChanged,
            _onIndexChangedAsync;
    };
}