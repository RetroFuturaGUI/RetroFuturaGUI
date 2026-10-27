#pragma once
#include "IncludeHelper.hpp"
#include <memory>
#include <string>
#include <string_view>
#include <list>
#include <vector>
#include "Text.hpp"
#include "IDropDown.hpp"

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
        void SetSize(const glm::vec3& size) override;
        void SetPosition(const glm::vec3& position) override;
        void SetRotation(const glm::vec3& rotation) override;
        void AddItem(std::string_view name);
        const std::string& GetSelectedText() const;
        void RemoveItem(const uSize index) override;
        void SetFontFamily(std::string_view fontFamily, const f32 fontSize, const PlatformBridge::Fonts::Slant slant, const PlatformBridge::Fonts::Weight fontWeight) override;
        void SetTextAlignment(const TextAlignment alignment) override;
        void SetTextPadding(const f32 padding) override;
        void SetDropDownPosition(const DropDownPosition position) override;

        /// @brief Shows a caption in the closed box instead of the selected item, so the box reads like a menu's title: "File".
        ///        Picking an item still selects it and fires OnItemClicked, which a menu acts on. Empty shows the selection again.
        void SetCaption(std::string_view caption);

        /// @brief Enables or disables an item. A disabled one is drawn in the disabled item color and can't be picked.
        void SetItemEnabled(const uSize index, const bool enable);

        bool IsItemEnabled(const uSize index) const;

        /// @brief Sets the item texts' colors: ColorState::Disabled sets the disabled items' colors, every other state the enabled items'.
        void SetItemTextColors(std::span<const glm::vec4> colors, const ColorState state);

    private:
        void interact();

        /// @brief Makes the given item the selected one and emits what changed.
        void selectItem(const uSize index) override;

        void drawDropDown() override;
        void setColors(const ColorState state) override;
        void updateItemGeometry();

        /// @brief Colors every item by whether it is enabled.
        void updateItemColors();

        /// @brief Places a text in a box of the given width: the closed box's, or the open list's.
        void placeItemText(Text& item, const glm::vec3& center, const f32 width) const;

    //Elements
        std::list<std::unique_ptr<Text>> _itemTexts {};
        inline static const std::string _invalidIndex { "" };

    //Design
        std::vector<glm::vec4>
            _itemTextColorsEnabled { glm::vec4(1.0f, 1.0f, 1.0f, 1.0f) },
            _itemTextColorsDisabled { glm::vec4(0.5f, 0.5f, 0.5f, 1.0f) };

    //Logic
        std::vector<bool> _itemEnabledFlags {}; //one per item
        bool _hasCaption { false };
    };
}