#include "Caret.hpp"
#include <array>
#include <algorithm>

RetroFuturaGUI::Caret::Caret(Projection* projection)
    : _plane(projection)
{
    _plane.SetFillType(FillType::Solid);
    _colors.SetBuiltIn(std::array { glm::vec4(1.0f) }, ColorState::Enabled);
    applyStyle();
}

void RetroFuturaGUI::Caret::SetColors(std::span<const glm::vec4> colors)
{
    _colors.SetOwn(colors, ColorState::Enabled);
    applyStyle();
}

void RetroFuturaGUI::Caret::SetFillType(const FillType fillType)
{
    _fillTypeLayer.Set(fillType);
    applyStyle();
}

void RetroFuturaGUI::Caret::SetGradientAnimationSpeed(const f32 speed)
{
    _gradientAnimationSpeedLayer.Set(speed);
    applyStyle();
}

void RetroFuturaGUI::Caret::SetBlinkTime(const f64 milliseconds)
{
    if(16.0 >= milliseconds) //at or below one frame a blink can't be seen
    {
        _neverBlinks = true;
        return;
    }

    _blinkTime = milliseconds;
    _neverBlinks = false;
}

f64 RetroFuturaGUI::Caret::GetBlinkTime() const
{
    return _blinkTime;
}

bool RetroFuturaGUI::Caret::NeverBlinks() const
{
    return _neverBlinks;
}

void RetroFuturaGUI::Caret::Draw()
{
    followFlavors();

    //Read before the check: Get reads arrived once the transition is done
    if(_isMoving)
    {
        _plane.SetPosition(_position.Get());
        _plane.SetSize(_size.Get());
        _isMoving = _position.IsRunning() || _size.IsRunning();
    }

    _plane.Draw();
}

void RetroFuturaGUI::Caret::SetSize(const glm::vec2& size)
{
    _size.Jump(size);
    _plane.SetSize(size);
}

void RetroFuturaGUI::Caret::SetPosition(const glm::vec3& position)
{
    //The widget moved: no place in the text, so the next move jumps rather than gliding from here
    _hasPlace = false;
    _isMoving = false;
    _position.Jump(position);
    _size.Jump(_size.GetTarget());
    _plane.SetPosition(position);
    _plane.SetSize(_size.GetTarget());
}

void RetroFuturaGUI::Caret::SetTransitionSettings(const TransitionSettings* settings)
{
    _plane.SetTransitionSettings(settings);
}

void RetroFuturaGUI::Caret::SetMoveTransition(const Transition& transition)
{
    _moveTransition = transition;
}

const RetroFuturaGUI::Transition& RetroFuturaGUI::Caret::GetMoveTransition() const
{
    return _moveTransition;
}

void RetroFuturaGUI::Caret::MoveTo(const glm::vec3& position, const glm::vec2& size)
{
    const std::chrono::steady_clock::time_point now { std::chrono::steady_clock::now() };
    const std::chrono::steady_clock::time_point lastMoveTime { _lastMoveTime };
    _lastMoveTime = now;

    if(!_hasPlace || 0 >= _moveTransition._Duration.count())
    {
        _hasPlace = true;
        _isMoving = false;
        _position.Jump(position);
        _size.Jump(size);
        _plane.SetPosition(position);
        _plane.SetSize(size);
        return;
    }

    //The same place again, as a held press marking the same spot every frame, keeps the glide going rather than starting it over
    if(position == _position.GetTarget() && size == _size.GetTarget())
        return;

    //Moves coming sooner than the transition takes, as a held arrow key or fast typing, each take only the time since the last,
    //so the caret keeps pace instead of falling further behind. A single move takes the whole transition
    Transition transition { _moveTransition };
    transition._Duration = std::min(transition._Duration, std::chrono::duration_cast<std::chrono::milliseconds>(now - lastMoveTime));

    //From where it is shown now, also midway through the last move; the glide's own value, since the plane only follows it while drawn
    _position.Jump(_isMoving ? _position.Get() : _plane.GetPosition());
    _size.Jump(_isMoving ? _size.Get() : _plane.GetSize());
    _position.Set(position, transition);
    _size.Set(size, transition);
    _isMoving = true;
}

void RetroFuturaGUI::Caret::ResetSetting(const StyleSetting setting)
{
    switch(setting)
    {
        case StyleSetting::FillType:
            _fillTypeLayer.Reset();
        break;
        case StyleSetting::GradientAnimationSpeed:
            _gradientAnimationSpeedLayer.Reset();
        break;
        default: //a setting the caret doesn't have.
            return;
    }

    applyStyle();
}

void RetroFuturaGUI::Caret::restyle()
{
    applyStyle();
}

void RetroFuturaGUI::Caret::applyStyle()
{
    const std::span<const u32> flavors { getFlavors() };
    const StylePart part { getStylePart() };

    _plane.SetColors(_colors.GetShown(ColorState::Enabled, flavors, isStyled(), part));

    if(const FillType* value { _fillTypeLayer.Resolve(flavors, isStyled(), part, FillType::Solid, [](const FlavorPart& style) -> const std::optional<FillType>& { return style.GetFillType(); }) })
        _plane.SetFillType(*value);

    if(const f32* value { _gradientAnimationSpeedLayer.Resolve(flavors, isStyled(), part, 0.018f, [](const FlavorPart& style) -> const std::optional<f32>& { return style.GetGradientAnimationSpeed(); }) })
        _plane.SetGradientAnimationSpeed(*value);
}
