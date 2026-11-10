#pragma once
#include "IncludeHelper.hpp"
#include "StylingManager.hpp"
#include <array>
#include <span>
#include <vector>

namespace RetroFuturaGUI
{
    /// @brief A part inside a widget that the widget's flavors style and that isn't a background, a border or a text: an icon,
    ///        the caret, the text selection, item texts. Keeps where its values come from; the widget hands it its flavors
    ///        (IWidget::useSubPartFlavors). Until then it goes from its own values straight to its built-in ones
    class IStyledPart
    {
    public:
        /// @brief Restyles when the flavors changed since the part last took its values. Called by the part before it draws, or by
        ///        the widget before it reads the part (a table's cell and header styles)
        void followFlavors();

    protected:
        IStyledPart() = default;
        ~IStyledPart() = default;

        /// @brief Takes every value through the layers again
        virtual void restyle() = 0;

        /// @brief The widget's flavors, empty before the widget handed them over
        std::span<const u32> getFlavors() const;

        /// @brief Whether the widget handed the part its flavors
        bool isStyled() const;

        /// @brief Which of a flavor's parts the part takes its values from
        StylePart getStylePart() const;

    private:
        friend class IWidget; //hands it the flavors

        /// @brief Has the part take unset values from the widget's flavors, and takes them now
        void setStyleSource(const std::vector<u32>* flavors, const StylePart part);

        const std::vector<u32>* _flavors { nullptr }; //the widget's flavors, kept by the widget
        StylePart _stylePart { StylePart::Background };
        u32 _styleGeneration { 0 }; //the StylingManager's generation the values were last taken at
    };

    /// @brief A part's colors per state through the style layers. Its own, else the first of its widget's flavors (then
    ///        .default) that gives the state colors, else its built-in ones. A state none of them sets shows the Enabled colors
    class LayeredColors final
    {
    public:
        /// @brief Sets the part's own colors for a state. None leave the state to the flavors and the built-in colors
        void SetOwn(std::span<const glm::vec4> colors, const ColorState state);

        /// @brief Returns the part's own colors for a state, empty when unset
        const std::vector<glm::vec4>& GetOwn(const ColorState state) const;

        /// @brief Sets the built-in colors for a state. The part's own look, which everything else overrides
        void SetBuiltIn(std::span<const glm::vec4> colors, const ColorState state);

        /// @brief Returns the colors shown in a state
        /// @param isStyled Whether the part takes its widget's flavors at all
        const std::vector<glm::vec4>& GetShown(const ColorState state, std::span<const u32> flavors, const bool isStyled, const StylePart part) const;

        /// @brief Returns the colors a state has through the layers, nullptr when none of them sets it (no Enabled fallback)
        const std::vector<glm::vec4>* Find(const ColorState state, std::span<const u32> flavors, const bool isStyled, const StylePart part) const;

    private:
        std::array<std::vector<glm::vec4>, 5>
            _own {}, //per ColorState, in its order
            _builtIn {};
    };
}