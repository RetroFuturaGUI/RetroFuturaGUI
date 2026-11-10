#pragma once
#include "IContainer.hpp"
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

    class ExtendedComboBox final : public IDropDown, public IContainer
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
        void SetDropDownPosition(const DropDownPosition position) override;

        /// @brief Builds a widget of type T as the last item and owns it. The box is its parent, so an open list counts the row
        ///        as its own instead of blocking it like a widget beneath, and passes its projection and window on to it
        /// @param args Constructor arguments after the parent window, such as an SvgImage's path
        /// @return The item, so the caller can carry on configuring it, or nullptr, building nothing, when the name is taken
        /// @warning Call from the GUI thread only: a widget creates OpenGL objects, and those must stay on the context's thread
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

            if constexpr (std::is_same_v<T, Label> || std::is_same_v<T, Button> || std::is_same_v<T, TextBox>)
            {
                if(!_fontFamily.empty())
                    itemPointer->GetText().SetFontFamily(_fontFamily, _fontSize, FontSizeUnit::Pixels, _slant, _fontWeight);
            }
            else if constexpr (std::is_same_v<T, ComboBox> || std::is_same_v<T, ExtendedComboBox>)
            {
                if(!_fontFamily.empty())
                    itemPointer->SetFontFamily(_fontFamily, _fontSize, FontSizeUnit::Pixels, _slant, _fontWeight);
            }

            _itemCount = _itemWidgets.size();
            updateDropDownGeometry();
            return itemPointer;
        }

        /// @brief Returns the text shown in the closed box, which stands in for the selected item: its string, colors per state,
        ///        fills and effects. Its font, alignment and padding follow the box's (SetFontFamily, SetTextAlignment, SetTextPadding).
        WidgetText& GetPreview();

        /// @brief Returns the text shown in the closed box; see the other overload.
        const WidgetText& GetPreview() const;

        bool IsPointInsideOpenDropDown(const glm::vec2& point) const override;

        /// @brief Removes the item at the given index. The row goes, and the widget is destroyed, at the start of the next Interact, so the item's own slot may remove it
        void RemoveItem(const uSize index) override;
        void SetFontFamily(std::string_view fontFamily, const f32 fontSize, const FontSizeUnit sizeUnit, const PlatformBridge::Fonts::Slant slant, const PlatformBridge::Fonts::Weight fontWeight) override;
        void SetTextAlignment(const TextAlignment alignment) override;
        void SetTextPadding(const f32 padding) override;
        const std::list<std::unique_ptr<IWidget>>& GetItems() const;

    protected:
        void applySize(const glm::vec3& size) override;

        void applyPosition(const glm::vec3& position) override;

    private:
        /// @brief Sweeps the removed items, follows the pointer for the box and its list, then passes the input on to the rows while the list is open
        void interact() override;

        /// @brief Passes the frame's update on to every widget, hidden ones included
        void update() override;

        /// @brief Hover, clicks and picking a row for the box and its open list
        void followPointer();

        /// @brief Makes the given item the selected one and emits what changed
        void selectItem(const uSize index) override;

        void drawDropDown() override;
        void setColors(const ColorState state) override;
        void updateItemGeometry();
        void updatePreviewTextGeometry();

        /// @brief Returns what is left of a box of the given width, the closed box's or the open list's, once the thickest of the two borders is taken off
        glm::vec2 contentSize(const f32 width) const;

        /// @brief Returns whether a world space point falls on a drop down one of the visible rows has open
        bool isPointInsideItemDropDown(const glm::vec2& point) const;

        void placeItemWidget(IWidget* item, const glm::vec3& center) const;

    //Elements
        std::list<std::unique_ptr<IWidget>> _itemWidgets {}; //a removed item leaves an empty row until the next Interact
        std::vector<std::unique_ptr<IWidget>> _removedWidgets {}; //destroyed at the start of the next Interact

        /// @brief Whether an item already has the given name.
        bool isItemNameTaken(std::string_view name) const;

        /// @brief Drops the rows of removed items, and the widgets with them
        void performPendingRemoval();

    //Design
        WidgetText _preview { &_projection };

    };
}