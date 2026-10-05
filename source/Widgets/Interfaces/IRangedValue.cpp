#include "IRangedValue.hpp"
#include "Rectangle.hpp"
#include <memory>


RetroFuturaGUI::IRangedValue::IRangedValue(std::string_view name, Projection* projection, IHierarchyNode* parentWidget, const WidgetTypeID parentWidgetTypeID, GLFWwindow* parentWindow)
    : IWidget(name, projection, parentWidget, parentWidgetTypeID, parentWindow)
{
    _background = std::make_unique<Rectangle>(projection);
    _border = std::make_unique<Rectangle>(projection);
    _indicatorBackground = std::make_unique<Rectangle>(projection);
    _indicatorBorder = std::make_unique<Rectangle>(projection);

    if (_background)
        _background->SetRectangleMode(RectangleMode::Plane);

    if (_border)
        _border->SetRectangleMode(RectangleMode::Border);

    if (_indicatorBackground)
        _indicatorBackground->SetRectangleMode(RectangleMode::Plane);

    if (_indicatorBorder)
        _indicatorBorder->SetRectangleMode(RectangleMode::Border);

    _track = _background.get();
    _elementProjection = projection;
    _useIndicator = true;
}

void RetroFuturaGUI::IRangedValue::SetPosition(const glm::vec3& position)
{
    IWidget::SetPosition(position);

    if (_background)
        _background->SetPosition(position);

    if (_border)
        _border->SetPosition(position + glm::vec3(0.0f, 0.0f, 0.01f));

    alignElementsToTrack();
}

void RetroFuturaGUI::IRangedValue::SetSize(const glm::vec3& size)
{
    IWidget::SetSize(size);

    const glm::vec3 trackSize { _orientation == Orientation::Vertical
        ? glm::vec3(size.y, size.x, size.z)
        : size };

    if (_background)
        _background->SetSize(trackSize);

    if (_border)
        _border->SetSize(trackSize);

    setIndicatorSize();
    alignElementsToTrack();
}

void RetroFuturaGUI::IRangedValue::SetRotation(const glm::vec3& rotation)
{
    IWidget::SetRotation(rotation);
    const glm::vec3 trackRotation { orientedRotation(rotation) }; //use track's orientation

    if (_background)
        _background->SetRotation(trackRotation);

    if (_border)
        _border->SetRotation(trackRotation);

    alignElementsToTrack();
}

void RetroFuturaGUI::IRangedValue::SetOrientation(const Orientation orientation)
{
    _orientation = orientation;
    SetSize(_size);
    SetRotation(_rotation);
}

void RetroFuturaGUI::IRangedValue::SetTrackDirection(const TrackDirection direction)
{
    _trackDirection = direction;
    alignElementsToTrack();
}

void RetroFuturaGUI::IRangedValue::SetIndicatorSize(const glm::vec2& size, const ElementSizing sizingMode)
{
    _indicatorSize = size;
    _indicatorSizingMode = sizingMode;
    setIndicatorSize();
    alignElementsToTrack();
}

void RetroFuturaGUI::IRangedValue::EnableIndicator(const bool value)
{
    _useIndicator = value;

    if(_useIndicator && !_indicatorBackground)
    {
        _indicatorBackground = std::make_unique<Rectangle>(_elementProjection);
        _indicatorBackground->SetRectangleMode(RectangleMode::Plane);
        setIndicatorBackgroundColors();
    }

    alignElementsToTrack();
}

void RetroFuturaGUI::IRangedValue::SetIndicatorType(const IndicatorType type)
{
    _indicatorType = type;
    const bool isCircle { _indicatorType == IndicatorType::Circle };

    if(_indicatorBackground)
    {
        if(isCircle)
        {
            _indicatorBackground->SetShaderFeatures(RoundedCorners);
            _indicatorBackground->SetCornerRadii(glm::vec4(_indicatorBackground->GetSize().x * 0.5f));
        }
        else
        {
            _indicatorBackground->SetShaderFeatures(0);
            _indicatorBackground->SetCornerRadii(glm::vec4(0.0f));
        }
    }

    if(_indicatorBorder)
    {
        if(isCircle)
        {
            _indicatorBorder->SetShaderFeatures(RoundedCorners);
            _indicatorBorder->SetCornerRadii(glm::vec4(_indicatorBorder->GetSize().x * 0.5f));
        }
        else
        {
            _indicatorBorder->SetShaderFeatures(0);
            _indicatorBorder->SetCornerRadii(glm::vec4(0.0f));
        }
    }
}

void RetroFuturaGUI::IRangedValue::SetIndicatorBackgroundColors(std::span<glm::vec4> colors, const ColorState state)
{
    switch(state)
    {
        case ColorState::Clicked:
            _indicatorBackgroundColorClicked.assign(colors.begin(), colors.end());
        break;
        case ColorState::Disabled:
            _indicatorBackgroundColorDisabled.assign(colors.begin(), colors.end());
        break;
        case ColorState::Hover:
            _indicatorBackgroundColorHover.assign(colors.begin(), colors.end());
        break;
        default: // Enabled
            _indicatorBackgroundColorEnabled.assign(colors.begin(), colors.end());
    }

    setIndicatorBackgroundColors();
}

void RetroFuturaGUI::IRangedValue::SetIndicatorBorderColors(std::span<glm::vec4> colors, const ColorState state)
{
    switch(state)
    {
        case ColorState::Clicked:
            _indicatorBorderColorClicked.assign(colors.begin(), colors.end());
        break;
        case ColorState::Disabled:
            _indicatorBorderColorDisabled.assign(colors.begin(), colors.end());
        break;
        case ColorState::Hover:
            _indicatorBorderColorHover.assign(colors.begin(), colors.end());
        break;
        default: // Enabled
            _indicatorBorderColorEnabled.assign(colors.begin(), colors.end());
    }

    setIndicatorBorderColors();
}

void RetroFuturaGUI::IRangedValue::SetIndicatorCornerRadii(const glm::vec4& radii)
{
    if(_indicatorBackground)
        _indicatorBackground->SetCornerRadii(radii);

    if(_indicatorBorder)
        _indicatorBorder->SetCornerRadii(radii);
}

void RetroFuturaGUI::IRangedValue::SetIndicatorBackgroundFillType(const FillType fillType)
{
    if(_indicatorBackground)
        _indicatorBackground->SetFillType(fillType);
}

void RetroFuturaGUI::IRangedValue::SetIndicatorBackgroundGradientOffset(const f32 gradientOffset)
{
    if(_indicatorBackground)
        _indicatorBackground->SetGradientOffset(gradientOffset);
}

void RetroFuturaGUI::IRangedValue::SetIndicatorBackgroundGradientAnimationSpeed(const f32 animationSpeed)
{
    if(_indicatorBackground)
        _indicatorBackground->SetGradientAnimationSpeed(animationSpeed);
}

void RetroFuturaGUI::IRangedValue::SetIndicatorBackgroundGradientDegree(const f32 degree)
{
    if(_indicatorBackground)
        _indicatorBackground->SetGradientDegree(degree);
}

void RetroFuturaGUI::IRangedValue::SetIndicatorBackgroundGradientRotationSpeed(const f32 rotationSpeed)
{
    if(_indicatorBackground)
        _indicatorBackground->SetGradientRotationSpeed(rotationSpeed);
}

void RetroFuturaGUI::IRangedValue::SetIndicatorWindowBackgroundImageTextureID(const u32 textureID)
{
    if(_indicatorBackground)
        _indicatorBackground->SetWindowBackgroundImageTextureID(textureID);
}

void RetroFuturaGUI::IRangedValue::SetIndicatorBackgroundPrimaryRasterColor(const glm::vec4& color)
{
    if(_indicatorBackground)
        _indicatorBackground->SetPrimaryRasterColor(color);
}

void RetroFuturaGUI::IRangedValue::SetIndicatorBackgroundDotDistance(const f32 distance)
{
    if(_indicatorBackground)
        _indicatorBackground->SetDotDistance(distance);
}

void RetroFuturaGUI::IRangedValue::SetIndicatorBackgroundRasterDegree(const f32 degree)
{
    if(_indicatorBackground)
        _indicatorBackground->SetRasterDegree(degree);
}

void RetroFuturaGUI::IRangedValue::SetIndicatorBackgroundPrimaryRasterWidthTransfer(std::span<f32> widthTransfer)
{
    _indicatorBackgroundPrimaryRasterWidthTransfer.assign(widthTransfer.begin(), widthTransfer.end());

    if(_indicatorBackground)
        _indicatorBackground->SetPrimaryRasterWidthTransfer(_indicatorBackgroundPrimaryRasterWidthTransfer);
}

void RetroFuturaGUI::IRangedValue::SetIndicatorBackgroundDotTransparencyTransfer(const f32 transparencyTransfer)
{
    if(_indicatorBackground)
        _indicatorBackground->SetDotTransparencyTransfer(transparencyTransfer);
}

void RetroFuturaGUI::IRangedValue::SetIndicatorBackgroundRasterAnimationSpeed(const f32 animationSpeed)
{
    if(_indicatorBackground)
        _indicatorBackground->SetRasterAnimationSpeed(animationSpeed);
}

void RetroFuturaGUI::IRangedValue::SetIndicatorBackgroundFogAlpha(const f32 alpha)
{
    if(_indicatorBackground)
        _indicatorBackground->SetFogAlpha(alpha);
}

void RetroFuturaGUI::IRangedValue::SetIndicatorBackgroundFogSpeed(const f32 speed)
{
    if(_indicatorBackground)
        _indicatorBackground->SetFogSpeed(speed);
}

void RetroFuturaGUI::IRangedValue::SetIndicatorBackgroundFogDensity(std::span<f32> density)
{
    _indicatorBackgroundFogDensity.assign(density.begin(), density.end());

    if(_indicatorBackground)
        _indicatorBackground->SetFogDensity(_indicatorBackgroundFogDensity);
}

void RetroFuturaGUI::IRangedValue::SetIndicatorBackgroundFogClearing(const f32 clearing)
{
    if(_indicatorBackground)
        _indicatorBackground->SetFogClearing(clearing);
}

void RetroFuturaGUI::IRangedValue::SetIndicatorBorderFillType(const FillType fillType)
{
    if(_indicatorBorder)
        _indicatorBorder->SetFillType(fillType);
}

void RetroFuturaGUI::IRangedValue::SetIndicatorBorderGradientOffset(const f32 gradientOffset)
{
    if(_indicatorBorder)
        _indicatorBorder->SetGradientOffset(gradientOffset);
}

void RetroFuturaGUI::IRangedValue::SetIndicatorBorderGradientAnimationSpeed(const f32 animationSpeed)
{
    if(_indicatorBorder)
        _indicatorBorder->SetGradientAnimationSpeed(animationSpeed);
}

void RetroFuturaGUI::IRangedValue::SetIndicatorBorderGradientDegree(const f32 degree)
{
    if(_indicatorBorder)
        _indicatorBorder->SetGradientDegree(degree);
}

void RetroFuturaGUI::IRangedValue::SetIndicatorBorderGradientRotationSpeed(const f32 rotationSpeed)
{
    if(_indicatorBorder)
        _indicatorBorder->SetGradientRotationSpeed(rotationSpeed);
}

void RetroFuturaGUI::IRangedValue::SetIndicatorWindowBorderImageTextureID(const u32 textureID)
{
    if(_indicatorBorder)
        _indicatorBorder->SetWindowBackgroundImageTextureID(textureID);
}

void RetroFuturaGUI::IRangedValue::SetIndicatorBorderPrimaryRasterColor(const glm::vec4& color)
{
    if(_indicatorBorder)
        _indicatorBorder->SetPrimaryRasterColor(color);
}

void RetroFuturaGUI::IRangedValue::SetIndicatorBorderDotDistance(const f32 distance)
{
    if(_indicatorBorder)
        _indicatorBorder->SetDotDistance(distance);
}

void RetroFuturaGUI::IRangedValue::SetIndicatorBorderRasterDegree(const f32 degree)
{
    if(_indicatorBorder)
        _indicatorBorder->SetRasterDegree(degree);
}

void RetroFuturaGUI::IRangedValue::SetIndicatorBorderPrimaryRasterWidthTransfer(std::span<f32> widthTransfer)
{
    _indicatorBorderPrimaryRasterWidthTransfer.assign(widthTransfer.begin(), widthTransfer.end());

    if(_indicatorBorder)
        _indicatorBorder->SetPrimaryRasterWidthTransfer(_indicatorBorderPrimaryRasterWidthTransfer);
}

void RetroFuturaGUI::IRangedValue::SetIndicatorBorderDotTransparencyTransfer(const f32 transparencyTransfer)
{
    if(_indicatorBorder)
        _indicatorBorder->SetDotTransparencyTransfer(transparencyTransfer);
}

void RetroFuturaGUI::IRangedValue::SetIndicatorBorderRasterAnimationSpeed(const f32 animationSpeed)
{
    if(_indicatorBorder)
        _indicatorBorder->SetRasterAnimationSpeed(animationSpeed);
}

void RetroFuturaGUI::IRangedValue::SetIndicatorBorderWidth(const f32 borderWidth)
{
    if(_indicatorBorder)
        _indicatorBorder->SetBorderWidth(borderWidth);
}

void RetroFuturaGUI::IRangedValue::SetIndicatorBorderGaps(std::span<BorderGap> gaps)
{
    if(_indicatorBorder)
        _indicatorBorder->SetBorderGaps(gaps);
}

void RetroFuturaGUI::IRangedValue::EnableGraph(const bool value)
{
    _useGraph = value;

    if(_useGraph && !_graph)
    {
        _graph = std::make_unique<Rectangle>(_elementProjection);
        _graph->SetRectangleMode(RectangleMode::Plane);
        setGraphColorsApply();
    }

    alignElementsToTrack();
}

void RetroFuturaGUI::IRangedValue::SetGraphMode(const GraphMode mode)
{
    _graphMode = mode;

    if(!_graph)
        return;

    const u32 features = _graphMode == GraphMode::Wave
        ? (_graph->GetShaderFeatures() | Wave)
        : (_graph->GetShaderFeatures() & ~Wave);

    _graph->SetShaderFeatures(features);
    // TODO: GraphMode::Wave has no distinct rendering path yet — it draws identically to Bar.
}

void RetroFuturaGUI::IRangedValue::SetGraphColors(std::span<glm::vec4> colors, const ColorState state)
{
    switch(state)
    {
        case ColorState::Disabled:
            _graphColorDisabled.assign(colors.begin(), colors.end());
        break;
        case ColorState::Hover:
            _graphColorHover.assign(colors.begin(), colors.end());
        break;
        case ColorState::Clicked:
            _graphColorClicked.assign(colors.begin(), colors.end());
        break;
        default: // Enabled
            _graphColorEnabled.assign(colors.begin(), colors.end());
    }

    setGraphColorsApply();
}

void RetroFuturaGUI::IRangedValue::SetGraphCornerRadii(const glm::vec4& radii)
{
    if(_graph)
        _graph->SetCornerRadii(radii);
}

void RetroFuturaGUI::IRangedValue::SetGraphWidth(const f32 width)
{
    _graphWidth = width;
    alignElementsToTrack();
}

void RetroFuturaGUI::IRangedValue::SetGraphFillType(const FillType fillType)
{
    if(_graph)
        _graph->SetFillType(fillType);
}

void RetroFuturaGUI::IRangedValue::SetGraphGradientOffset(const f32 gradientOffset)
{
    if(_graph)
        _graph->SetGradientOffset(gradientOffset);
}

void RetroFuturaGUI::IRangedValue::SetGraphGradientAnimationSpeed(const f32 animationSpeed)
{
    if(_graph)
        _graph->SetGradientAnimationSpeed(animationSpeed);
}

void RetroFuturaGUI::IRangedValue::SetGraphGradientDegree(const f32 degree)
{
    if(_graph)
        _graph->SetGradientDegree(degree);
}

void RetroFuturaGUI::IRangedValue::SetGraphGradientRotationSpeed(const f32 rotationSpeed)
{
    if(_graph)
        _graph->SetGradientRotationSpeed(rotationSpeed);
}

void RetroFuturaGUI::IRangedValue::SetGraphWindowBackgroundImageTextureID(const u32 textureID)
{
    if(_graph)
        _graph->SetWindowBackgroundImageTextureID(textureID);
}

void RetroFuturaGUI::IRangedValue::SetGraphPrimaryRasterColor(const glm::vec4& color)
{
    if(_graph)
        _graph->SetPrimaryRasterColor(color);
}

void RetroFuturaGUI::IRangedValue::SetGraphDotDistance(const f32 distance)
{
    if(_graph)
        _graph->SetDotDistance(distance);
}

void RetroFuturaGUI::IRangedValue::SetGraphRasterDegree(const f32 degree)
{
    if(_graph)
        _graph->SetRasterDegree(degree);
}

void RetroFuturaGUI::IRangedValue::SetGraphPrimaryRasterWidthTransfer(std::span<f32> widthTransfer)
{
    _graphPrimaryRasterWidthTransfer.assign(widthTransfer.begin(), widthTransfer.end());

    if(_graph)
        _graph->SetPrimaryRasterWidthTransfer(_graphPrimaryRasterWidthTransfer);
}

void RetroFuturaGUI::IRangedValue::SetGraphDotTransparencyTransfer(const f32 transparencyTransfer)
{
    if(_graph)
        _graph->SetDotTransparencyTransfer(transparencyTransfer);
}

void RetroFuturaGUI::IRangedValue::SetGraphRasterAnimationSpeed(const f32 animationSpeed)
{
    if(_graph)
        _graph->SetRasterAnimationSpeed(animationSpeed);
}

void RetroFuturaGUI::IRangedValue::SetGraphFogAlpha(const f32 alpha)
{
    if(_graph)
        _graph->SetFogAlpha(alpha);
}

void RetroFuturaGUI::IRangedValue::SetGraphFogSpeed(const f32 speed)
{
    if(_graph)
        _graph->SetFogSpeed(speed);
}

void RetroFuturaGUI::IRangedValue::SetGraphFogDensity(std::span<f32> density)
{
    _graphFogDensity.assign(density.begin(), density.end());

    if(_graph)
        _graph->SetFogDensity(_graphFogDensity);
}

void RetroFuturaGUI::IRangedValue::SetGraphFogClearing(const f32 clearing)
{
    if(_graph)
        _graph->SetFogClearing(clearing);
}

glm::vec3 RetroFuturaGUI::IRangedValue::orientedRotation(const glm::vec3& rotation) const
{
    return _orientation == Orientation::Vertical
        ? rotation + glm::vec3(0.0f, 0.0f, 90.0f)
        : rotation;
}

f64 RetroFuturaGUI::IRangedValue::toF64(const PrimitiveUnion value) const
{
    switch(_valueType)
    {
        case PrimitiveTypeID::Int8:
            return static_cast<f64>(value.Int8);
        case PrimitiveTypeID::Int16:
            return static_cast<f64>(value.Int16);
        case PrimitiveTypeID::Int32:
            return static_cast<f64>(value.Int32);
        case PrimitiveTypeID::Int64:
            return static_cast<f64>(value.Int64);
        case PrimitiveTypeID::UInt8:
            return static_cast<f64>(value.UInt8);
        case PrimitiveTypeID::UInt16:
            return static_cast<f64>(value.UInt16);
        case PrimitiveTypeID::UInt32:
            return static_cast<f64>(value.UInt32);
        case PrimitiveTypeID::UInt64:
            return static_cast<f64>(value.UInt64);
        case PrimitiveTypeID::Float32:
            return static_cast<f64>(value.Float32);
        case PrimitiveTypeID::Float64:
            return value.Float64;
        default: // Bool
            return value.Bool ? 1.0 : 0.0;
    }
}

f32 RetroFuturaGUI::IRangedValue::getRangeFraction(const f64 value) const
{
    // A bool's range is always false..true, whatever its bounds hold
    if(_valueType == PrimitiveTypeID::Bool)
        return 0.0 != value ? 1.0f : 0.0f;

    // f64 rather than f32: with a large minimum, such as an absolute timestamp, f32 collapses nearby values into one fraction
    const f64
        minValue { toF64(_minValue) },
        range { toF64(_maxValue) - minValue };

    if(0.0 == range)
        return 0.0f;

    return static_cast<f32>((value - minValue) / range);
}

f32 RetroFuturaGUI::IRangedValue::toTrackFraction(const f32 rangeFraction) const
{
    return _trackDirection == TrackDirection::Inverted ? 1.0f - rangeFraction : rangeFraction;
}

void RetroFuturaGUI::IRangedValue::setIndicatorPosition(const f32 trackFraction)
{
    if(!_track)
        return;

    if(!_indicatorBackground)
        return;

    const f32
        borderInset { _border ? _border->GetBorderWidth() * 2.0f : 0.0f },
        trackLength { glm::max(_track->GetSize().x - borderInset, 0.0f) },
        indicatorLength { _indicatorBackground->GetSize().x },
        travelRange { 0.0f < trackLength - indicatorLength ? trackLength - indicatorLength : 0.0f },
        indicatorSliderPosition { indicatorLength * 0.5f + trackFraction * travelRange };

    // The indicator always travels along the track's local x-axis.
    // Rotate that local offset by the track's rotation to place it correctly in world space.
    const glm::vec2 localOffset(indicatorSliderPosition - trackLength * 0.5f, 0.0f);
    const f32 radians = glm::radians(_track->GetRotation().z);
    const glm::vec2 rotatedOffset
    (
        localOffset.x * cos(radians) - localOffset.y * sin(radians),
        localOffset.x * sin(radians) + localOffset.y * cos(radians)
    );

    const glm::vec3 position
    (
        _track->GetPosition().x + rotatedOffset.x,
        _track->GetPosition().y + rotatedOffset.y,
        _track->GetPosition().z + 0.02f
    );

    _indicatorBackground->SetPosition(position);
    _indicatorBackground->SetRotation(_track->GetRotation());

    if(_indicatorBorder)
    {
        _indicatorBorder->SetPosition(position + glm::vec3(0.0f, 0.0f, 0.01f));
        _indicatorBorder->SetRotation(_track->GetRotation());
    }
}

void RetroFuturaGUI::IRangedValue::setGraphSize(const f32 trackFraction)
{
    if(!_track)
        return;

    if(!_graph)
        return;

    const f32
        trackWidth { _track->GetSize().x },
        trackHeight { _track->GetSize().y },
        graphWidth { trackWidth * trackFraction },
        graphHeight { 0.0f < _graphWidth ? (_graphWidth < trackHeight ? _graphWidth : trackHeight) : trackHeight };

    _graph->SetSize(glm::vec2(graphWidth, graphHeight));
}

void RetroFuturaGUI::IRangedValue::setGraphPosition(const f32 trackFraction)
{
    if(!_track)
        return;

    if(!_graph)
        return;

    setGraphSize(trackFraction);

    const f32
        trackWidth { _track->GetSize().x },
        graphWidth { _graph->GetSize().x };

    // The graph always grows along the track's local x-axis, anchored to its left edge.
    const glm::vec2 localOffset(graphWidth * 0.5f - trackWidth * 0.5f, 0.0f);
    const f32 radians = glm::radians(_track->GetRotation().z);
    const glm::vec2 rotatedOffset
    (
        localOffset.x * cos(radians) - localOffset.y * sin(radians),
        localOffset.x * sin(radians) + localOffset.y * cos(radians)
    );

    const glm::vec3 position
    (
        _track->GetPosition().x + rotatedOffset.x,
        _track->GetPosition().y + rotatedOffset.y,
        _track->GetPosition().z + 0.02f
    );

    _graph->SetPosition(position);
    _graph->SetRotation(_track->GetRotation());
}

void RetroFuturaGUI::IRangedValue::setIndicatorColors(const ColorState state)
{
    _indicatorBackgroundColorState = state;
    _indicatorBorderColorState = state;
    setIndicatorBackgroundColors();
    setIndicatorBorderColors();
}

void RetroFuturaGUI::IRangedValue::setGraphColors(const ColorState state)
{
    _graphColorState = state;
    setGraphColorsApply();
}

void RetroFuturaGUI::IRangedValue::setIndicatorBackgroundColors()
{
    if(!_indicatorBackground)
        return;

    switch(_indicatorBackgroundColorState)
    {
        case ColorState::Enabled:
            _indicatorBackground->SetColors(_indicatorBackgroundColorEnabled);
        break;
        case ColorState::Clicked:
            _indicatorBackground->SetColors(_indicatorBackgroundColorClicked);
        break;
        case ColorState::Hover:
            _indicatorBackground->SetColors(_indicatorBackgroundColorHover);
        break;
        default: //Disabled
            _indicatorBackground->SetColors(_indicatorBackgroundColorDisabled);
    }
}

void RetroFuturaGUI::IRangedValue::setIndicatorBorderColors()
{
    if(!_indicatorBorder)
        return;

    switch(_indicatorBorderColorState)
    {
        case ColorState::Enabled:
            _indicatorBorder->SetColors(_indicatorBorderColorEnabled);
        break;
        case ColorState::Clicked:
            _indicatorBorder->SetColors(_indicatorBorderColorClicked);
        break;
        case ColorState::Hover:
            _indicatorBorder->SetColors(_indicatorBorderColorHover);
        break;
        default: //Disabled
            _indicatorBorder->SetColors(_indicatorBorderColorDisabled);
    }
}

void RetroFuturaGUI::IRangedValue::setGraphColorsApply()
{
    if(!_graph)
        return;

    switch(_graphColorState)
    {
        case ColorState::Enabled:
            _graph->SetColors(_graphColorEnabled);
        break;
        case ColorState::Clicked:
            _graph->SetColors(_graphColorClicked);
        break;
        case ColorState::Hover:
            _graph->SetColors(_graphColorHover);
        break;
        default: //Disabled
            _graph->SetColors(_graphColorDisabled);
    }
}

void RetroFuturaGUI::IRangedValue::drawIndicator()
{
    if(!_useIndicator)
        return;

    if(_indicatorBackground)
        _indicatorBackground->Draw();

    if(_indicatorBorder)
        _indicatorBorder->Draw();
}

void RetroFuturaGUI::IRangedValue::drawGraph()
{
    if(!_useGraph || !_graph)
        return;

    _graph->Draw(); // TODO: GraphMode::Wave currently draws identically to GraphMode::Bar — wave geometry/shader not implemented yet.
}

void RetroFuturaGUI::IRangedValue::setIndicatorSize()
{
    if(!_background)
        return;

    // The border shader insets the frame uBorderWidth inward from every edge, inside the track's own bounds,
    // so the area the indicator can occupy without painting over it is the track shrunk by that much per side.
    const glm::vec2 trackSize { _background->GetSize() };
    const f32 borderInset { _border ? _border->GetBorderWidth() * 2.0f : 0.0f };
    const glm::vec2 innerSize { glm::max(trackSize.x - borderInset, 0.0f), glm::max(trackSize.y - borderInset, 0.0f) };

    // Both axes are the track's own - x along its length, y across its thickness - and the indicator carries the
    // track's rotation, so this holds for Vertical without a per-orientation case.
    const glm::vec2 resolved { _indicatorSizingMode == ElementSizing::Percent
        ? glm::vec2(innerSize.x * _indicatorSize.x * 0.01f, innerSize.y * _indicatorSize.y * 0.01f)
        : glm::vec2(glm::min(_indicatorSize.x, innerSize.x), glm::min(_indicatorSize.y, innerSize.y)) };

    const glm::vec3 size { resolved.x, resolved.y, 0.01f };

    if(_indicatorBackground)
        _indicatorBackground->SetSize(size);

    if(_indicatorBorder)
        _indicatorBorder->SetSize(size);

    SetIndicatorType(_indicatorType);
}