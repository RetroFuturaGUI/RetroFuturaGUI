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
#include "Button.hpp"
#include <span>

namespace RetroFuturaGUI
{
    enum class ElementSizing : u32
    {
        Pixels,
        Percent
    };

    // A slider which value can be set by dragging its grab
    class Slider : public IWidget, public IClickable, public IBackground, public IBorder
    {
    public:
    /// @brief Constructs the Slider
        Slider(const std::string& name, Projection* projection, IWidget* parentWidget, const WidgetTypeID parentWidgetTypeID, GLFWwindow* parentWindow);
        ~Slider() = default;
        Slider(const Slider&) = delete;
        Slider(Slider&&) = delete;
        auto operator =(const Slider&) = delete;
        auto operator =(Slider&&) = delete;

        /// @brief Draws the Slider
        void Draw() override;

        /// @brief Sets the Slider's position
        void SetPosition(const glm::vec3& position) override;

        /// @brief Sets the Slider's size
        void SetSize(const glm::vec3& size) override;

        /// @brief Sets the Slider's Rotation
        void SetRotation(const glm::vec3& rotation) override;

        /// @brief Sets the Slider's grab's size along the slider's length
        /// @param size (f32): slider's length
        /// @param sizingMode (ElementSizing): Whether the size is set in pixelsize (abolute) or in percent (relative to the Slider's total length)
        void SetGrabSize(const f32 size, const ElementSizing sizingMode);

        void SetGrabBackgroundColors(std::span<glm::vec4> colors, const ColorState state);

        void SetGrabBorderColors(std::span<glm::vec4> colors, const ColorState state);

        /// @brief Sets the corner rounding radii of the Slider's track background and border.
        void SetCornerRadii(const glm::vec4& radii);

        /// @brief Sets the corner rounding radii of the Slider's grab background and border.
        void SetGrabCornerRadii(const glm::vec4& radii);

        /// @brief Sets the grab background fill type (solid, linear/radial/huestar gradient).
        void SetGrabBackgroundFillType(const FillType fillType);

        /// @brief Sets the offset applied to the grab background gradient's start position.
        void SetGrabBackgroundGradientOffset(const f32 gradientOffset);

        /// @brief Sets the speed at which the grab background gradient animates over time.
        void SetGrabBackgroundGradientAnimationSpeed(const f32 animationSpeed);

        /// @brief Sets the angle of the grab background's linear gradient, in degrees.
        void SetGrabBackgroundGradientDegree(const f32 degree);

        /// @brief Sets the speed at which the grab background gradient rotates over time.
        void SetGrabBackgroundGradientRotationSpeed(const f32 rotationSpeed);

        /// @brief Sets the texture ID sampled for the grab background's glass-effect-with-image shader feature.
        void SetGrabWindowBackgroundImageTextureID(const u32 textureID);

        /// @brief Sets the dot color used for the grab background's DottedPattern shader feature.
        void SetGrabBackgroundDotColor(const glm::vec4& color);

        /// @brief Sets the spacing between dot centers, in pixels, for the grab background's DottedPattern shader feature.
        void SetGrabBackgroundDotDistance(const f32 distance);

        /// @brief Sets the direction, in degrees, along which the grab background's dot radii/animation transfer.
        void SetGrabBackgroundDotSizeTransferDegree(const f32 degree);

        /// @brief Sets the per-position dot radius curve, in pixels, for the grab background's DottedPattern shader feature. Enables the feature when non-empty.
        void SetGrabBackgroundDotRadiusTransfer(std::span<f32> radiusTransfer);

        /// @brief Sets how far each grab background dot's opacity reaches from its center before fading to transparent.
        void SetGrabBackgroundDotTransparencyTransfer(const f32 transparencyTransfer);

        /// @brief Sets the speed at which the grab background's dotted pattern animates.
        void SetGrabBackgroundDotAnimationSpeed(const f32 animationSpeed);

        /// @brief Sets the overall opacity of the grab background's FogEffect shader feature.
        void SetGrabBackgroundFogAlpha(const f32 alpha);

        /// @brief Sets the speed at which the grab background's fog drifts over time.
        void SetGrabBackgroundFogSpeed(const f32 speed);

        /// @brief Sets the per-octave density/weight curve driving the grab background's fog. Enables the FogEffect shader feature when non-empty.
        void SetGrabBackgroundFogDensity(std::span<f32> density);

        /// @brief Sets the coverage threshold above which the grab background's fog appears; higher values carve larger clear gaps out of the cloud.
        void SetGrabBackgroundFogClearing(const f32 clearing);

        /// @brief Sets the grab border fill type (solid, linear/radial/huestar gradient).
        void SetGrabBorderFillType(const FillType fillType);

        /// @brief Sets the offset applied to the grab border gradient's start position.
        void SetGrabBorderGradientOffset(const f32 gradientOffset);

        /// @brief Sets the speed at which the grab border gradient animates over time.
        void SetGrabBorderGradientAnimationSpeed(const f32 animationSpeed);

        /// @brief Sets the angle of the grab border's linear gradient, in degrees.
        void SetGrabBorderGradientDegree(const f32 degree);

        /// @brief Sets the speed at which the grab border gradient rotates over time.
        void SetGrabBorderGradientRotationSpeed(const f32 rotationSpeed);

        /// @brief Sets the texture ID sampled for the grab border's glass-effect-with-image shader feature.
        void SetGrabWindowBorderImageTextureID(const u32 textureID);

        /// @brief Sets the dot color used for the grab border's DottedPattern shader feature.
        void SetGrabBorderDotColor(const glm::vec4& color);

        /// @brief Sets the spacing between dot centers, in pixels, for the grab border's DottedPattern shader feature.
        void SetGrabBorderDotDistance(const f32 distance);

        /// @brief Sets the direction, in degrees, along which the grab border's dot radii/animation transfer.
        void SetGrabBorderDotSizeTransferDegree(const f32 degree);

        /// @brief Sets the per-position dot radius curve, in pixels, for the grab border's DottedPattern shader feature. Enables the feature when non-empty.
        void SetGrabBorderDotRadiusTransfer(std::span<f32> radiusTransfer);

        /// @brief Sets how far each grab border dot's opacity reaches from its center before fading to transparent.
        void SetGrabBorderDotTransparencyTransfer(const f32 transparencyTransfer);

        /// @brief Sets the speed at which the grab border's dotted pattern animates.
        void SetGrabBorderDotAnimationSpeed(const f32 animationSpeed);

        /// @brief Sets the grab's border width.
        void SetGrabBorderWidth(const f32 borderWidth);

        /// @brief Sets sections of the grab's border to skip drawing; see Rectangle::SetBorderGaps.
        void SetGrabBorderGaps(std::span<BorderGap> gaps);

        void EnableSliderButtons(const bool value);

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
            if(emitSignal)
            {
                _onValueChanged.Emit();
                _onValueChangedAsync.EmitAsync();
            }

            if constexpr (std::is_same_v<T, i8>)
            {
                _value.Int8 = value < _minValue.Int8 ? _minValue.Int8 : value;
                _value.Int8 = value > _maxValue.Int8 ? _maxValue.Int8 : value;
                _valueType = SliderValueType::Int8;
            }
            else if constexpr (std::is_same_v<T, i16>)
            {
                _value.Int16 = value < _minValue.Int16 ? _minValue.Int16 : value;
                _value.Int16 = value > _maxValue.Int16 ? _maxValue.Int16 : value;
                _valueType = SliderValueType::Int16;
            }
            else if constexpr (std::is_same_v<T, i32>)
            {
                _value.Int32 = value < _minValue.Int32 ? _minValue.Int32 : value;
                _value.Int32 = value > _maxValue.Int32 ? _maxValue.Int32 : value;
                _valueType = SliderValueType::Int32;
            }
            else if constexpr (std::is_same_v<T, i64>)
            {
                _value.Int64 = value < _minValue.Int64 ? _minValue.Int64 : value;
                _value.Int64 = value > _maxValue.Int64 ? _maxValue.Int64 : value;
                _valueType = SliderValueType::Int64;
            }
            else if constexpr (std::is_same_v<T, u8>)
            {
                _value.UInt8 = value < _minValue.UInt8 ? _minValue.UInt8 : value;
                _value.UInt8 = value > _maxValue.UInt8 ? _maxValue.UInt8 : value;
                _valueType = SliderValueType::UInt8;
            }
            else if constexpr (std::is_same_v<T, u16>)
            {
                _value.UInt16 = value < _minValue.UInt16 ? _minValue.UInt16 : value;
                _value.UInt16 = value > _maxValue.UInt16 ? _maxValue.UInt16 : value;
                _valueType = SliderValueType::UInt16;
            }
            else if constexpr (std::is_same_v<T, u32>)
            {
                _value.UInt32 = value < _minValue.UInt32 ? _minValue.UInt32 : value;
                _value.UInt32 = value > _maxValue.UInt32 ? _maxValue.UInt32 : value;
                _valueType = SliderValueType::UInt32;
            }
            else if constexpr (std::is_same_v<T, u64>)
            {
                _value.UInt64 = value < _minValue.UInt64 ? _minValue.UInt64 : value;
                _value.UInt64 = value > _maxValue.UInt64 ? _maxValue.UInt64 : value;
                _valueType = SliderValueType::UInt64;
            }
            else if constexpr (std::is_same_v<T, f32>)
            {
                _value.Float32 = value < _minValue.Float32 ? _minValue.Float32 : value;
                _value.Float32 = value > _maxValue.Float32 ? _maxValue.Float32 : value;
                _valueType = SliderValueType::Float32;
            }
            else if constexpr (std::is_same_v<T, f64>)
            {
                _value.Float64 = value < _minValue.Float64 ? _minValue.Float64 : value;
                _value.Float64 = value > _maxValue.Float64 ? _maxValue.Float64 : value;
                _valueType = SliderValueType::Float64;
            }
            else
            {
                _value.Bool = value;
                _valueType = SliderValueType::Bool;
            }

            setGrabPosition();
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

            setGrabPosition();
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
            
            setGrabPosition();
        }

        template <typename T> const T GetValue() const
        {
            return getValueByType<T>();
        }

    private:
        void interact();
        bool isInsideGrab(const glm::vec2& mousePos);
        void setGrabBackgroundColors();
        void setGrabBorderColors();
        void setGrabSize();
        void setGrabPosition();
        void calculateGrabSliderPosition();
        void setValueFromMousePosition(const glm::vec2& mousePos);
        void setColors(const ColorState state);
        void setGrabColors(const ColorState state);

        template<typename T> T getValueByType() const
        {
            switch(_valueType)
            {
                case SliderValueType::Int8:
                {
                    if constexpr (std::is_same_v<T, i8>)
                        return _value.Int8;
                    else
                        return static_cast<T>(_value.Int8);
                }
                case SliderValueType::Int16:
                {
                    if constexpr (std::is_same_v<T, i16>)
                        return _value.Int16;
                    else
                        return static_cast<T>(_value.Int16);
                }
                case SliderValueType::Int32:
                {
                    if constexpr (std::is_same_v<T, i32>)
                        return _value.Int32;
                    else
                        return static_cast<T>(_value.Int32);
                }
                case SliderValueType::Int64:
                {
                    if constexpr (std::is_same_v<T, i64>)
                        return _value.Int32;
                    else
                        return static_cast<T>(_value.Int64);
                }
                case SliderValueType::UInt8:
                {
                    if constexpr (std::is_same_v<T, u8>)
                        return _value.Int32;
                    else
                        return static_cast<T>(_value.UInt8); 
                }
                case SliderValueType::UInt16:
                {
                    if constexpr (std::is_same_v<T, u16>)
                        return _value.Int32;
                    else
                        return static_cast<T>(_value.UInt16);
                }
                case SliderValueType::UInt32:
                {
                    if constexpr (std::is_same_v<T, u32>)
                        return _value.Int32;
                    else
                        return static_cast<T>(_value.UInt32);
                }
                case SliderValueType::UInt64:
                {
                    if constexpr (std::is_same_v<T, u64>)
                        return _value.Int32;
                    else
                        return static_cast<T>(_value.UInt64);
                }
                case SliderValueType::Float32:
                {
                    if constexpr (std::is_same_v<T, f32>)
                        return _value.Int32;
                    else
                        return static_cast<T>(_value.Float32);
                }
                case SliderValueType::Float64:
                {
                    if constexpr (std::is_same_v<T, f64>)
                        return _value.Int32;
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

        enum class SliderValueType : u32
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
            _grabBackground { nullptr },
            _grabBorder { nullptr };
        /*std::unique_ptr<Button>
            _buttonLower { nullptr },
            _buttonHigher { nullptr };*/
        
        // Settings
        SliderValueType _valueType { SliderValueType::Int32 };
        bool _useSliderButtons { false };
        ElementSizing _grabSizingMode { ElementSizing::Percent };
        bool _isDraggingGrab { false };
        f32 _grabSliderPosition { 0.0f };
        glm::i32vec2 _previousGrabPosition { glm::i32vec2(0) };
        f32 _grabSizingFactor { 25.0f };

        //Design
        //std::unique_ptr<Texture> _grabImage { nullptr };
        std::vector<glm::vec4>
            _grabBackgroundColorEnabled { glm::vec4(1.0f, 0.0f, 0.0f, 1.0f) },
            _grabBackgroundColorClicked { glm::vec4(0.0f, 1.0f, 0.0f, 1.0f) },
            _grabBackgroundColorHover { glm::vec4(0.0f, 0.0f, 1.0f, 1.0f) },
            _grabBackgroundColorDisabled { glm::vec4(0.5f, .5f, 0.5f, 1.0f) },
            _grabBorderColorEnabled { glm::vec4(1.0f) },
            _grabBorderColorClicked { glm::vec4(1.0f) },
            _grabBorderColorHover { glm::vec4(1.0f) },
            _grabBorderColorDisabled { glm::vec4(1.0f) };
        ColorState
            _grabBackgroundColorState { ColorState::Enabled },
            _grabBorderColorState { ColorState::Enabled };
        std::vector<f32>
            _grabBackgroundDotRadiusTransfer,
            _grabBackgroundFogDensity,
            _grabBorderDotRadiusTransfer;

        Signal<>
            _onValueChanged,
            _onValueChangedAsync,
            _onValueSet,
            _onValueSetAsync;
    };
}