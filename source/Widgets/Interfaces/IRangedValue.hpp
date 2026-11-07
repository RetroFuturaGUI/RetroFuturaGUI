#pragma once
#include "config.hpp"
#include "IncludeHelper.hpp"
#include "Frame.hpp"
#include "Plane.hpp"
#include "WavePlane.hpp"
#include <cmath>
#include <concepts>
#include <limits>
#include <memory>
#include <span>
#include <type_traits>
#include <utility>
#include <vector>
#include "Background.hpp"
#include "Border.hpp"
#include "Indicator.hpp"
#include "Graph.hpp"
#include "IClickable.hpp"
#include "IWidget.hpp"
#include "IncludeHelper.hpp"

namespace RetroFuturaGUI
{
    /// @brief Shared range, track, indicator and graph for widgets that map values into a min/max range. IRangedSingleValue holds one value (Slider, ProgressBar), IRangedMultiValue many (Histogram, LineGraph)
    class IRangedValue : public IWidget, public IClickable
    {
    public:
        enum GraphDecoration : u32
        {
            None,
            Sweep,
            Indicator
        };

        enum class Orientation : u32
        {
            Horizontal,
            Vertical
        };

        /// @brief Which end of the track the minimum value sits at.
        enum class TrackDirection : u32
        {
            Normal,
            Inverted
        };

        IRangedValue(std::string_view name, Projection* projection, IHierarchyNode* parentWidget, const WidgetTypeID parentWidgetTypeID, GLFWwindow* parentWindow);

        /// @brief Sets whether the track runs horizontally or vertically at 0° rotation.
        void SetOrientation(const Orientation orientation);

        /// @brief Sets which end of the track holds the minimum value
        void SetTrackDirection(const TrackDirection direction);

        Background& GetBackground();
        const Background& GetBackground() const;
        Border& GetBorder();
        const Border& GetBorder() const;

        /// @brief Returns the marker riding the track: shown or hidden, its shape, size, background and border
        RetroFuturaGUI::Indicator& GetIndicator();

        /// @brief Returns the marker riding the track: shown or hidden, its shape, size, background and border
        const RetroFuturaGUI::Indicator& GetIndicator() const;

        /// @brief Returns the fill growing along the track. Shown or hidden, a bar or a wave, its thickness, colors and effects
        RetroFuturaGUI::Graph& GetGraph();

        /// @brief Returns the fill growing along the track. Shown or hidden, a bar or a wave, its thickness, colors and effects
        const RetroFuturaGUI::Graph& GetGraph() const;

        /// @brief Sets the range's lower bound, stored in the type resolveValueType picks: T on a single-value widget (converting the value and step to T
        /// as well, see setValueType), the data's type on a multi-value widget that has data (so 0.5f becomes 0 for u8 data).
        template <NumericValueType T> void SetMinValue(T value)
        {
            setValueType(resolveValueType(GetPrimitiveTypeID<T>()));
            _minValue = convertPrimitiveTo(value, _valueType);
            alignValueToRange();
        }

        /// @brief Sets the range's upper bound, stored in the type resolveValueType picks: T on a single-value widget (converting the value and step to T
        /// as well, see setValueType), the data's type on a multi-value widget that has data (so 0.5f becomes 0 for u8 data).
        template <NumericValueType T> void SetMaxValue(T value)
        {
            setValueType(resolveValueType(GetPrimitiveTypeID<T>()));
            _maxValue = convertPrimitiveTo(value, _valueType);
            alignValueToRange();
        }

    protected:
        // Non-owning alias of the projection the elements are created with
        Projection* _elementProjection { nullptr };

        /// @brief The rectangle the values are laid out along.
        const Background* getTrack() const { return &_background; }

        /// @brief The rotation the orientation gives the track. None for Horizontal. A quarter turn counter-clockwise for
        ///        Vertical. The widget's own rotation is in its frame
        glm::vec3 orientedRotation() const;

        /// @brief Turns the track to the orientation and places the indicator along it.
        void orientTrack();

        /// @brief Places the indicator along the track.
        /// @param trackFraction Where along the track, 0 at the track's start; see toTrackFraction.
        void setIndicatorPosition(const f32 trackFraction);

        /// @brief Sizes the graph to fill the track up to trackFraction.
        void setGraphSize(const f32 trackFraction);

        /// @brief Sizes the graph to trackFraction and anchors it to the track's start.
        void setGraphPosition(const f32 trackFraction);

        void setIndicatorColors(const ColorState state);
        void setGraphColors(const ColorState state);
        void drawIndicator();
        virtual void drawGraph();

        /// @brief Called after SetMinValue or SetMaxValue stored a new bound. Clamps whatever values the derived interface owns.
        virtual void alignValueToRange() = 0;

        /// @brief Called after the track moved, resized, rotated or changed direction, or an element was enabled or resized. Places the indicator and graph for the derived interface's values.
        /// @note Pure virtual, so nothing in IRangedValue's constructor may lead here.
        virtual void alignElementsToTrack() = 0;

        /// @brief Reads a value stored in the range's type (_valueType) as f32.
        f32 toF32(const PrimitiveUnion value) const;

        /// @brief Where value sits in the range: 0 at the minimum, 1 at the maximum. Not clamped - a value outside the range maps outside 0..1.
        f32 getRangeFraction(const f64 value) const;

        /// @brief Turns a range fraction into a position along the track, 0 at the track's start. The same unless the track direction is Inverted.
        f32 toTrackFraction(const f32 rangeFraction) const;

        /// @brief Makes type the one the range and the derived interface's values are read as.
        /// @note A union may only be read through the member last written. When the type changes, every union still holds the old type's bytes,
        /// so the bounds are converted to the new type and written through its member, and convertValuesToType does the same for the derived interface's values.
        void setValueType(const PrimitiveTypeID type);

        /// @brief Called when setValueType changed the type. Converts the values the derived interface owns from previousType to _valueType (see convertPrimitive).
        virtual void convertValuesToType(const PrimitiveTypeID previousType) = 0;

        /// @brief Picks the type SetMinValue and SetMaxValue store a bound in, given the type they were called with.
        /// @return requestedType - the range follows whatever type it is set with. IRangedMultiValue overrides this, because its data decides the type.
        virtual PrimitiveTypeID resolveValueType(const PrimitiveTypeID requestedType) const;

        /// @brief Converts value to T like static_cast, except where static_cast is undefined behavior or wraps: a value beyond T's limits
        /// saturates to the nearest limit, and NaN becomes 0 when T is an integer.
        template <NumericValueType T, NumericValueType From> static T saturatingCast(const From value)
        {
            if constexpr (std::same_as<T, bool> || std::same_as<From, bool>)
                return static_cast<T>(value);
            else if constexpr (std::is_integral_v<T> && std::is_floating_point_v<From>)
            {
                if(std::isnan(value))
                    return T {};

                if(value <= static_cast<From>(std::numeric_limits<T>::lowest()))
                    return std::numeric_limits<T>::lowest();

                if(value >= static_cast<From>(std::numeric_limits<T>::max()))
                    return std::numeric_limits<T>::max();

                return static_cast<T>(value);
            }
            else if constexpr (std::is_integral_v<T>)
            {
                if(std::cmp_less(value, std::numeric_limits<T>::lowest()))
                    return std::numeric_limits<T>::lowest();

                if(std::cmp_greater(value, std::numeric_limits<T>::max()))
                    return std::numeric_limits<T>::max();

                return static_cast<T>(value);
            }
            else if constexpr (std::same_as<T, f32> && std::same_as<From, f64>)
            {
                if(value < static_cast<f64>(std::numeric_limits<f32>::lowest()))
                    return std::numeric_limits<f32>::lowest();

                if(value > static_cast<f64>(std::numeric_limits<f32>::max()))
                    return std::numeric_limits<f32>::max();

                return static_cast<f32>(value);
            }
            else
                return static_cast<T>(value);
        }

        /// @brief value, stored as type from, converted to type to - saturating like saturatingCast, so 300 becomes 255 as u8, -1 becomes 0 as an unsigned type,
        /// and a fraction is cut off towards zero (2.7 becomes 2).
        static PrimitiveUnion convertPrimitive(const PrimitiveUnion value, const PrimitiveTypeID from, const PrimitiveTypeID to);

        /// @brief value converted to type to, written through that type's member so it is the member that may be read.
        template <NumericValueType From> static PrimitiveUnion convertPrimitiveTo(const From value, const PrimitiveTypeID to)
        {
            switch(to)
            {
                case PrimitiveTypeID::Int8:
                    return PrimitiveUnion { .Int8 = saturatingCast<i8>(value) };
                case PrimitiveTypeID::Int16:
                    return PrimitiveUnion { .Int16 = saturatingCast<i16>(value) };
                case PrimitiveTypeID::Int32:
                    return PrimitiveUnion { .Int32 = saturatingCast<i32>(value) };
                case PrimitiveTypeID::Int64:
                    return PrimitiveUnion { .Int64 = saturatingCast<i64>(value) };
                case PrimitiveTypeID::UInt8:
                    return PrimitiveUnion { .UInt8 = saturatingCast<u8>(value) };
                case PrimitiveTypeID::UInt16:
                    return PrimitiveUnion { .UInt16 = saturatingCast<u16>(value) };
                case PrimitiveTypeID::UInt32:
                    return PrimitiveUnion { .UInt32 = saturatingCast<u32>(value) };
                case PrimitiveTypeID::UInt64:
                    return PrimitiveUnion { .UInt64 = saturatingCast<u64>(value) };
                case PrimitiveTypeID::Float32:
                    return PrimitiveUnion { .Float32 = saturatingCast<f32>(value) };
                case PrimitiveTypeID::Float64:
                    return PrimitiveUnion { .Float64 = saturatingCast<f64>(value) };
                default: // Bool
                    return PrimitiveUnion { .Bool = saturatingCast<bool>(value) };
            }
        }

        // Written through the member matching _valueType's default, so that member is the one that may be read
        PrimitiveUnion
            _minValue { .Int32 = 0 },
            _maxValue { .Int32 = 1 };
        PrimitiveTypeID _valueType { PrimitiveTypeID::Int32 };

        // Elements
        Background _background { &_projection };
        Border _border { &_projection };
        RetroFuturaGUI::Indicator _indicator { &_projection, { ._OnLayoutChange = [this]() { setIndicatorSize(); alignElementsToTrack(); } } }; //the GraphDecoration enumerator hides the type's own name in here
        RetroFuturaGUI::Graph _graph { &_projection, { ._OnLayoutChange = [this]() { alignElementsToTrack(); } } };

        // Settings
        Orientation _orientation { Orientation::Horizontal };
        TrackDirection _trackDirection { TrackDirection::Normal };
        glm::i32vec2 _previousIndicatorPosition { glm::i32vec2(0) };

        // Design

    protected:
        /// @brief Sets the position
        void applyPosition(const glm::vec3& position) override final;

        /// @brief Sets the size
        void applySize(const glm::vec3& size) override final;

    private:
        void setIndicatorSize();
    };
}