#pragma once
#include "Projection.hpp"
#include "IncludeHelper.hpp"
#include "IRectangle.hpp"
#include "StylingManager.hpp"
#include "ShaderLayers.hpp"
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
        /// @bechaml internal
        void SetColorState(const ColorState state);

        /// @brief Returns the corner radii shown: the part's own, else its flavors', .default's or the built-in ones
        glm::vec4 GetCornerRadii() const;

        /// @brief Sets the corner radii, clockwise from the top left, which win over every flavor
        void SetCornerRadii(const glm::vec4& radii);

        /// @brief Sets which corners are cut off straight instead of rounded: SetCutCorners(CutCorners::TopLeft | CutCorners::BottomRight).
        ///        A cut corner is cut by its corner radius along each edge; the others stay rounded, or square with a radius of 0
        void SetCutCorners(const CutCorners corners);

        /// @brief Returns the cut corners shown
        CutCorners GetCutCorners() const;

        /// @brief Sets the angle in degrees the part's side edges lean by, which win over every flavor: above 0 the top moves right
        ///        and the bottom left, a parallelogram whose top and bottom stay level. Its shadow and glows lean with it. The
        ///        widget's text doesn't
        void SetSkew(const f32 degrees);

        /// @brief Returns the skew shown, in degrees
        f32 GetSkew() const;

        /// @brief Leaves a setting to the widget's flavors, .default and the built-in look (colors: set none instead)
        void ResetSetting(const StyleSetting setting);

    //Called by the widget that owns the part, which alone holds a StyleKey
        /// @brief Sets the built-in colors for a state: the widget type's own look, which everything else overrides
        void SetBuiltInColors(const StyleKey& key, std::span<const glm::vec4> colors, const ColorState state);

        /// @brief Sets the built-in corner radii
        void SetBuiltInCornerRadii(const StyleKey& key, const glm::vec4& radii);

    protected:
        virtual void setColors() = 0;
        /// @brief Hands the corner radii and the cut corners to the element
        virtual void setCornerShape() = 0;

        /// @brief Hands the element the shader settings whose value through the layers changed
        virtual void setShader() = 0;

        /// @brief Sets the raster's speed in one color state, for the part's SetRasterAnimationSpeed(speed, state)
        void setStateRasterAnimationSpeed(const f32 animationSpeed, const ColorState state);

        /// @brief Hands the element the raster speed of the state shown: the part's own for that state, else its flavors', else
        ///        the speed for every state. Called after the colors and the shader settings went to the element
        void followStateRasterAnimationSpeed();

        /// @brief Hands the raster speed to the element. Nothing for a part whose element has no raster
        virtual void setElementRasterAnimationSpeed(const f32 animationSpeed);

        /// @brief Moves the element's raster back to where it starts. Nothing for a part whose element has no raster
        virtual void resetElementRasterAnimation();

        /// @brief Resets a setting only the derived part keeps (a border's width, a background's image). False for any other
        virtual bool resetPartSetting(const StyleSetting setting);

        /// @brief Returns the colors set on the part itself for the state, empty when unset
        const std::vector<glm::vec4>& getOwnColors(const ColorState state) const;

        /// @brief Takes every value again when the flavors changed since it last did. Called first in Draw
        void followStyles();

        /// @brief Takes every value through its layers again and hands the ones that changed on
        virtual void restyle();

        /// @brief The widget's flavors, in order, which this part takes unset values from; nullptr for a part no flavor
        ///        styles (a sub-part, a window's), which goes from its own values straight to its built-in look
        std::span<const u32> getFlavors() const;

        /// @brief The state whose colors the part shows. Invalid while its widget is invalid and the part has colors for it
        ///        (Disabled still wins), else the state its widget handed it
        ColorState getShownState() const;

        Projection* _projection { nullptr };
        std::function<void()> _onCornerRadiiChange {};
        std::array<std::vector<glm::vec4>, ColorStateCount>
            _ownColors {}, //per ColorState, in its order.
            _builtInColors
            {
                std::vector<glm::vec4> { glm::vec4(0.5f, 0.5f, 0.5f, 1.0f) }, //Enabled
                std::vector<glm::vec4> { glm::vec4(0.25f, 0.25f, 0.25f, 1.0f) }, //Disabled
                std::vector<glm::vec4> { glm::vec4(0.6f, 0.6f, 0.6f, 1.0f) }, //Clicked
                std::vector<glm::vec4> { glm::vec4(0.55f, 0.55f, 0.55f, 1.0f) }, //Hover
                std::vector<glm::vec4> {}, //Selected
                std::vector<glm::vec4> {} //Invalid
            };
        std::optional<glm::vec4> _ownCornerRadii {};
        std::optional<CutCorners> _ownCutCorners {};
        std::optional<f32> _ownSkew {};
        std::array<std::optional<f32>, ColorStateCount> _ownRasterAnimationSpeeds {}; //per ColorState, in its order.
        bool _isStateRasterAnimationSpeedShown { false }; //whether the element moves its raster at a state's speed now
        glm::vec4
            _builtInCornerRadii { 0.0f },
            _cornerRadii { 0.0f }; //shown.
        CutCorners _cutCorners { CutCorners::None }; //shown.
        f32 _skew { 0.0f };
        const std::vector<u32>* _flavors { nullptr };
        StylePart _stylePart { StylePart::Background };
        u32 _styleGeneration { 0 }; //the StylingManager's generation the values were last taken at

        ShaderLayers _shader {};
        ColorState _colorState { ColorState::Enabled };
        bool _isInvalid { false }; //its widget's, handed over by the widget

        /// @brief The radii handed to the element. The shown ones, or the ones its window draws in their place (a docked window
        ///        squares its corners)
        glm::vec4 getDrawnCornerRadii() const;

    private:
        friend class IWidget; //makes it one of its styled parts
        friend class Window; //styles its parts like a widget, and squares their corners while docked

        /// @brief Has the part take unset values from the widget's flavors, as the given part of them
        void setStyleSource(const std::vector<u32>* flavors, const StylePart part);

        /// @brief Shows the Invalid colors, where the part has any, while its widget is invalid
        void setInvalid(const bool isInvalid);

        /// @brief The colors a state has through the layers, nullptr when none of them sets it (no Enabled fallback)
        const std::vector<glm::vec4>* findColors(const ColorState state) const;

        /// @brief Takes the corner radii and cut corners through their layers; hands them on and tells the owner if they changed
        void resolveCornerShape();

        /// @brief Draws the corners with these radii in place of the shown ones, nothing to draw the shown ones again. Called by a
        ///        window, whose asked-for radii stay what the layers show
        void setDrawnCornerRadii(const std::optional<glm::vec4>& radii);

        std::optional<glm::vec4> _drawnCornerRadii {};
    };
}