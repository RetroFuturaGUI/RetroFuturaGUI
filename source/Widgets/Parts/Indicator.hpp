#pragma once
#include "IncludeHelper.hpp"
#include "Background.hpp"
#include "Border.hpp"
#include <functional>

namespace RetroFuturaGUI
{
    /// @brief The shape of a ranged widget's indicator
    enum class IndicatorType : u32
    {
        None,
        Stroke,
        Circle,
        Pill
    };

    /// @brief How a size is read: in pixels, or in percent of the space it sits in
    enum class ElementSizing : u32
    {
        Pixels,
        Percent
    };

    /// @brief What an indicator tells its owning widget, for a layout that depends on it
    struct IndicatorHooks
    {
        std::function<void()> _OnLayoutChange {};
    };

    /// @brief The marker riding a ranged widget's track, with a background and a border of its own. The widget owns it and hands
    ///        it out through GetIndicator(), so it is styled as slider.GetIndicator().GetBackground().SetColors(...)
    class Indicator final
    {
    public:
        explicit Indicator(Projection* projection, IndicatorHooks hooks = {});
        Indicator() = delete;
        Indicator(const Indicator&) = delete;
        Indicator(Indicator&&) = delete;
        ~Indicator() = default;
        auto operator =(const Indicator&) = delete;
        auto operator =(Indicator&&) = delete;

        /// @brief Shows or hides the indicator
        void Show(const bool show);

        /// @brief Returns whether the indicator is shown
        bool IsShown() const;

        /// @brief Sets the shape. A Circle is round, its diameter the smaller of the two sizes. A Pill keeps the size and rounds its
        ///        shorter sides fully; a Stroke squares the corners; None keeps the radii set on the background and the border
        void SetType(const IndicatorType type);

        /// @brief Returns the shape
        IndicatorType GetType() const;

        /// @brief Sets the size: x along the track's length, y across it. In pixels, or in percent of the track inside its border
        void SetSize(const glm::vec2& size, const ElementSizing sizingMode);

        /// @brief Returns the indicator's background
        Background& GetBackground();

        /// @brief Returns the indicator's background
        const Background& GetBackground() const;

        /// @brief Returns the indicator's border
        Border& GetBorder();

        /// @brief Returns the indicator's border
        const Border& GetBorder() const;

    //Called by the owning widget
        /// @brief Draws the background and the border, while the indicator is shown
        void Draw();

        /// @brief Sizes the indicator inside the space it may take, the track inside its border
        void Fit(const glm::vec2& space);

        /// @brief Places and turns the indicator, its border just in front of its background
        void Place(const glm::vec3& position, const glm::vec3& rotation);

        /// @brief Returns the size the indicator is drawn at
        glm::vec2 GetDrawnSize() const;

        /// @brief Returns where the indicator is drawn
        glm::vec3 GetPosition() const;

        /// @brief Returns how the indicator is turned
        glm::vec3 GetRotation() const;

        /// @brief Shows the colors of the given state
        void SetColorState(const ColorState state);

        /// @brief Has the colors and corner radii move to new values over the widget's transitions
        void SetTransitionSettings(const TransitionSettings* settings);

    private:
        /// @brief Rounds or squares the corners for the shape, unless it is None
        void applyType();

        /// @brief Tells the widget its layout changed, if it asked to know
        void notifyLayoutChange();

        Background _background;
        Border _border;
        std::function<void()> _onLayoutChange {};
        glm::vec2 _size { 25.0f, 100.0f };
        ElementSizing _sizingMode { ElementSizing::Percent };
        IndicatorType _type { IndicatorType::None };
        bool _isShown { true };
    };
}