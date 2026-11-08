#pragma once
#include "IncludeHelper.hpp"
#include <span>
#include <vector>

namespace RetroFuturaGUI
{
    /// @brief The colors of a list's item texts per state: Enabled, Disabled for disabled items, Hover for the hovered row and
    ///        Clicked for the picked one. The widget owns it and hands it out through GetItemText(), so it is styled as
    ///        comboBox.GetItemText().SetColors(...)
    class ItemTextStyle final
    {
    public:
        /// @brief Sets the colors for the given state: one for a solid fill, the stops for a gradient. Selected colors are optional:
        ///        without any, the Selected state shows the Enabled colors.
        void SetColors(std::span<const glm::vec4> colors, const ColorState state);

        /// @brief Returns the colors for the given state.
        const std::vector<glm::vec4>& GetColors(const ColorState state) const;

    //Called by the owning widget
        /// @brief The colors an item shows in the given state: its own, or the Enabled ones for a Selected state without any.
        const std::vector<glm::vec4>& GetShownColors(const ColorState state) const;

    private:
        std::vector<glm::vec4>& colorsOf(const ColorState state);

        std::vector<glm::vec4>
            _enabled { glm::vec4(1.0f) },
            _disabled { glm::vec4(0.5f, 0.5f, 0.5f, 1.0f) },
            _hover { glm::vec4(1.0f) },
            _clicked { glm::vec4(0.5f, 0.5f, 0.5f, 1.0f) },
            _selected {};
    };
}
