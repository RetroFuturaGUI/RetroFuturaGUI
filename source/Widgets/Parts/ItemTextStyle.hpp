#pragma once
#include "IncludeHelper.hpp"
#include "StyledPart.hpp"
#include <span>
#include <vector>

namespace RetroFuturaGUI
{
    /// @brief The colors of a list's item texts per state: Enabled, Disabled for disabled items, Hover for the hovered row and
    ///        Clicked for the picked one. The widget owns it and hands it out through GetItemText(), so it is styled as
    ///        comboBox.GetItemText().SetColors(...)
    class ItemTextStyle final : public IStyledPart
    {
    public:
        ItemTextStyle();

        /// @brief Sets the colors for the given state, over every flavor: one for a solid fill, the stops for a gradient. None leave
        ///        the state to the widget's flavors and the built-in colors; a state none of them sets shows the Enabled colors
        void SetColors(std::span<const glm::vec4> colors, const ColorState state);

        /// @brief Returns the colors set on the item texts themselves for the given state, empty when unset
        const std::vector<glm::vec4>& GetColors(const ColorState state) const;

    //Called by the owning widget
        /// @brief The colors an item shows in the given state: the own ones, else the flavors', else the built-in ones
        const std::vector<glm::vec4>& GetShownColors(const ColorState state) const;

    private:
        void restyle() override; //the colors are read when drawn, so there is nothing to take again

        LayeredColors _colors {};
    };
}
