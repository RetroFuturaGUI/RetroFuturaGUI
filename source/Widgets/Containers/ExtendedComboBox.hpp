#pragma once
#include <concepts>
#include <list>
#include <memory>
#include <print>
#include <string>
#include <string_view>
#include <utility>
#include <vector>
#include "ComboBox.hpp"
#include "IDropDown.hpp"
#include "IncludeHelper.hpp"
#include "Label.hpp"
#include "Button.hpp"
#include "TextBox.hpp"
#include "SvgImage.hpp"
#include "CheckBox.hpp"
#include "Slider.hpp"
#include "ProgressBar.hpp"
#include "Prefab.hpp"
#include "ComboBox.hpp"

namespace RetroFuturaGUI 
{
    class ExtendedComboBox;

    template<typename T>
    concept ExtendedComboBoxWidgets =
        std::same_as<T, Label>  || std::same_as<T, Button> || std::same_as<T, TextBox>
        || std::same_as<T, Image> || std::same_as<T, SvgImage> || std::same_as<T, CheckBox>
        || std::same_as<T, Slider> || std::same_as<T, ProgressBar> || std::same_as<T, Prefab>
        || std::same_as<T, ComboBox> || std::same_as<T, ExtendedComboBox>;

    class ExtendedComboBox final : public IDropDown
    {
    public:
        ExtendedComboBox(std::string_view name, Projection* projection, IHierarchyNode* parentWidget, const WidgetTypeID parentWidgetTypeID, GLFWwindow* parentWindow);
        ExtendedComboBox() = delete;
        ExtendedComboBox(const ExtendedComboBox&) = delete;
        ExtendedComboBox(ExtendedComboBox&&) = delete;
        ~ExtendedComboBox() = default;
        auto operator =(const ExtendedComboBox&) = delete;
        auto operator =(ExtendedComboBox&&) = delete;

        void Draw() override;
        void SetSize(const glm::vec3& size) override;
        void SetPosition(const glm::vec3& position) override;
        void SetRotation(const glm::vec3& rotation) override;
        void SetDropDownPosition(const DropDownPosition position) override;

        /// @brief Builds a widget of type T as the last item and owns it. The box is its parent, so an open list counts the row
        ///        as its own instead of blocking it like a widget beneath, and passes its projection and window on to it.
        /// @param args Constructor arguments after the parent window, such as an SvgImage's path.
        /// @return The item, so the caller can carry on configuring it, or nullptr, building nothing, when the name is taken.
        /// @warning Call from the GUI thread only: a widget creates OpenGL objects, and those must stay on the context's thread.
        template<ExtendedComboBoxWidgets T, typename... Args>
        T* AddItem(std::string_view name, Args&&... args)
        {
            if(isItemNameTaken(name))
            {
                std::println("ERROR::EXTENDEDCOMBOBOX::ITEM_NAME_TAKEN::{}::{}", _name, name);
                return nullptr;
            }

            //Most widget constructors take the name as const std::string&, so it is materialised here
            const std::string itemName { name };
            std::unique_ptr<T> item { std::make_unique<T>(itemName, &_projection, this, _widgetTypeID, _parentWindow, std::forward<Args>(args)...) };
            T* itemPointer { item.get() };
            _itemWidgets.push_back(std::move(item));

            if constexpr (std::is_same_v<T, Label> || std::is_same_v<T, Button> || std::is_same_v<T, TextBox>
                          || std::is_same_v<T, ComboBox> || std::is_same_v<T, ExtendedComboBox>)
            {
                if(!_fontFamily.empty())
                    itemPointer->SetFontFamily(_fontFamily, _fontSize, _slant, _fontWeight);
            }

            _itemCount = _itemWidgets.size();
            updateDropDownGeometry();
            return itemPointer;
        }

        /// @brief Sets the text shown in the closed box, which stands in for the selected item.
        void SetPreviewText(std::string_view text);

        /// @brief Returns the text shown in the closed box.
        const std::string& GetPreviewText() const;

        /// @brief Sets the preview text colors for the given color state.
        void SetPreviewTextColors(std::span<const glm::vec4> colors, const ColorState state);

        /// @brief Returns the preview text colors configured for the given color state.
        const std::vector<glm::vec4>& GetPreviewTextColors(const ColorState state) const;

        bool IsPointInsideOpenDropDown(const glm::vec2& point) const override;

        /// @brief Removes the item at the given index. The row goes, and the widget is destroyed, at the start of the next Draw,
        ///        so the item's own slot may remove it.
        void RemoveItem(const uSize index) override;
        void SetFontFamily(std::string_view fontFamily, const f32 fontSize, const PlatformBridge::Fonts::Slant slant, const PlatformBridge::Fonts::Weight fontWeight) override;
        void SetTextAlignment(const TextAlignment alignment) override;
        void SetTextPadding(const f32 padding) override;

    private:
        void interact();

        /// @brief Makes the given item the selected one and emits what changed.
        void selectItem(const uSize index) override;

        void drawDropDown() override;
        void setColors(const ColorState state) override;
        void setPreviewTextColors();
        void updateItemGeometry();
        void updatePreviewTextGeometry() const;

        /// @brief Returns what is left of a box of the given width, the closed box's or the open list's, once the thickest of the two borders is taken off.
        glm::vec2 contentSize(const f32 width) const;

        /// @brief Returns whether a world space point falls on a drop down one of the visible rows has open.
        bool isPointInsideItemDropDown(const glm::vec2& point) const;

        void placeItemWidget(IWidget* item, const glm::vec3& center) const;

    //Elements
        std::list<std::unique_ptr<IWidget>> _itemWidgets {}; //a removed item leaves an empty row until the next Draw
        std::vector<std::unique_ptr<IWidget>> _removedWidgets {}; //destroyed at the start of the next Draw

        /// @brief Whether an item already has the given name.
        bool isItemNameTaken(std::string_view name) const;

        /// @brief Drops the rows of removed items, and the widgets with them.
        void performPendingRemoval();
        inline static const std::string _emptyText { "" };

    //Design
        std::vector<glm::vec4>
            _previewTextColorsEnabled { glm::vec4(1.0f, 1.0f, 1.0f, 1.0f) },
            _previewTextColorsDisabled { glm::vec4(0.5f, 0.5f, 0.5f, 1.0f) },
            _previewTextColorsHover { glm::vec4(1.0f, 1.0f, 1.0f, 1.0f) },
            _previewTextColorsClicked { glm::vec4(1.0f, 1.0f, 1.0f, 1.0f) };

    //Logic
        ColorState _previewTextColorState { ColorState::Enabled };
    };
}