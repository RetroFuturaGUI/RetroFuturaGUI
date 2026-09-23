#pragma once
#include <concepts>
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

        /// @brief Takes ownership of a widget and appends it as the last item.
        /// @return The item, so the caller can carry on configuring it.
        template<ExtendedComboBoxWidgets T> T* AddItem(std::unique_ptr<T> item)
        {
            if(!item)
                return nullptr;

            /* The widget types delete their copy and move constructors, so an item can only arrive
               as a pointer that is already owning. The raw pointer is kept before the unique_ptr is
               handed over, because the list stores it as an IWidget and T is what the caller wants back. */
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

        /// @brief Sets the preview text color for the given color state.
        void SetPreviewTextColor(const glm::vec4& color, const ColorState state);

        /// @brief Returns the preview text color configured for the given color state.
        glm::vec4 GetPreviewTextColor(const ColorState state) const;

        bool IsPointInsideOpenDropDown(const glm::vec2& point) const override;
        void RemoveItem(const uSize index) override;
        void SetFontFamily(std::string_view fontFamily, const f32 fontSize, const PlatformBridge::Fonts::Slant slant, const PlatformBridge::Fonts::Weight fontWeight) override;
        void SetTextAlignment(const TextAlignment alignment) override;
        void SetTextPadding(const f32 padding) override;

    private:
        void interact();

        /// @brief Makes the given item the selected one and emits what changed.
        void selectItem(const uSize index) override;

        
        void setColors(const ColorState state) override;
        void setPreviewTextColors();
        void updateItemGeometry();
        void updatePreviewTextGeometry() const;

        /// @brief Returns what is left of the box once the thickest of the two borders is taken off.
        glm::vec2 contentSize() const;

        /// @brief Returns whether a world space point falls on a drop down one of the visible rows has open.
        bool isPointInsideItemDropDown(const glm::vec2& point) const;

        void placeItemWidget(IWidget* item, const glm::vec3& center) const;

    //Elements
        std::list<std::unique_ptr<IWidget>> _itemWidgets {};
        inline static const std::string _emptyText { "" };

    //Design
        glm::vec4
            _previewTextColorEnabled { 1.0f, 1.0f, 1.0f, 1.0f },
            _previewTextColorDisabled { 0.5f, 0.5f, 0.5f, 1.0f },
            _previewTextColorHover { 1.0f, 1.0f, 1.0f, 1.0f },
            _previewTextColorClicked { 1.0f, 1.0f, 1.0f, 1.0f };

    //Logic
        ColorState _previewTextColorState { ColorState::Enabled };
    };
}