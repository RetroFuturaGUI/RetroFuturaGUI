#pragma once
#include "config.hpp"
#include "Signal.hpp"
#include "IncludeHelper.hpp"
#include "Rectangle.hpp"
#include <memory>
#include <span>
#include <vector>

namespace RetroFuturaGUI
{
    /// @brief Shared value/indicator/graph for widgets that visualize a min/max-bounded value (Slider, ProgressBar, ...)
    class IRangedValue
    {
    public:
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

        /// @brief Connects a slot to be called when the value has changed
        /// @param async If true, the slot is invoked asynchronously.
        void Connect_OnValueChanged(const typename Signal<>::Slot& slot, const bool async);

        /// @brief Connects a slot to be called when the value has been set
        /// @param async If true, the slot is invoked asynchronously.
        void Connect_OnValueSet(const typename Signal<>::Slot& slot, const bool async);

        /// @brief Disconnects a previously connected OnValueChanged slot.
        void Disconnect_OnValueChanged(const typename Signal<>::Slot& slot);

        /// @brief Disconnects a previously connected OnValueSet slot.
        void Disconnect_OnValueSet(const typename Signal<>::Slot& slot);

        template <typename T> void SetValue(T value, const bool emitSignal = true)
        {
            if constexpr (std::is_same_v<T, i8>)
            {
                _value.Int8 = value < _minValue.Int8 ? _minValue.Int8 : value;
                _value.Int8 = value > _maxValue.Int8 ? _maxValue.Int8 : value;
                _valueType = ValueType::Int8;
            }
            else if constexpr (std::is_same_v<T, i16>)
            {
                _value.Int16 = value < _minValue.Int16 ? _minValue.Int16 : value;
                _value.Int16 = value > _maxValue.Int16 ? _maxValue.Int16 : value;
                _valueType = ValueType::Int16;
            }
            else if constexpr (std::is_same_v<T, i32>)
            {
                _value.Int32 = value < _minValue.Int32 ? _minValue.Int32 : value;
                _value.Int32 = value > _maxValue.Int32 ? _maxValue.Int32 : value;
                _valueType = ValueType::Int32;
            }
            else if constexpr (std::is_same_v<T, i64>)
            {
                _value.Int64 = value < _minValue.Int64 ? _minValue.Int64 : value;
                _value.Int64 = value > _maxValue.Int64 ? _maxValue.Int64 : value;
                _valueType = ValueType::Int64;
            }
            else if constexpr (std::is_same_v<T, u8>)
            {
                _value.UInt8 = value < _minValue.UInt8 ? _minValue.UInt8 : value;
                _value.UInt8 = value > _maxValue.UInt8 ? _maxValue.UInt8 : value;
                _valueType = ValueType::UInt8;
            }
            else if constexpr (std::is_same_v<T, u16>)
            {
                _value.UInt16 = value < _minValue.UInt16 ? _minValue.UInt16 : value;
                _value.UInt16 = value > _maxValue.UInt16 ? _maxValue.UInt16 : value;
                _valueType = ValueType::UInt16;
            }
            else if constexpr (std::is_same_v<T, u32>)
            {
                _value.UInt32 = value < _minValue.UInt32 ? _minValue.UInt32 : value;
                _value.UInt32 = value > _maxValue.UInt32 ? _maxValue.UInt32 : value;
                _valueType = ValueType::UInt32;
            }
            else if constexpr (std::is_same_v<T, u64>)
            {
                _value.UInt64 = value < _minValue.UInt64 ? _minValue.UInt64 : value;
                _value.UInt64 = value > _maxValue.UInt64 ? _maxValue.UInt64 : value;
                _valueType = ValueType::UInt64;
            }
            else if constexpr (std::is_same_v<T, f32>)
            {
                _value.Float32 = value < _minValue.Float32 ? _minValue.Float32 : value;
                _value.Float32 = value > _maxValue.Float32 ? _maxValue.Float32 : value;
                _valueType = ValueType::Float32;
            }
            else if constexpr (std::is_same_v<T, f64>)
            {
                _value.Float64 = value < _minValue.Float64 ? _minValue.Float64 : value;
                _value.Float64 = value > _maxValue.Float64 ? _maxValue.Float64 : value;
                _valueType = ValueType::Float64;
            }
            else
            {
                _value.Bool = value;
                _valueType = ValueType::Bool;
            }

            if(emitSignal)
            {
                _onValueChanged.Emit();
                _onValueChangedAsync.EmitAsync();
            }

            setIndicatorPosition();
            setGraphPosition();
        }

        template <typename T> void SetMinValue(T value)
        {
            if constexpr (std::is_same_v<T, i8>)
            {
                _minValue.Int8 = value;

                if(_minValue.Int8 > _value.Int8)
                    _value.Int8 = _minValue.Int8;
            }
            else if constexpr (std::is_same_v<T, i16>)
            {
                _minValue.Int16 = value;

                if(_minValue.Int16 > _value.Int16)
                    _value.Int16 = _minValue.Int16;
            }
            else if constexpr (std::is_same_v<T, i32>)
            {
                _minValue.Int32 = value;

                if(_minValue.Int32 > _value.Int32)
                    _value.Int32 = _minValue.Int32;
            }
            else if constexpr (std::is_same_v<T, i64>)
            {
                _minValue.Int64 = value;

                if(_minValue.Int64 > _value.Int64)
                    _value.Int64 = _minValue.Int64;
            }
            else if constexpr (std::is_same_v<T, u8>)
            {
                _minValue.UInt8 = value;

                if(_minValue.UInt8 > _value.UInt8)
                    _value.UInt8 = _minValue.UInt8;
            }
            else if constexpr (std::is_same_v<T, u16>)
            {
                _minValue.UInt16 = value;

                if(_minValue.UInt16 > _value.UInt16)
                    _value.UInt16 = _minValue.UInt16;
            }
            else if constexpr (std::is_same_v<T, u32>)
            {
                _minValue.UInt32 = value;

                if(_minValue.UInt32 > _value.UInt32)
                    _value.UInt32 = _minValue.UInt32;
            }
            else if constexpr (std::is_same_v<T, u64>)
            {
                _minValue.UInt64 = value;

                if(_minValue.UInt64 > _value.UInt64)
                    _value.UInt64 = _minValue.UInt64;
            }
            else if constexpr (std::is_same_v<T, f32>)
            {
                _minValue.Float32 = value;

                if(_minValue.Float32 > _value.Float32)
                    _value.Float32 = _minValue.Float32;
            }
            else if constexpr (std::is_same_v<T, f64>)
            {
                _minValue.Float64 = value;

                if(_minValue.Float64 > _value.Float64)
                    _value.Float64 = _minValue.Float64;
            }
            else
                _minValue.Bool = value;

            setIndicatorPosition();
            setGraphPosition();
        }

        template <typename T> void SetMaxValue(T value)
        {
            if constexpr (std::is_same_v<T, i8>)
            {
                _maxValue.Int8 = value;

                if(_maxValue.Int8 < _value.Int8)
                    _value.Int8 = _maxValue.Int8;
            }
            else if constexpr (std::is_same_v<T, i16>)
            {
                _maxValue.Int16 = value;

                if(_maxValue.Int16 < _value.Int16)
                    _value.Int16 = _maxValue.Int16;
            }
            else if constexpr (std::is_same_v<T, i32>)
            {
                _maxValue.Int32 = value;

                if(_maxValue.Int32 < _value.Int32)
                    _value.Int32 = _maxValue.Int32;
            }
            else if constexpr (std::is_same_v<T, i64>)
            {
                _maxValue.Int64 = value;

                if(_maxValue.Int64 < _value.Int64)
                    _value.Int64 = _maxValue.Int64;
            }
            else if constexpr (std::is_same_v<T, u8>)
            {
                _maxValue.UInt8 = value;

                if(_maxValue.UInt8 < _value.UInt8)
                    _value.UInt8 = _maxValue.UInt8;
            }
            else if constexpr (std::is_same_v<T, u16>)
            {
                _maxValue.UInt16 = value;

                if(_maxValue.UInt16 < _value.UInt16)
                    _value.UInt16 = _maxValue.UInt16;
            }
            else if constexpr (std::is_same_v<T, u32>)
            {
                _maxValue.UInt32 = value;

                if(_maxValue.UInt32 < _value.UInt32)
                    _value.UInt32 = _maxValue.UInt32;
            }
            else if constexpr (std::is_same_v<T, u64>)
            {
                _maxValue.UInt64 = value;

                if(_maxValue.UInt64 < _value.UInt64)
                    _value.UInt64 = _maxValue.UInt64;
            }
            else if constexpr (std::is_same_v<T, f32>)
            {
                _maxValue.Float32 = value;

                if(_maxValue.Float32 < _value.Float32)
                    _value.Float32 = _maxValue.Float32;
            }
            else if constexpr (std::is_same_v<T, f64>)
            {
                _maxValue.Float64 = value;

                if(_maxValue.Float64 < _value.Float64)
                    _value.Float64 = _maxValue.Float64;
            }
            else
                _maxValue.Bool = value;

            setIndicatorPosition();
            setGraphPosition();
        }

        template <typename T> const T GetValue() const
        {
            switch(_valueType)
            {
                case ValueType::Int8:
                {
                    if constexpr (std::is_same_v<T, i8>)
                        return _value.Int8;
                    else
                        return static_cast<T>(_value.Int8);
                }
                case ValueType::Int16:
                {
                    if constexpr (std::is_same_v<T, i16>)
                        return _value.Int16;
                    else
                        return static_cast<T>(_value.Int16);
                }
                case ValueType::Int32:
                {
                    if constexpr (std::is_same_v<T, i32>)
                        return _value.Int32;
                    else
                        return static_cast<T>(_value.Int32);
                }
                case ValueType::Int64:
                {
                    if constexpr (std::is_same_v<T, i64>)
                        return _value.Int64;
                    else
                        return static_cast<T>(_value.Int64);
                }
                case ValueType::UInt8:
                {
                    if constexpr (std::is_same_v<T, u8>)
                        return _value.UInt8;
                    else
                        return static_cast<T>(_value.UInt8);
                }
                case ValueType::UInt16:
                {
                    if constexpr (std::is_same_v<T, u16>)
                        return _value.UInt16;
                    else
                        return static_cast<T>(_value.UInt16);
                }
                case ValueType::UInt32:
                {
                    if constexpr (std::is_same_v<T, u32>)
                        return _value.UInt32;
                    else
                        return static_cast<T>(_value.UInt32);
                }
                case ValueType::UInt64:
                {
                    if constexpr (std::is_same_v<T, u64>)
                        return _value.UInt64;
                    else
                        return static_cast<T>(_value.UInt64);
                }
                case ValueType::Float32:
                {
                    if constexpr (std::is_same_v<T, f32>)
                        return _value.Float32;
                    else
                        return static_cast<T>(_value.Float32);
                }
                case ValueType::Float64:
                {
                    if constexpr (std::is_same_v<T, f64>)
                        return _value.Float64;
                    else
                        return static_cast<T>(_value.Float64);
                }
                default:
                {
                    if constexpr (std::is_same_v<T, bool>)
                        return _value.Bool;
                    else
                         return static_cast<T>(_value.Bool);
                }
            }
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

        /// @brief Sets the dot color used for the indicator background's DottedPattern shader feature.
        void SetIndicatorBackgroundDotColor(const glm::vec4& color);

        /// @brief Sets the spacing between dot centers, in pixels, for the indicator background's DottedPattern shader feature.
        void SetIndicatorBackgroundDotDistance(const f32 distance);

        /// @brief Sets the direction, in degrees, along which the indicator background's dot radii/animation transfer.
        void SetIndicatorBackgroundDotSizeTransferDegree(const f32 degree);

        /// @brief Sets the per-position dot radius curve, in pixels, for the indicator background's DottedPattern shader feature. Enables the feature when non-empty.
        void SetIndicatorBackgroundDotRadiusTransfer(std::span<f32> radiusTransfer);

        /// @brief Sets how far each indicator background dot's opacity reaches from its center before fading to transparent.
        void SetIndicatorBackgroundDotTransparencyTransfer(const f32 transparencyTransfer);

        /// @brief Sets the speed at which the indicator background's dotted pattern animates.
        void SetIndicatorBackgroundDotAnimationSpeed(const f32 animationSpeed);

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

        /// @brief Sets the dot color used for the indicator border's DottedPattern shader feature.
        void SetIndicatorBorderDotColor(const glm::vec4& color);

        /// @brief Sets the spacing between dot centers, in pixels, for the indicator border's DottedPattern shader feature.
        void SetIndicatorBorderDotDistance(const f32 distance);

        /// @brief Sets the direction, in degrees, along which the indicator border's dot radii/animation transfer.
        void SetIndicatorBorderDotSizeTransferDegree(const f32 degree);

        /// @brief Sets the per-position dot radius curve, in pixels, for the indicator border's DottedPattern shader feature. Enables the feature when non-empty.
        void SetIndicatorBorderDotRadiusTransfer(std::span<f32> radiusTransfer);

        /// @brief Sets how far each indicator border dot's opacity reaches from its center before fading to transparent.
        void SetIndicatorBorderDotTransparencyTransfer(const f32 transparencyTransfer);

        /// @brief Sets the speed at which the indicator border's dotted pattern animates.
        void SetIndicatorBorderDotAnimationSpeed(const f32 animationSpeed);

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

        /// @brief Sets the dot color used for the graph's DottedPattern shader feature.
        void SetGraphDotColor(const glm::vec4& color);

        /// @brief Sets the spacing between dot centers, in pixels, for the graph's DottedPattern shader feature.
        void SetGraphDotDistance(const f32 distance);

        /// @brief Sets the direction, in degrees, along which the graph's dot radii/animation transfer.
        void SetGraphDotSizeTransferDegree(const f32 degree);

        /// @brief Sets the per-position dot radius curve, in pixels, for the graph's DottedPattern shader feature. Enables the feature when non-empty.
        void SetGraphDotRadiusTransfer(std::span<f32> radiusTransfer);

        /// @brief Sets how far each graph dot's opacity reaches from its center before fading to transparent.
        void SetGraphDotTransparencyTransfer(const f32 transparencyTransfer);

        /// @brief Sets the speed at which the graph's dotted pattern animates.
        void SetGraphDotAnimationSpeed(const f32 animationSpeed);

        /// @brief Sets the overall opacity of the graph's FogEffect shader feature.
        void SetGraphFogAlpha(const f32 alpha);

        /// @brief Sets the speed at which the graph's fog drifts over time.
        void SetGraphFogSpeed(const f32 speed);

        /// @brief Sets the per-octave density/weight curve driving the graph's fog. Enables the FogEffect shader feature when non-empty.
        void SetGraphFogDensity(std::span<f32> density);

        /// @brief Sets the coverage threshold above which the graph's fog appears; higher values carve larger clear gaps out of the cloud.
        void SetGraphFogClearing(const f32 clearing);

        /// @brief Sets the orientation of the ranged value
        /// @param orientation The orientation to set (Horizontal or Vertical)
        virtual void SetOrientation(const Orientation orientation);

    protected:
        // Non-owning aliases that derive from the widgets. This avoids the inclusion of IWidget
        Rectangle* _track { nullptr };
        Projection* _elementProjection { nullptr };

        /// @brief Folds the orientation into the widget's own rotation.
        /// @return The given rotation for Horizontal, or that rotation turned a quarter turn counter-clockwise for Vertical.
        glm::vec3 orientedRotation(const glm::vec3& rotation) const;

        void setIndicatorPosition();
        void setGraphSize();
        void setGraphPosition();
        void setValueFromMousePosition(const glm::vec2& mousePos);
        void setIndicatorColors(const ColorState state);
        void setGraphColors(const ColorState state);
        void drawIndicator();
        void drawGraph();

        enum class ValueType : u32
        {
            Bool,
            Int8,
            Int16,
            Int32,
            Int64,
            UInt8,
            UInt16,
            UInt32,
            UInt64,
            Float32,
            Float64
        };

        union
        {
            bool Bool;
            i8 Int8;
            i16 Int16;
            i32 Int32;
            i64 Int64;
            u8 UInt8;
            u16 UInt16;
            u32 UInt32;
            u64 UInt64;
            f32 Float32;
            f64 Float64;
        } _value { .UInt64 = 0 };

        union
        {
            bool Bool;
            i8 Int8;
            i16 Int16;
            i32 Int32;
            i64 Int64;
            u8 UInt8;
            u16 UInt16;
            u32 UInt32;
            u64 UInt64;
            f32 Float32;
            f64 Float64;
        } _minValue { .UInt64 = 0 };

        union
        {
            bool Bool;
            i8 Int8;
            i16 Int16;
            i32 Int32;
            i64 Int64;
            u8 UInt8;
            u16 UInt16;
            u32 UInt32;
            u64 UInt64;
            f32 Float32;
            f64 Float64;
        } _maxValue { .UInt64 = 0 };

        ValueType _valueType { ValueType::Int32 };

        // Elements
        std::unique_ptr<Rectangle>
            _indicatorBackground { nullptr },
            _indicatorBorder { nullptr },
            _graph { nullptr };

        // Settings
        bool _useIndicator { false };
        bool _useGraph { false };
        IndicatorType _indicatorType { IndicatorType::None };
        GraphMode _graphMode { GraphMode::Bar };
        f32 _graphWidth { 0.0f }; // graph thickness; 0 matches the track's height
        Orientation _orientation { Orientation::Horizontal };

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
            _indicatorBackgroundDotRadiusTransfer,
            _indicatorBackgroundFogDensity,
            _indicatorBorderDotRadiusTransfer,
            _graphDotRadiusTransfer,
            _graphFogDensity;

        Signal<>
            _onValueChanged,
            _onValueChangedAsync,
            _onValueSet,
            _onValueSetAsync;

    private:
        f32 getValueFraction() const;
        void setIndicatorBackgroundColors();
        void setIndicatorBorderColors();
        void setGraphColorsApply();
    };
}
