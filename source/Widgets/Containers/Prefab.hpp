#pragma once
#include "IWidget.hpp"
#include "IContainer.hpp"
#include <string_view>
#include "IncludeHelper.hpp"
#include "Lasagna.hpp"
#include <memory>
#include <utility>
#include <print>

namespace RetroFuturaGUI
{
    /// @brief A group of widgets laid out in a Lasagna of its own, which builds, owns and parents them.
    class Prefab final : public IWidget, public IContainer
    {
    public:
        /// @brief Constructs a prefab and the Lasagna its children sit in.
        /// @param axisDefinition The rows, columns and layers of the prefab's Lasagna. Falls back to a single full-size cell if invalid
        Prefab(std::string_view name, Projection* projection, IHierarchyNode* parentWidget, const WidgetTypeID parentWidgetTypeID, GLFWwindow* parentWindow, const AxisDefinition& axisDefinition = SingleCellAxis);
        Prefab() = delete;
        Prefab(const Prefab&) = delete;
        Prefab(Prefab&&) = delete;
        ~Prefab() override = default;
        auto operator =(const Prefab&) = delete;
        auto operator =(Prefab&&) = delete;
        void Draw() override;
        /// @brief Replaces the rows, columns and layers of the prefab's Lasagna, keeping every child in its cell and span. A child
        ///        reaching past the new last track is removed, destroyed at the start of the next Interact. An invalid definition
        ///        changes nothing
        /// @return The names of the children removed
        std::vector<std::string> SetAxisDefinition(const AxisDefinition& axisDefinition);

        /// @brief Returns the rows, columns and layers of the prefab's Lasagna
        const AxisDefinition& GetAxisDefinition() const;

        /// @brief Replaces the column tracks, as SetAxisDefinition does with the rows and layers kept
        /// @return The names of the children removed.
        std::vector<std::string> SetColumnDefinition(std::vector<f32> columnDefinition);

        /// @brief Replaces the row tracks, as SetAxisDefinition does with the columns and layers kept
        /// @return The names of the children removed.
        std::vector<std::string> SetRowDefinition(std::vector<f32> rowDefinition);

        /// @brief Replaces the layer tracks, as SetAxisDefinition does with the rows and columns kept
        /// @return The names of the children removed
        std::vector<std::string> SetLayerDefinition(std::vector<f32> layerDefinition);

        void ShowChildWidget(std::string_view name, const bool show);
        bool IsChildWidgetShown(std::string_view name) const;

        /// @brief Returns the child with the given name, or nullptr when there is none or it isn't of type T.
        template<WidgetTypes T> T* GetChildWidget(std::string_view name) const
        {
            if(!_lasagna)
                return nullptr;

            return _lasagna->GetWidget<T>(name);
        }

        /// @brief Builds a widget of type T in the prefab's Lasagna, which owns it and is its parent.
        /// @param args Extra constructor arguments
        /// @return The attached widget, or nullptr if the prefab has no Lasagna, the name is already taken, or the target cell is out of range or occupied
        template<WidgetTypes T, typename... Args>
        T* AddWidget(std::string_view name, const CellPlacement& placement, Args&&... args)
        {
            if(!_lasagna)
            {
                std::println("ERROR::PREFAB::NO_LAYOUT::{}", name);
                return nullptr;
            }

            return _lasagna->AddWidget<T>(name, placement, std::forward<Args>(args)...);
        }

        /// @brief Removes the child with the given name. It is destroyed at the start of the next Interact, so its own slot may remove it.
        /// @return false when no child has that name.
        bool RemoveChildWidget(std::string_view name);

    protected:
        void applySize(const glm::vec3& size) override;

        void applyPosition(const glm::vec3& position) override;

    private:
        void interact() override;

        /// @brief Passes the frame's update on to every widget, hidden ones included.
        void update() override;

        std::unique_ptr<Lasagna> _lasagna { nullptr };
    };
}