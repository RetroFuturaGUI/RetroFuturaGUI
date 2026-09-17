#pragma once
#include <string>

namespace RetroFuturaGUI
{
    class IHierarchyNode
    {
    public:
        virtual ~IHierarchyNode() = default;

        /// @brief Returns the node's own name, unqualified by any parent.
        virtual const std::string& GetName() const = 0;

        /// @brief Returns the node this one hangs under, or nullptr when it is a root.
        virtual const IHierarchyNode* GetParent() const = 0;

        /// @brief Returns the name composed from the root down, e.g. "MainWindow/ButtonSlider/Slider".
        std::string GetPath() const;
    };
}
