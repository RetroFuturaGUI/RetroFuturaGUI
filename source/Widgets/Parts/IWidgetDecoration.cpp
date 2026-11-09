#include "IWidgetDecoration.hpp"
#include <utility>

RetroFuturaGUI::IWidgetDecoration::IWidgetDecoration(Projection* projection, std::function<void()> onCornerRadiiChange)
    : _projection(projection), _onCornerRadiiChange(std::move(onCornerRadiiChange))
{

}

void RetroFuturaGUI::IWidgetDecoration::SetColors(std::span<const glm::vec4> colors, const ColorState state)
{
    _ownColors[static_cast<uSize>(state)].assign(colors.begin(), colors.end());
    setColors();
}

const std::vector<glm::vec4>& RetroFuturaGUI::IWidgetDecoration::GetShownColors(const ColorState state) const
{
    const std::vector<glm::vec4>& own { _ownColors[static_cast<uSize>(state)] };

    if(!own.empty())
        return own;

    //A part without a widget's flavors (a sub-part, a window's) goes from its own colors straight to its built-in ones.
    const std::vector<glm::vec4>* styled { _flavors ? StylingManager::FindColors(getFlavors(), _stylePart, state) : nullptr };

    if(styled)
        return *styled;

    const std::vector<glm::vec4>& builtIn { _builtInColors[static_cast<uSize>(state)] };

    if(!builtIn.empty() || ColorState::Enabled == state)
        return builtIn;

    return GetShownColors(ColorState::Enabled); //a state nothing sets, such as Selected, looks like Enabled
}

void RetroFuturaGUI::IWidgetDecoration::SetColorState(const ColorState state)
{
    _colorState = state;
    setColors();
}

glm::vec4 RetroFuturaGUI::IWidgetDecoration::GetCornerRadii() const
{
   return _cornerRadii;
}

void RetroFuturaGUI::IWidgetDecoration::SetCornerRadii(const glm::vec4& radii)
{
    _ownCornerRadii = radii;
    resolveCornerShape();
}

void RetroFuturaGUI::IWidgetDecoration::SetCutCorners(const CutCorners corners)
{
    _ownCutCorners = corners;
    resolveCornerShape();
}

RetroFuturaGUI::CutCorners RetroFuturaGUI::IWidgetDecoration::GetCutCorners() const
{
    return _cutCorners;
}

void RetroFuturaGUI::IWidgetDecoration::SetBuiltInColors(const StyleKey&, std::span<const glm::vec4> colors, const ColorState state)
{
    _builtInColors[static_cast<uSize>(state)].assign(colors.begin(), colors.end());
    setColors();
}

void RetroFuturaGUI::IWidgetDecoration::SetBuiltInCornerRadii(const StyleKey&, const glm::vec4& radii)
{
    _builtInCornerRadii = radii;
    resolveCornerShape();
}

const std::vector<glm::vec4>& RetroFuturaGUI::IWidgetDecoration::getOwnColors(const ColorState state) const
{
    return _ownColors[static_cast<uSize>(state)];
}

void RetroFuturaGUI::IWidgetDecoration::followStyles()
{
    if(StylingManager::GetGeneration() == _styleGeneration)
        return;

    restyle();
}

void RetroFuturaGUI::IWidgetDecoration::restyle()
{
    _styleGeneration = StylingManager::GetGeneration();
    setColors(); //the colors are handed on by reference, so they go again even when unchanged: a flavor's list may have moved.
    setShader();
    resolveCornerShape();
}

std::span<const u32> RetroFuturaGUI::IWidgetDecoration::getFlavors() const
{
    if(!_flavors)
        return {};

    return *_flavors;
}

void RetroFuturaGUI::IWidgetDecoration::setStyleSource(const std::vector<u32>* flavors, const StylePart part)
{
    _flavors = flavors;
    _stylePart = part;
    restyle();
}

void RetroFuturaGUI::IWidgetDecoration::resolveCornerShape()
{
    //A part without a widget's flavors (a sub-part, a window's) goes from its own values straight to its built-in ones.
    const glm::vec4 radii { _ownCornerRadii.value_or(_flavors ? StylingManager::FindCornerRadii(getFlavors(), _stylePart).value_or(_builtInCornerRadii) : _builtInCornerRadii) };
    const CutCorners cutCorners { _ownCutCorners.value_or(_flavors ? StylingManager::FindCutCorners(getFlavors(), _stylePart).value_or(CutCorners::None) : CutCorners::None) };

    if(radii == _cornerRadii && cutCorners == _cutCorners)
        return;

    _cornerRadii = radii;
    _cutCorners = cutCorners;
    setCornerShape();

    if(!_onCornerRadiiChange)
        return;

    _onCornerRadiiChange(); //what follows the corners follows their cut too.
}

void RetroFuturaGUI::IWidgetDecoration::ResetSetting(const StyleSetting setting)
{
    if(resetPartSetting(setting))
        return;

    switch(setting)
    {
        case StyleSetting::CornerRadii:
            _ownCornerRadii.reset();
            resolveCornerShape();
        break;
        case StyleSetting::CutCorners:
            _ownCutCorners.reset();
            resolveCornerShape();
        break;
        default:
            _shader.Reset(setting);
            setShader();
    }
}

bool RetroFuturaGUI::IWidgetDecoration::resetPartSetting(const StyleSetting)
{
    return false;
}