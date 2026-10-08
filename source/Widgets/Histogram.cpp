#include "Histogram.hpp"
#include "config.hpp"

RetroFuturaGUI::Histogram::Histogram(std::string_view name, Projection* projection, IHierarchyNode* parentWidget, const WidgetTypeID parentWidgetTypeID, GLFWwindow* parentWindow)
    : IRangedMultiValue(name, projection, parentWidget, parentWidgetTypeID, parentWindow)
{
    _widgetTypeID = WidgetTypeID::Histogram;
}

void RetroFuturaGUI::Histogram::Draw()
{
    interact();

    if (_background)
        _background->Draw();

    if (_border)
        _border->Draw();

    drawGraph();
    drawIndicator();
}

void RetroFuturaGUI::Histogram::SetBarInterstice(const f32 interstice)
{
    _barInterstice = interstice;
    alignElementsToTrack();
}

void RetroFuturaGUI::Histogram::calculateBarWidth()
{
    if(!_dataCount)
    {
        _barWidth = 0.0f;
        return;
    }

    _barWidth = _orientation == Orientation::Horizontal
        ? _size.x / static_cast<f32>(_dataCount)
        : _size.y / static_cast<f32>(_dataCount);
    _barWidth -= _barInterstice;
}

f32 RetroFuturaGUI::Histogram::calculateBarHeight(const uSize index) const
{
    if(!_data)
        return 0.0f;

    f32 height { 0.0f };
    
    switch(_valueType)
    {
        case PrimitiveTypeID::Bool:
            height = reinterpret_cast<const bool*>(_data)[index] ? 1.0f : 0.0f;
        break;
        case PrimitiveTypeID::Int8:
            height = static_cast<f32>(reinterpret_cast<const i8*>(_data)[index]);
        break;
        case PrimitiveTypeID::Int16:
            height = static_cast<f32>(reinterpret_cast<const i16*>(_data)[index]);
        break;
        case PrimitiveTypeID::Int32:
            height = static_cast<f32>(reinterpret_cast<const i32*>(_data)[index]);
        break;
        case PrimitiveTypeID::Int64:
            height = static_cast<f32>(reinterpret_cast<const i64*>(_data)[index]);
        break;
        case PrimitiveTypeID::UInt8:
            height = static_cast<f32>(reinterpret_cast<const u8*>(_data)[index]);
        break;
        case PrimitiveTypeID::UInt16:
            height = static_cast<f32>(reinterpret_cast<const u16*>(_data)[index]);
        break;
        case PrimitiveTypeID::UInt32:
            height = static_cast<f32>(reinterpret_cast<const u32*>(_data)[index]);
        break;
        case PrimitiveTypeID::UInt64:
            height = static_cast<f32>(reinterpret_cast<const u64*>(_data)[index]);
        break;
        case PrimitiveTypeID::Float64:
            height = saturatingCast<f32>(reinterpret_cast<const f64*>(_data)[index]); // beyond f32's range a plain cast is undefined behavior
        break;
        default: // Float32
            height = reinterpret_cast<const f32*>(_data)[index];
        break;
    }
    
    f32 fraction { getRangeFraction(height) < 0.0f ? 0.0f : getRangeFraction(height) > 1.0f ? 1.0f : getRangeFraction(height) };

    if(_orientation == Orientation::Horizontal)
        return _size.y * fraction;
    else
        return _size.x * fraction;
}

void RetroFuturaGUI::Histogram::alignElementsToTrack()
{
    calculateBarWidth();
}

glm::vec3 RetroFuturaGUI::Histogram::calculateBarPosition(const uSize index) const
{
    if(!_data)
        return glm::vec3(0.0f);

    f32 
        barHeight { calculateBarHeight(index) },
        barOffset { (_barWidth + _barInterstice) * static_cast<f32>(index) + _barInterstice + _barWidth * 0.5f };

        const glm::vec2 localOffset { _orientation == Orientation::Horizontal
        ? glm::vec2(-_size.x * 0.5f + barOffset, -_size.y * 0.5f + barHeight * 0.5f)
        : glm::vec2(-_size.x * 0.5f + barHeight * 0.5f, -_size.y * 0.5f + barOffset) };

    const f32 radians { glm::radians(_rotation.z) };
    const glm::vec2 rotatedOffset
    (
        localOffset.x * cos(radians) - localOffset.y * sin(radians),
        localOffset.x * sin(radians) + localOffset.y * cos(radians)
    );

    return glm::vec3(_position.x + rotatedOffset.x, _position.y + rotatedOffset.y, _position.z + 0.02f);
}

glm::vec3 RetroFuturaGUI::Histogram::calculateBarRotation() const
{
    return orientedRotation(_rotation);
}

void RetroFuturaGUI::Histogram::drawGraph()
{
    if(!_useGraph || !_graph)
        return;

    for(uSize i = 0; i < _dataCount; ++i)
    {
        glm::vec3 
            barPosition = calculateBarPosition(i),
            barRotation = calculateBarRotation();
        f32 barHeight = calculateBarHeight(i);

        _graph->SetSize(glm::vec3(_barWidth, barHeight, 1.0f));
        _graph->SetPosition(barPosition);
        _graph->SetRotation(barRotation);
        _graph->Draw();
    }
}

void RetroFuturaGUI::Histogram::interact()
{

}