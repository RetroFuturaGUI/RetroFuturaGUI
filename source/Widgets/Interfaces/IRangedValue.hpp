#pragma once
#include "config.hpp"
#include "IncludeHelper.hpp"
#include "Rectangle.hpp"
#include <cmath>
#include <concepts>
#include <limits>
#include <memory>
#include <span>
#include <type_traits>
#include <utility>
#include <vector>
#include "IBackground.hpp"
#include "IBorder.hpp"
#include "IClickable.hpp"
#include "IWidget.hpp"
#include "IncludeHelper.hpp"

namespace RetroFuturaGUI
{
    /// @brief Shared range, track, indicator and graph for widgets that map values into a min/max range. IRangedSingleValue holds one value (Slider, ProgressBar), IRangedMultiValue many (Histogram, LineGraph).
    class IRangedValue : public IWidget, public IClickable, public IBackground, public IBorder
    {
    public:
        enum class ElementSizing : u32
        {
            Pixels,
            Percent
        };

        enum class GraphMode : u32
        {
            Bar,
            Wave
        };

        enum GraphDecoration : u32
        {
            None,
            Sweep,
            Indicator
        };

        enum class IndicatorType : u32
        {
            None,
            Stroke,
            Circle
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

        /// @brief Sets the position
        void SetPosition(const glm::vec3& position) override final;

        /// @brief Sets the size
        void SetSize(const glm::vec3& size) override final;

        /// @brief Sets the rotation
        void SetRotation(const glm::vec3& rotation) override final;

        /// @brief Sets whether the track runs horizontally or vertically at 0° rotation.
        void SetOrientation(const Orientation orientation);

        /// @brief Sets which end of the track holds the minimum value
        void SetTrackDirection(const TrackDirection direction);

        /// @brief Sets the indicator's size along the track's length
        /// @param size (glm::vec2): indicator's size
        /// @param sizingMode (ElementSizing): Whether the size is set in pixelsize (abolute) or in percent (relative to the track's total length)
        void SetIndicatorSize(const glm::vec2& size, const ElementSizing sizingMode);

        /// @brief Sets the corner rounding radii of the track's track background and border.
        void SetCornerRadii(const glm::vec4& radii);

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

        /// @brief Enables or disables the indicator marker. Lazily constructs it (background only) on first enable; subsequent toggles just show/hide it.
        void EnableIndicator(const bool value);

        /// @brief Sets the indicator's shape (a plain stroke or a fully-rounded circle).
        void SetIndicatorType(const IndicatorType type);

        void SetIndicatorBackgroundColors(std::span<glm::vec4> colors, const ColorState state);

        void SetIndicatorBorderColors(std::span<glm::vec4> colors, const ColorState state);

        /// @brief Sets the corner rounding radii of the indicator's background and border.
        void SetIndicatorCornerRadii(const glm::vec4& radii);

        /// @brief Sets the indicator background fill type (solid, linear/radial/huestar gradient).
        void SetIndicatorBackgroundFillType(const FillType fillType);

        /// @brief Sets the offset applied to the indicator background gradient's start position.
        void SetIndicatorBackgroundGradientOffset(const f32 gradientOffset);

        /// @brief Sets the speed at which the indicator background gradient animates over time.
        void SetIndicatorBackgroundGradientAnimationSpeed(const f32 animationSpeed);

        /// @brief Sets the angle of the indicator background's linear gradient, in degrees.
        void SetIndicatorBackgroundGradientDegree(const f32 degree);

        /// @brief Sets the speed at which the indicator background gradient rotates over time.
        void SetIndicatorBackgroundGradientRotationSpeed(const f32 rotationSpeed);

        /// @brief Sets the texture ID sampled for the indicator background's glass-effect-with-image shader feature.
        void SetIndicatorWindowBackgroundImageTextureID(const u32 textureID);

        /// @brief Sets the dot color used for the indicator background's Dotted raster pattern.
        void SetIndicatorBackgroundPrimaryRasterColor(const glm::vec4& color);

        /// @brief Sets the spacing between dot centers, in pixels, for the indicator background's Dotted raster pattern.
        void SetIndicatorBackgroundDotDistance(const f32 distance);

        /// @brief Sets the direction, in degrees, along which the indicator background's raster widths/animation transfer.
        void SetIndicatorBackgroundRasterDegree(const f32 degree);

        /// @brief Sets the per-position dot width curve, in pixels (each value is a dot's full width across), for the indicator background's Dotted raster pattern. Enables the Raster shader feature when non-empty.
        void SetIndicatorBackgroundPrimaryRasterWidthTransfer(std::span<f32> widthTransfer);

        /// @brief Sets how far each indicator background dot's opacity reaches from its center before fading to transparent.
        void SetIndicatorBackgroundDotTransparencyTransfer(const f32 transparencyTransfer);

        /// @brief Sets the speed at which the indicator background's raster pattern animates.
        void SetIndicatorBackgroundRasterAnimationSpeed(const f32 animationSpeed);

        /// @brief Sets the overall opacity of the indicator background's FogEffect shader feature.
        void SetIndicatorBackgroundFogAlpha(const f32 alpha);

        /// @brief Sets the speed at which the indicator background's fog drifts over time.
        void SetIndicatorBackgroundFogSpeed(const f32 speed);

        /// @brief Sets the per-octave density/weight curve driving the indicator background's fog. Enables the FogEffect shader feature when non-empty.
        void SetIndicatorBackgroundFogDensity(std::span<f32> density);

        /// @brief Sets the coverage threshold above which the indicator background's fog appears; higher values carve larger clear gaps out of the cloud.
        void SetIndicatorBackgroundFogClearing(const f32 clearing);

        /// @brief Sets the indicator border fill type (solid, linear/radial/huestar gradient).
        void SetIndicatorBorderFillType(const FillType fillType);

        /// @brief Sets the offset applied to the indicator border gradient's start position.
        void SetIndicatorBorderGradientOffset(const f32 gradientOffset);

        /// @brief Sets the speed at which the indicator border gradient animates over time.
        void SetIndicatorBorderGradientAnimationSpeed(const f32 animationSpeed);

        /// @brief Sets the angle of the indicator border's linear gradient, in degrees.
        void SetIndicatorBorderGradientDegree(const f32 degree);

        /// @brief Sets the speed at which the indicator border gradient rotates over time.
        void SetIndicatorBorderGradientRotationSpeed(const f32 rotationSpeed);

        /// @brief Sets the texture ID sampled for the indicator border's glass-effect-with-image shader feature.
        void SetIndicatorWindowBorderImageTextureID(const u32 textureID);

        /// @brief Sets the dot color used for the indicator border's Dotted raster pattern.
        void SetIndicatorBorderPrimaryRasterColor(const glm::vec4& color);

        /// @brief Sets the spacing between dot centers, in pixels, for the indicator border's Dotted raster pattern.
        void SetIndicatorBorderDotDistance(const f32 distance);

        /// @brief Sets the direction, in degrees, along which the indicator border's raster widths/animation transfer.
        void SetIndicatorBorderRasterDegree(const f32 degree);

        /// @brief Sets the per-position dot width curve, in pixels (each value is a dot's full width across), for the indicator border's Dotted raster pattern. Enables the Raster shader feature when non-empty.
        void SetIndicatorBorderPrimaryRasterWidthTransfer(std::span<f32> widthTransfer);

        /// @brief Sets how far each indicator border dot's opacity reaches from its center before fading to transparent.
        void SetIndicatorBorderDotTransparencyTransfer(const f32 transparencyTransfer);

        /// @brief Sets the speed at which the indicator border's raster pattern animates.
        void SetIndicatorBorderRasterAnimationSpeed(const f32 animationSpeed);

        /// @brief Sets the indicator's border width.
        void SetIndicatorBorderWidth(const f32 borderWidth);

        /// @brief Sets sections of the indicator's border to skip drawing; see Rectangle::SetBorderGaps.
        void SetIndicatorBorderGaps(std::span<BorderGap> gaps);

        /// @brief Enables or disables the graph (growing fill). Lazily constructs it on first enable; subsequent toggles just show/hide it.
        void EnableGraph(const bool value);

        /// @brief Sets whether the graph is drawn as a solid fill (Bar) or as a wave (Wave).
        /// @note GraphMode::Wave currently renders identically to GraphMode::Bar — wave geometry isn't implemented yet.
        void SetGraphMode(const GraphMode mode);

        void SetGraphColors(std::span<glm::vec4> colors, const ColorState state);

        /// @brief Sets the corner rounding radii of the graph.
        void SetGraphCornerRadii(const glm::vec4& radii);

        /// @brief Sets the graph's thickness (its size along the track's short axis). Values at or above the track's height are clamped to it; 0 matches the track's height exactly.
        void SetGraphWidth(const f32 width);

        /// @brief Sets the graph fill type (solid, linear/radial/huestar gradient).
        void SetGraphFillType(const FillType fillType);

        /// @brief Sets the offset applied to the graph gradient's start position.
        void SetGraphGradientOffset(const f32 gradientOffset);

        /// @brief Sets the speed at which the graph gradient animates over time.
        void SetGraphGradientAnimationSpeed(const f32 animationSpeed);

        /// @brief Sets the angle of the graph's linear gradient, in degrees.
        void SetGraphGradientDegree(const f32 degree);

        /// @brief Sets the speed at which the graph gradient rotates over time.
        void SetGraphGradientRotationSpeed(const f32 rotationSpeed);

        /// @brief Sets the texture ID sampled for the graph's glass-effect-with-image shader feature.
        void SetGraphWindowBackgroundImageTextureID(const u32 textureID);

        /// @brief Sets the dot color used for the graph's Dotted raster pattern.
        void SetGraphPrimaryRasterColor(const glm::vec4& color);

        /// @brief Sets the spacing between dot centers, in pixels, for the graph's Dotted raster pattern.
        void SetGraphDotDistance(const f32 distance);

        /// @brief Sets the direction, in degrees, along which the graph's raster widths/animation transfer.
        void SetGraphRasterDegree(const f32 degree);

        /// @brief Sets the per-position dot width curve, in pixels (each value is a dot's full width across), for the graph's Dotted raster pattern. Enables the Raster shader feature when non-empty.
        void SetGraphPrimaryRasterWidthTransfer(std::span<f32> widthTransfer);

        /// @brief Sets how far each graph dot's opacity reaches from its center before fading to transparent.
        void SetGraphDotTransparencyTransfer(const f32 transparencyTransfer);

        /// @brief Sets the speed at which the graph's raster pattern animates.
        void SetGraphRasterAnimationSpeed(const f32 animationSpeed);

        /// @brief Sets the overall opacity of the graph's FogEffect shader feature.
        void SetGraphFogAlpha(const f32 alpha);

        /// @brief Sets the speed at which the graph's fog drifts over time.
        void SetGraphFogSpeed(const f32 speed);

        /// @brief Sets the per-octave density/weight curve driving the graph's fog. Enables the FogEffect shader feature when non-empty.
        void SetGraphFogDensity(std::span<f32> density);

        /// @brief Sets the coverage threshold above which the graph's fog appears; higher values carve larger clear gaps out of the cloud.
        void SetGraphFogClearing(const f32 clearing);

    protected:
        // Non-owning alias of the projection the elements are created with
        Projection* _elementProjection { nullptr };

        /// @brief The rectangle the values are laid out along: the widget's background, read each time so a replaced _background can't leave a stale pointer behind.
        Rectangle* getTrack() const { return _background.get(); }

        /// @brief Folds the orientation into the widget's own rotation.
        /// @return The given rotation for Horizontal, or that rotation turned a quarter turn counter-clockwise for Vertical.
        glm::vec3 orientedRotation(const glm::vec3& rotation) const;

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
        std::unique_ptr<Rectangle>
            _indicatorBackground { nullptr },
            _indicatorBorder { nullptr },
            _graph { nullptr };
        ElementSizing _indicatorSizingMode { ElementSizing::Percent };

        // Settings
        bool _useIndicator { false };
        bool _useGraph { false };
        IndicatorType _indicatorType { IndicatorType::None };
        GraphMode _graphMode { GraphMode::Bar };
        f32 _graphWidth { 0.0f }; // graph thickness; 0 matches the track's height
        Orientation _orientation { Orientation::Horizontal };
        TrackDirection _trackDirection { TrackDirection::Normal };
        glm::i32vec2 _previousIndicatorPosition { glm::i32vec2(0) };
        glm::vec2 _indicatorSize { 25.0f, 100.0f };

        // Design
        std::vector<glm::vec4>
            _indicatorBackgroundColorEnabled { glm::vec4(1.0f, 0.0f, 0.0f, 1.0f) },
            _indicatorBackgroundColorClicked { glm::vec4(0.0f, 1.0f, 0.0f, 1.0f) },
            _indicatorBackgroundColorHover { glm::vec4(0.0f, 0.0f, 1.0f, 1.0f) },
            _indicatorBackgroundColorDisabled { glm::vec4(0.5f, .5f, 0.5f, 1.0f) },
            _indicatorBorderColorEnabled { glm::vec4(1.0f) },
            _indicatorBorderColorClicked { glm::vec4(1.0f) },
            _indicatorBorderColorHover { glm::vec4(1.0f) },
            _indicatorBorderColorDisabled { glm::vec4(1.0f) },
            _graphColorEnabled { glm::vec4(1.0f, 0.0f, 0.0f, 1.0f) },
            _graphColorClicked { glm::vec4(1.0f, 1.0f, 0.0f, 1.0f) },
            _graphColorHover { glm::vec4(0.0f, 0.0f, 1.0f, 1.0f) },
            _graphColorDisabled { glm::vec4(0.5f, .5f, 0.5f, 1.0f) };
        ColorState
            _indicatorBackgroundColorState { ColorState::Enabled },
            _indicatorBorderColorState { ColorState::Enabled },
            _graphColorState { ColorState::Enabled };
        std::vector<f32>
            _indicatorBackgroundPrimaryRasterWidthTransfer,
            _indicatorBackgroundFogDensity,
            _indicatorBorderPrimaryRasterWidthTransfer,
            _graphPrimaryRasterWidthTransfer,
            _graphFogDensity;

    private:
        void setIndicatorBackgroundColors();
        void setIndicatorBorderColors();
        void setGraphColorsApply();
        void setIndicatorSize();
    };
}
