#pragma once
#include "IWidget.hpp"
#include <string_view>
#include "IncludeHelper.hpp"
#include "Lasagna.hpp"
#include <unordered_map>
#include <vector>
#include <string>
#include <memory>
#include <utility>
#include <print>

namespace RetroFuturaGUI
{
    /// @brief Where a child widget sits in the prefab's Lasagna. Aggregate so call sites can use designated initializers (example: AttachWidget<Button>("Inc", { ._Row = 0, ._Column = 2 }))
    struct CellPlacement
    {
        u32
            _Row { 0 },
            _Column { 0 },
            _Layer { 0 };
        SizingMode _SizingMode { SizingMode::FILL };
        u32
            _RowSpan { 1 },
            _ColSpan { 1 },
            _LayerSpan { 1 };
    };

    struct ChildWidget
    {
        std::unique_ptr<IWidget> _Widget;
        CellPlacement _Placement;
    };

    class Prefab final : public IWidget
    {
    public:
        /// @brief Constructs a prefab and the Lasagna its children sit in.
        /// @param axisDefinition The grid to build. Falls back to a single full-size cell if invalid
        Prefab(std::string_view name, Projection* projection, IHierarchyNode* parentWidget, const WidgetTypeID parentWidgetTypeID, GLFWwindow* parentWindow, const AxisDefinition& axisDefinition = SingleCellAxis);
        Prefab() = delete;
        Prefab(const Prefab&) = delete;
        Prefab(Prefab&&) = delete;
        ~Prefab() override = default;
        auto operator =(const Prefab&) = delete;
        auto operator =(Prefab&&) = delete;
        void Draw() override;
        void SetSize(const glm::vec3& size) override;
        void SetPosition(const glm::vec3& position) override;
        void SetRotation(const glm::vec3& rotation) override;

        void ShowChildWidget(std::string_view name, const bool show);
        bool IsChildWidgetShown(std::string_view name) const;
        template<WidgetTypes T> T* GetChildWidget(std::string_view name) const
        {
            RetroFuturaGUI::LasagnaCell* cell = findCell(name);

            if(cell == nullptr)
                return nullptr;

            return dynamic_cast<T*>(cell->_Widget);
        }

        /// @brief Constructs a widget of type T, takes ownership of it and places it in the prefab's Lasagna
        /// @param args Extra constructor arguments
        /// @return The attached widget, or nullptr if the prefab has no grid yet, the name is already taken, or the target cell is out of range or occupied
        template<WidgetTypes T, typename... Args>
        T* AttachWidget(std::string_view name, const CellPlacement& placement, Args&&... args)
        {
            if(_lasagna == nullptr)
            {
                std::println("ERROR::PREFAB::NO_LAYOUT::{}", name);
                return nullptr;
            }

            const u32 key { Fnv1aHash(name) };

            if(_widgetLookup.contains(key))
            {
                std::println("ERROR::PREFAB::CHILD_WIDGET_NAME_TAKEN::{}", name);
                return nullptr;
            }

            //Concrete widgets take const std::string&, not a view, so the name has to be materialised here
            const std::string widgetName { name };
            std::unique_ptr<T> widget { std::make_unique<T>(widgetName, &_projection, this, _widgetTypeID, _parentWindow, std::forward<Args>(args)...) };
            T* _attached { widget.get() };

            if(!_lasagna->AttachWidget(placement._Row, placement._Column, placement._Layer, _attached,
                                       placement._SizingMode, placement._RowSpan, placement._ColSpan, placement._LayerSpan))
            {   //Cell out of range or already taken
                std::println("ERROR::PREFAB::CELL_UNAVAILABLE::{}::[{},{},{}]", name, placement._Row, placement._Column, placement._Layer);
                return nullptr;
            }

            _widgetLookup.emplace(key, _children.size());
            _children.push_back(ChildWidget { ._Widget = std::move(widget), ._Placement = placement });
            return _attached;
        }

    private:

        /// @brief Looks a child up by name, verifying the stored name so a hash collision cannot silently return the wrong one.
        const ChildWidget* findChild(std::string_view name) const;

        /// @brief The child's current cell. Fetched on demand rather than stored, so it stays correct across grid rebuilds.
        LasagnaCell* findCell(std::string_view name) const;

        std::vector<ChildWidget> _children {};
        std::unordered_map<u32, uSize> _widgetLookup {};
        std::unique_ptr<Lasagna> _lasagna { nullptr };
    };
}