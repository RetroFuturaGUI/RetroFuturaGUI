#ifdef RETROFUTURAGUI_MODULES
module;
#include "ModuleExternals.hpp"
module RetroFuturaGUI;
#else
#include "Transition.hpp"
#endif
#include <algorithm>
#include <cmath>

void RetroFuturaGUI::Transitions::SetDefault(const TransitionProperty properties, const Transition& transition)
{
    Override(_defaults, properties, transition);
}

RetroFuturaGUI::Transition RetroFuturaGUI::Transitions::GetDefault(const TransitionProperty property)
{
    return select(_defaults, property).value_or(Transition {});
}

void RetroFuturaGUI::Transitions::Override(TransitionSettings& settings, const TransitionProperty properties, const Transition& transition)
{
    for(const TransitionProperty property : AllProperties)
    {
        if(!HasTransitionProperty(properties, property))
            continue;

        select(settings, property) = transition;
    }
}

void RetroFuturaGUI::Transitions::Reset(TransitionSettings& settings, const TransitionProperty properties)
{
    for(const TransitionProperty property : AllProperties)
    {
        if(!HasTransitionProperty(properties, property))
            continue;

        select(settings, property).reset();
    }
}

bool RetroFuturaGUI::Transitions::IsInstant()
{
    return _areInstant;
}

std::optional<RetroFuturaGUI::Transition>& RetroFuturaGUI::Transitions::select(TransitionSettings& settings, const TransitionProperty property)
{
    switch(property)
    {
        case TransitionProperty::CornerRadii:
            return settings._CornerRadii;
        case TransitionProperty::Position:
            return settings._Position;
        case TransitionProperty::Size:
            return settings._Size;
        case TransitionProperty::Rotation:
            return settings._Rotation;
        case TransitionProperty::Opacity:
            return settings._Opacity;
        case TransitionProperty::BorderGap:
            return settings._BorderGap;
        case TransitionProperty::Effects:
            return settings._Effects;
        case TransitionProperty::Colors:
        default:
            return settings._Colors;
    }
}

const std::optional<RetroFuturaGUI::Transition>& RetroFuturaGUI::Transitions::select(const TransitionSettings& settings, const TransitionProperty property)
{
    return select(const_cast<TransitionSettings&>(settings), property);
}

RetroFuturaGUI::Transitions::InstantScope::InstantScope(const bool active)
    : _wasInstant(_areInstant)
{
    _areInstant = _wasInstant || active;
}

RetroFuturaGUI::Transitions::InstantScope::~InstantScope()
{
    _areInstant = _wasInstant;
}

const std::optional<RetroFuturaGUI::Transition>& RetroFuturaGUI::Transitions::GetOverride(const TransitionSettings& settings, const TransitionProperty property)
{
    return select(settings, property);
}

RetroFuturaGUI::Transition RetroFuturaGUI::Transitions::Resolve(const TransitionSettings& settings, const TransitionProperty property)
{
    if(_areInstant)
        return Transition {};

    return select(settings, property).value_or(GetDefault(property));
}

f32 RetroFuturaGUI::Transitions::GetProgress(const std::chrono::steady_clock::time_point start, const std::chrono::steady_clock::time_point now, const Transition& transition)
{
    if(0 >= transition._Duration.count())
        return 1.0f;

    const f64 elapsed { std::chrono::duration<f64, std::milli>(now - start).count() };
    const f64 linear { std::clamp(elapsed / static_cast<f64>(transition._Duration.count()), 0.0, 1.0) };
    return Ease(static_cast<f32>(linear), transition._Easing);
}

f32 RetroFuturaGUI::Transitions::Ease(const f32 progress, const Easing easing)
{
    const f32 linear { std::clamp(progress, 0.0f, 1.0f) };

    switch(easing)
    {
        case Easing::EaseIn:
            return linear * linear * linear;
        case Easing::EaseOut:
            return 1.0f - std::pow(1.0f - linear, 3.0f);
        case Easing::EaseInOut:
            return 0.5f > linear ? 4.0f * linear * linear * linear : 1.0f - std::pow(2.0f - 2.0f * linear, 3.0f) * 0.5f;
        case Easing::Linear:
        default:
            return linear;
    }
}

glm::vec4 RetroFuturaGUI::Transitions::BlendColor(const glm::vec4& from, const glm::vec4& to, const f32 amount)
{
    const f32 alpha { glm::mix(from.a, to.a, amount) };

    if(0.0f >= alpha)
        return glm::vec4(glm::mix(glm::vec3(from), glm::vec3(to), amount), 0.0f); //nothing shows, so no color to weigh

    const glm::vec3 premultiplied { glm::mix(glm::vec3(from) * from.a, glm::vec3(to) * to.a, amount) };
    return glm::vec4(premultiplied / alpha, alpha);
}

void RetroFuturaGUI::Transitions::ResampleStops(std::span<const glm::vec4> colors, const uSize stopCount, std::vector<glm::vec4>& stops)
{
    stops.clear();

    if(colors.empty())
        return;

    const uSize colorCount { colors.size() };
    stops.reserve(stopCount);

    //Stop j sits at j / stopCount along the gradient, where the shader mixes the two colors around it
    for(uSize stop { 0 }; stop < stopCount; ++stop)
    {
        const f64 position { static_cast<f64>(stop) * static_cast<f64>(colorCount) / static_cast<f64>(stopCount) };
        const uSize
            first { std::min(static_cast<uSize>(position), colorCount - 1) },
            second { (first + 1) % colorCount };

        stops.push_back(glm::mix(colors[first], colors[second], static_cast<f32>(position - static_cast<f64>(first))));
    }
}