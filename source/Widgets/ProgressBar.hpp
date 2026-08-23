#pragma once
#include "IBackground.hpp"
#include "IBorder.hpp"
#include "IClickable.hpp"
#include "IWidget.hpp"
#include "IncludeHelper.hpp"
#include "Texture.hpp"
#include "config.hpp"
#include <glm/ext/vector_int2_sized.hpp>
#include <memory>
#include <span>

namespace RetroFuturaGUI
{
    /// @brief A ProgressBar that visualizes a numeric value
    class ProgressBar : public IWidget, public IClickable, public IBackground, public IBorder
    {
    public:
        /// @brief Constructs the ProgressBar
        ProgressBar(const std::string& name, Projection* projection, IWidget* parentWidget, const WidgetTypeID parentWidgetTypeID, GLFWwindow* parentWindow);
        ~ProgressBar() = default;
        ProgressBar(const ProgressBar&) = delete;
        ProgressBar(ProgressBar&&) = delete;
        auto operator =(const ProgressBar&) = delete;
        auto operator =(ProgressBar&&) = delete;

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

        /// @brief Draws the ProgressBar
        void Draw() override;

        /// @brief Sets the ProgressBar's position
        void SetPosition(const glm::vec3& position) override;

        /// @brief Sets the ProgressBar's size
        void SetSize(const glm::vec3& size) override;

        /// @brief Sets the ProgressBar's Rotation
        void SetRotation(const glm::vec3& rotation) override;

        /// @brief Sets whether the graph is drawn as a solid fill (Bar) or as a wave (Wave).
        /// @note GraphMode::Wave currently renders identically to GraphMode::Bar — wave geometry isn't implemented yet.
        void SetGraphMode(const GraphMode mode);

        void SetGraphColors(std::span<glm::vec4> colors, const ColorState state);

        /// @brief Sets the corner rounding radii of the ProgressBar's track background and border.
        void SetCornerRadii(const glm::vec4& radii);

        /// @brief Sets the corner rounding radii of the ProgressBar's graph.
        void SetGraphCornerRadii(const glm::vec4& radii);

        /// @brief Sets the graph's thickness (its size along the track's short axis). Values at or above the track's height are clamped to it; 0 matches the track's height exactly.
        void SetGraphWidth(const f32 width);

        /// @brief Sets the graph background fill type (solid, linear/radial/huestar gradient).
        void SetGraphFillType(const FillType fillType);

        /// @brief Sets the offset applied to the graph background gradient's start position.
        void SetGraphGradientOffset(const f32 gradientOffset);

        /// @brief Sets the speed at which the graph background gradient animates over time.
        void SetGraphGradientAnimationSpeed(const f32 animationSpeed);

        /// @brief Sets the angle of the graph background's linear gradient, in degrees.
        void SetGraphGradientDegree(const f32 degree);

        /// @brief Sets the speed at which the graph background gradient rotates over time.
        void SetGraphGradientRotationSpeed(const f32 rotationSpeed);

        /// @brief Sets the texture ID sampled for the graph background's glass-effect-with-image shader feature.
        void SetGraphWindowBackgroundImageTextureID(const u32 textureID);

        /// @brief Sets the dot color used for the graph background's DottedPattern shader feature.
        void SetGraphDotColor(const glm::vec4& color);

        /// @brief Sets the spacing between dot centers, in pixels, for the graph background's DottedPattern shader feature.
        void SetGraphDotDistance(const f32 distance);

        /// @brief Sets the direction, in degrees, along which the graph background's dot radii/animation transfer.
        void SetGraphDotSizeTransferDegree(const f32 degree);

        /// @brief Sets the per-position dot radius curve, in pixels, for the graph background's DottedPattern shader feature. Enables the feature when non-empty.
        void SetGraphDotRadiusTransfer(std::span<f32> radiusTransfer);

        /// @brief Sets how far each graph background dot's opacity reaches from its center before fading to transparent.
        void SetGraphDotTransparencyTransfer(const f32 transparencyTransfer);

        /// @brief Sets the speed at which the graph background's dotted pattern animates.
        void SetGraphDotAnimationSpeed(const f32 animationSpeed);

        /// @brief Sets the overall opacity of the graph background's FogEffect shader feature.
        void SetGraphFogAlpha(const f32 alpha);

        /// @brief Sets the speed at which the graph background's fog drifts over time.
        void SetGraphFogSpeed(const f32 speed);

        /// @brief Sets the per-octave density/weight curve driving the graph background's fog. Enables the FogEffect shader feature when non-empty.
        void SetGraphFogDensity(std::span<f32> density);

        /// @brief Sets the coverage threshold above which the graph background's fog appears; higher values carve larger clear gaps out of the cloud.
        void SetGraphFogClearing(const f32 clearing);

        /// @brief Enables or disables the indicator marker that travels along the graph's current edge.
        void EnableIndicator(const bool value);

        /// @brief Sets the indicator's shape (a plain stroke or a fully-rounded circle).
        void SetIndicatorType(const IndicatorType type);

        /// @brief Sets the indicator's side length, in pixels.
        void SetIndicatorSize(const f32 size);

        void SetIndicatorColors(std::span<glm::vec4> colors, const ColorState state);

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
                _valueType = ProgressBarValueType::Int8;
            }
            else if constexpr (std::is_same_v<T, i16>)
            {
                _value.Int16 = value < _minValue.Int16 ? _minValue.Int16 : value;
                _value.Int16 = value > _maxValue.Int16 ? _maxValue.Int16 : value;
                _valueType = ProgressBarValueType::Int16;
            }
            else if constexpr (std::is_same_v<T, i32>)
            {
                _value.Int32 = value < _minValue.Int32 ? _minValue.Int32 : value;
                _value.Int32 = value > _maxValue.Int32 ? _maxValue.Int32 : value;
                _valueType = ProgressBarValueType::Int32;
            }
            else if constexpr (std::is_same_v<T, i64>)
            {
                _value.Int64 = value < _minValue.Int64 ? _minValue.Int64 : value;
                _value.Int64 = value > _maxValue.Int64 ? _maxValue.Int64 : value;
                _valueType = ProgressBarValueType::Int64;
            }
            else if constexpr (std::is_same_v<T, u8>)
            {
                _value.UInt8 = value < _minValue.UInt8 ? _minValue.UInt8 : value;
                _value.UInt8 = value > _maxValue.UInt8 ? _maxValue.UInt8 : value;
                _valueType = ProgressBarValueType::UInt8;
            }
            else if constexpr (std::is_same_v<T, u16>)
            {
                _value.UInt16 = value < _minValue.UInt16 ? _minValue.UInt16 : value;
                _value.UInt16 = value > _maxValue.UInt16 ? _maxValue.UInt16 : value;
                _valueType = ProgressBarValueType::UInt16;
            }
            else if constexpr (std::is_same_v<T, u32>)
            {
                _value.UInt32 = value < _minValue.UInt32 ? _minValue.UInt32 : value;
                _value.UInt32 = value > _maxValue.UInt32 ? _maxValue.UInt32 : value;
                _valueType = ProgressBarValueType::UInt32;
            }
            else if constexpr (std::is_same_v<T, u64>)
            {
                _value.UInt64 = value < _minValue.UInt64 ? _minValue.UInt64 : value;
                _value.UInt64 = value > _maxValue.UInt64 ? _maxValue.UInt64 : value;
                _valueType = ProgressBarValueType::UInt64;
            }
            else if constexpr (std::is_same_v<T, f32>)
            {
                _value.Float32 = value < _minValue.Float32 ? _minValue.Float32 : value;
                _value.Float32 = value > _maxValue.Float32 ? _maxValue.Float32 : value;
                _valueType = ProgressBarValueType::Float32;
            }
            else if constexpr (std::is_same_v<T, f64>)
            {
                _value.Float64 = value < _minValue.Float64 ? _minValue.Float64 : value;
                _value.Float64 = value > _maxValue.Float64 ? _maxValue.Float64 : value;
                _valueType = ProgressBarValueType::Float64;
            }
            else
            {
                _value.Bool = value;
                _valueType = ProgressBarValueType::Bool;
            }

            if(emitSignal)
            {
                _onValueChanged.Emit();
                _onValueChangedAsync.EmitAsync();
            }

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

            setGraphPosition();
        }

        template <typename T> const T GetValue() const
        {
            return getValueByType<T>();
        }

    private:
        void interact();
        bool isInsideGraph(const glm::vec2& mousePos);
        void setGraphColors();
        void setGraphColors(const ColorState state);
        void setGraphSize();
        void setGraphPosition();
        void calculateGraphProgressBarPosition();
        void setColors(const ColorState state);
        void setIndicatorPosition();
        void setIndicatorShape();
        void setIndicatorColors();
        void setIndicatorColors(const ColorState state);

        template<typename T> T getValueByType() const
        {
            switch(_valueType)
            {
                case ProgressBarValueType::Int8:
                {
                    if constexpr (std::is_same_v<T, i8>)
                        return _value.Int8;
                    else
                        return static_cast<T>(_value.Int8);
                }
                case ProgressBarValueType::Int16:
                {
                    if constexpr (std::is_same_v<T, i16>)
                        return _value.Int16;
                    else
                        return static_cast<T>(_value.Int16);
                }
                case ProgressBarValueType::Int32:
                {
                    if constexpr (std::is_same_v<T, i32>)
                        return _value.Int32;
                    else
                        return static_cast<T>(_value.Int32);
                }
                case ProgressBarValueType::Int64:
                {
                    if constexpr (std::is_same_v<T, i64>)
                        return _value.Int64;
                    else
                        return static_cast<T>(_value.Int64);
                }
                case ProgressBarValueType::UInt8:
                {
                    if constexpr (std::is_same_v<T, u8>)
                        return _value.UInt8;
                    else
                        return static_cast<T>(_value.UInt8);
                }
                case ProgressBarValueType::UInt16:
                {
                    if constexpr (std::is_same_v<T, u16>)
                        return _value.UInt16;
                    else
                        return static_cast<T>(_value.UInt16);
                }
                case ProgressBarValueType::UInt32:
                {
                    if constexpr (std::is_same_v<T, u32>)
                        return _value.UInt32;
                    else
                        return static_cast<T>(_value.UInt32);
                }
                case ProgressBarValueType::UInt64:
                {
                    if constexpr (std::is_same_v<T, u64>)
                        return _value.UInt64;
                    else
                        return static_cast<T>(_value.UInt64);
                }
                case ProgressBarValueType::Float32:
                {
                    if constexpr (std::is_same_v<T, f32>)
                        return _value.Float32;
                    else
                        return static_cast<T>(_value.Float32);
                }
                case ProgressBarValueType::Float64:
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

        enum class ProgressBarValueType : u32
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

        // Elements
        std::unique_ptr<Rectangle>
            _graph { nullptr },
            _indicator { nullptr };

        // Settings
        ProgressBarValueType _valueType { ProgressBarValueType::Int32 };
        GraphMode _graphMode { GraphMode::Bar };
        f32 _graphValueFraction { 0.0f }; // (value - min) / (max - min), in [0, 1]
        f32 _graphWidth { 0.0f };         // graph thickness; 0 matches the track's height
        bool _useIndicator { false };
        IndicatorType _indicatorType { IndicatorType::None };
        f32 _indicatorSize { 12.0f };

        //Design
        //std::unique_ptr<Texture> _graphImage { nullptr };
        std::vector<glm::vec4>
            _graphColorEnabled { glm::vec4(1.0f, 0.0f, 0.0f, 1.0f) },
            _graphColorClicked { glm::vec4(1.0f, 1.0f, 0.0f, 1.0f) },
            _graphColorHover { glm::vec4(0.0f, 0.0f, 1.0f, 1.0f) },
            _graphColorDisabled { glm::vec4(0.5f, .5f, 0.5f, 1.0f) },
            _indicatorColorEnabled { glm::vec4(1.0f) },
            _indicatorColorClicked {glm::vec4(1.0f, 0.0f, 1.0f, 1.0f)},
            _indicatorColorHover { glm::vec4(1.0f) },
            _indicatorColorDisabled { glm::vec4(0.5f, 0.5f, 0.5f, 1.0f) };
        ColorState
            _graphColorState { ColorState::Enabled },
            _indicatorColorState { ColorState::Enabled };
        std::vector<f32>
            _graphDotRadiusTransfer,
            _graphFogDensity;

        Signal<>
            _onValueChanged,
            _onValueChangedAsync,
            _onValueSet,
            _onValueSetAsync;
    };
}
