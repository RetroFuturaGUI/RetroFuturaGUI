#pragma once
#include "IncludeHelper.hpp"
#include <memory>
#include <string>
#include <string_view>
#include <list>
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

    private:
        void interact();

        /// @brief Makes the given item the selected one and emits what changed.
        void selectItem(const uSize index) override;

        
        void setColors(const ColorState state) override;
        void updateItemGeometry();
        
        void placeItemText(Text& item, const glm::vec3& center) const;

    //Elements
        std::list<std::unique_ptr<Text>> _itemTexts {};
        inline static const std::string _invalidIndex { "" };

    };
}