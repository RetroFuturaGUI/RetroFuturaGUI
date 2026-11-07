#include "Histogram.hpp"
#include "config.hpp"

RetroFuturaGUI::Histogram::Histogram(std::string_view name, Projection* projection, IHierarchyNode* parentWidget, const WidgetTypeID parentWidgetTypeID, GLFWwindow* parentWindow)
    : IRangedMultiValue(name, projection, parentWidget, parentWidgetTypeID, parentWindow)
{
    _widgetTypeID = WidgetTypeID::Histogram;
    _data.resize(1);
    _dataCount.resize(1);
}

void RetroFuturaGUI::Histogram::Draw()
{
    _background.Draw();
    _border.Draw();

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
    if(_dataCount.empty())
        return;

    if(!_dataCount.front())
    {
        _barWidth = 0.0f;
        return;
    }

    _barWidth = _orientation == Orientation::Horizontal
        ? _size.x / static_cast<f32>(_dataCount.front())
        : _size.y / static_cast<f32>(_dataCount.front());
    _barWidth -= _barInterstice;
}

f32 RetroFuturaGUI::Histogram::calculateBarHeight(const uSize index) const
{
    if(_data.empty())
        return 0.0f;

    if(!_data.front())
        return 0.0f;

    const f32 height { getDataValueAsF32(0, index) };
    f32 fraction { 0.0f > getRangeFraction(height) ? 0.0f : 1.0f < getRangeFraction(height) ? 1.0f : getRangeFraction(height) };

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
    if(_data.empty())
        return glm::vec3(0.0f);

    if(!_data.front())
        return glm::vec3(0.0f);

    f32 
        barHeight { calculateBarHeight(index) },
        barOffset { (_barWidth + _barInterstice) * static_cast<f32>(index) + _barInterstice + _barWidth * 0.5f };

        const glm::vec2 localOffset { _orientation == Orientation::Horizontal
        ? glm::vec2(-_size.x * 0.5f + barOffset, -_size.y * 0.5f + barHeight * 0.5f)
        : glm::vec2(-_size.x * 0.5f + barHeight * 0.5f, -_size.y * 0.5f + barOffset) };

    // Laid out untilted: the widget's frame turns the bars with it
    return glm::vec3(_position.x + localOffset.x, _position.y + localOffset.y, _position.z + 0.02f);
}

glm::vec3 RetroFuturaGUI::Histogram::calculateBarRotation() const
{
    return orientedRotation();
}

void RetroFuturaGUI::Histogram::drawGraph()
{
    if(_dataCount.empty())
        return;

    if(!_graph.IsShown())
        return;

    for(uSize i = 0; i < _dataCount.front(); ++i)
    {
        glm::vec3 
            barPosition = calculateBarPosition(i),
            barRotation = calculateBarRotation();
        f32 barHeight = calculateBarHeight(i);

        _graph.SetSize(glm::vec2(_barWidth, barHeight));
        _graph.SetPosition(barPosition);
        _graph.SetRotation(barRotation);
        _graph.Draw();
    }
}

void RetroFuturaGUI::Histogram::interact()
{

}