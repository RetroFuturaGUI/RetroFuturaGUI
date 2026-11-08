#pragma once
#include "IncludeHelper.hpp"
#include <memory>
#include <string>
#include <string_view>
#include <list>
#include <vector>
#include "Text.hpp"
#include "IDropDown.hpp"
#include "ItemTextStyle.hpp"

namespace RetroFuturaGUI
{
    class ComboBox final : public IDropDown
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
        void AddItem(std::string_view name);
        const std::string& GetSelectedText() const;
        void RemoveItem(const uSize index) override;
        void SetFontFamily(std::string_view fontFamily, const f32 fontSize, const FontSizeUnit sizeUnit, const PlatformBridge::Fonts::Slant slant, const PlatformBridge::Fonts::Weight fontWeight) override;
        void SetTextAlignment(const TextAlignment alignment) override;
        void SetTextPadding(const f32 padding) override;
        void SetDropDownPosition(const DropDownPosition position) override;

        /// @brief Shows a caption in the closed box instead of the selected item, so the box reads like a menu's title: "File".
        ///        Picking an item still selects it and fires OnItemClicked, which a menu acts on. Empty shows the selection again.
        void SetCaption(std::string_view caption);

        /// @brief Enables or disables an item. A disabled one is drawn in the disabled item color and can't be picked.
        void SetItemEnabled(const uSize index, const bool enable);

        bool IsItemEnabled(const uSize index) const;

        /// @brief Returns the item texts' colors: Enabled, Disabled for disabled items, Hover for the hovered row and Clicked for
        ///        the picked one. The caption shows the Enabled colors.
        ItemTextStyle& GetItemText();

        /// @brief Returns the item texts' colors; see the other overload.
        const ItemTextStyle& GetItemText() const;

        /// @brief Returns the ComboBox's border
        Border& GetBorder();

        /// @brief Returns the ComboBox's border
        const Border& GetBorder() const;
        
    protected:
        void applySize(const glm::vec3& size) override;

        void applyPosition(const glm::vec3& position) override;

    private:
        void interact() override;

        /// @brief Makes the given item the selected one and emits what changed.
        void selectItem(const uSize index) override;

        void drawDropDown() override;
        void setColors(const ColorState state) override;
        void updateItemGeometry();

        /// @brief Colors every item by its state: disabled, picked, hovered or enabled.
        void updateItemColors();

        /// @brief The state an item shows: Disabled, Clicked for the picked row while the list fades out, Hover or Enabled.
        ColorState itemState(const uSize index) const;

        /// @brief Places a text in a box of the given width: the closed box's, or the open list's.
        void placeItemText(Text& item, const glm::vec3& center, const f32 width) const;

    //Elements
        std::list<std::unique_ptr<Text>> _itemTexts {};
        inline static const std::string _invalidIndex { "" };

    //Design
        ItemTextStyle _itemText {};

    //Logic
        std::vector<bool> _itemEnabledFlags {}; //one per item
        uSize _pickedItemIndex { 0 };
        bool
            _hasCaption { false },
            _hasPickedItem { false }; //the last pick, shown Clicked while the list fades out
    };
}