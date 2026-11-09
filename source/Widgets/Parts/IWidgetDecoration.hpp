#pragma once
#include "Projection.hpp"
#include "IncludeHelper.hpp"
#include "IRectangle.hpp"
#include "StylingManager.hpp"
#include "FillLayers.hpp"
#include <glm/ext/vector_float4.hpp>
#include <array>
#include <functional>
#include <optional>
#include <vector>
#include <span>

namespace RetroFuturaGUI
{
    /// @brief What a background, a border or a graph shares. Colors per state and the corner shape. Each value shows what
    ///        the part sets itself. What it leaves unset comes from its widget's flavors and .default (StylingManager),
    ///        then from its widget's built-in look
    class IWidgetDecoration
    {
    public:
        IWidgetDecoration(Projection* projection, std::function<void()> onCornerRadiiChange);
        virtual ~IWidgetDecoration() = default;

        virtual void Draw() = 0;
        virtual void SetSize(const glm::vec2& size) = 0;
        virtual void SetPosition(const glm::vec3& position) = 0;
        virtual void SetRotation(const glm::vec3& rotation) = 0;
        virtual glm::vec2 GetSize() const = 0;
        virtual glm::vec3 GetPosition() const = 0;
        virtual glm::vec3 GetRotation() const = 0;

        /// @brief Sets the colors for the given color state, which win over every flavor. No colors leave the state to
        ///        the widget's flavors, .default and the built-in look
        void SetColors(std::span<const glm::vec4> colors, const ColorState state);

        /// @brief Returns the colors shown in the state: the part's own, else its flavors', .default's or the built-in ones.
        ///        A state none of them sets shows the Enabled colors
        const std::vector<glm::vec4>& GetShownColors(const ColorState state) const;

        /// @brief Shows the colors of the given state
        void SetColorState(const ColorState state);

        /// @brief Returns the corner radii shown: the part's own, else its flavors', .default's or the built-in ones
        glm::vec4 GetCornerRadii() const;

        /// @brief Sets the corner radii, clockwise from the top left, which win over every flavor
        void SetCornerRadii(const glm::vec4& radii);

        /// @brief Leaves the corner radii to the widget's flavors, .default and the built-in look
        void ResetCornerRadii();

        /// @brief Sets which corners are cut off straight instead of rounded: SetCutCorners(CutCorners::TopLeft | CutCorners::BottomRight).
        ///        A cut corner is cut by its corner radius along each edge; the others stay rounded, or square with a radius of 0
        void SetCutCorners(const CutCorners corners);

        /// @brief Leaves the cut corners to the widget's flavors, .default and the built-in look
        void ResetCutCorners();

        /// @brief Returns the cut corners shown
        CutCorners GetCutCorners() const;

        /// @brief Leave a fill setting to the widget's flavors, .default and the element's default
        void ResetFillType();
        void ResetGradientDegree();
        void ResetGradientOffset();
        void ResetGradientAnimationSpeed();
        void ResetGradientRotationSpeed();

    //Called by the widget that owns the part, which alone holds a StyleKey
        /// @brief Sets the built-in colors for a state: the widget type's own look, which everything else overrides
        void SetBuiltInColors(const StyleKey& key, std::span<const glm::vec4> colors, const ColorState state);

        /// @brief Sets the built-in corner radii
        void SetBuiltInCornerRadii(const StyleKey& key, const glm::vec4& radii);

    protected:
        virtual void setColors() = 0;
        /// @brief Hands the corner radii and the cut corners to the element
        virtual void setCornerShape() = 0;

        /// @brief Hands the element the fill settings whose value through the layers changed
        virtual void setFill() = 0;

        /// @brief Returns the colors set on the part itself for the state, empty when unset
        const std::vector<glm::vec4>& getOwnColors(const ColorState state) const;

        /// @brief Takes every value again when the flavors changed since it last did. Called first in Draw
        void followStyles();

        /// @brief Takes every value through its layers again and hands the ones that changed on
        virtual void restyle();

        /// @brief The widget's flavors, in order, which this part takes unset values from; nullptr for a part no flavor
        ///        styles (a sub-part, a window's), which goes from its own values straight to its built-in look
        std::span<const u32> getFlavors() const;

        Projection* _projection { nullptr };
        std::function<void()> _onCornerRadiiChange {};
        std::array<std::vector<glm::vec4>, 5>
            _ownColors {}, //per ColorState, in its order.
            _builtInColors
            {
                std::vector<glm::vec4> { glm::vec4(0.5f, 0.5f, 0.5f, 1.0f) }, //Enabled
                std::vector<glm::vec4> { glm::vec4(0.25f, 0.25f, 0.25f, 1.0f) }, //Disabled
                std::vector<glm::vec4> { glm::vec4(0.6f, 0.6f, 0.6f, 1.0f) }, //Clicked
                std::vector<glm::vec4> { glm::vec4(0.55f, 0.55f, 0.55f, 1.0f) }, //Hover
                std::vector<glm::vec4> {} //Selected
            };
        std::optional<glm::vec4> _ownCornerRadii {};
        std::optional<CutCorners> _ownCutCorners {};
        glm::vec4
            _builtInCornerRadii { 0.0f },
            _cornerRadii { 0.0f }; //shown.
        CutCorners _cutCorners { CutCorners::None }; //shown.
        const std::vector<u32>* _flavors { nullptr };
        StylePart _stylePart { StylePart::Background };
        u32 _styleGeneration { 0 }; //the StylingManager's generation the values were last taken at

        FillLayers _fill {};
        ColorState _colorState { ColorState::Enabled };

    private:
        friend class IWidget; //makes it one of its styled parts

        /// @brief Has the part take unset values from the widget's flavors, as the given part of them
        void setStyleSource(const std::vector<u32>* flavors, const StylePart part);

        /// @brief Takes the corner radii and cut corners through their layers; hands them on and tells the owner if they changed
        void resolveCornerShape();
    };
}