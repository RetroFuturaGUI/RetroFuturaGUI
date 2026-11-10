#pragma once
#include <concepts>

namespace RetroFuturaGUI
{
    class IWidget;

    /// @brief Implemented by the widgets that build, own and lay out other widgets: Lasagna, Environment, ScrollView, Prefab,
    ///        MenuBar, RadioButtonGroup, ExtendedComboBox, Tab and WindowTab. A new container implements it to be a scene's root
    class IContainer
    {
    public:
        virtual ~IContainer() = default;
    };

    /// @brief A widget that can be a scene's root, or an item of a Tab: any container
    template<typename T>
    concept RootContainer = std::derived_from<T, IWidget> && std::derived_from<T, IContainer>;
}