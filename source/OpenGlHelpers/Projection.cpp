#include "Projection.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include <cmath>

using namespace RetroFuturaGUI;

Projection::Projection(const f32 width, const f32 height)
: _resolution(width, height)
{
    setProjectionMatrix();
}

const glm::mat4& Projection::GetProjectionMatrix() const
{
    if(_viewOverride)
        return *_viewOverride;

    return _projectionMatrix;
}

void Projection::UpdateProjectionMatrix(const f32 width, const f32 height)
{
    _resolution.x = width;
    _resolution.y = height;
    setProjectionMatrix();
}

const glm::vec2& Projection::GetResolution() const
{
    return _resolution;
}

void Projection::SetPerspective(const f32 viewingDistance, const std::optional<glm::vec2>& vanishingPoint)
{
    _mode = ProjectionMode::Perspective;
    _viewingDistance = std::max(viewingDistance, 1.0f);
    _vanishingPoint = vanishingPoint;
    _depth = _perspectiveDepth;
    setProjectionMatrix();
}

void Projection::SetOrthographic()
{
    _mode = ProjectionMode::Orthographic;
    _depth = _orthographicDepth;
    setProjectionMatrix();
}

ProjectionMode Projection::GetMode() const
{
    return _mode;
}

f32 Projection::GetViewingDistance() const
{
    return _viewingDistance;
}

std::optional<glm::vec3> Projection::GetEyePosition() const
{
    if(ProjectionMode::Orthographic == _mode)
        return std::nullopt;

    return glm::vec3(_vanishingPoint.value_or(_resolution * 0.5f), _viewingDistance);
}

glm::vec3 Projection::GetTowardEye(const glm::vec3& point) const
{
    const glm::mat4& matrix { GetProjectionMatrix() };
    const auto row = [&matrix](const i32 index) { return glm::vec4(matrix[0][index], matrix[1][index], matrix[2][index], matrix[3][index]); };
    const glm::vec4 eye { crossProduct(row(0), row(1), row(3)) };

    if(std::abs(eye.w) <= 1e-6f * glm::length(glm::vec3(eye)))
        return glm::vec3(eye); //orthographic: the eye lies infinitely far along it

    return glm::vec3(eye) / eye.w - point;
}

glm::vec4 Projection::crossProduct(const glm::vec4& first, const glm::vec4& second, const glm::vec4& third)
{
    const auto determinant = [](const glm::vec3& a, const glm::vec3& b, const glm::vec3& c) { return glm::dot(a, glm::cross(b, c)); };

    return glm::vec4(
         determinant(glm::vec3(first.y, first.z, first.w), glm::vec3(second.y, second.z, second.w), glm::vec3(third.y, third.z, third.w)),
        -determinant(glm::vec3(first.x, first.z, first.w), glm::vec3(second.x, second.z, second.w), glm::vec3(third.x, third.z, third.w)),
         determinant(glm::vec3(first.x, first.y, first.w), glm::vec3(second.x, second.y, second.w), glm::vec3(third.x, third.y, third.w)),
        -determinant(glm::vec3(first.x, first.y, first.z), glm::vec3(second.x, second.y, second.z), glm::vec3(third.x, third.y, third.z)));
}

void Projection::setProjectionMatrix()
{
    if(ProjectionMode::Orthographic == _mode)
    {
        _projectionMatrix = glm::ortho(
            0.0f, _resolution.x,
            0.0f, _resolution.y,
            -_depth, _depth
        );

        return;
    }

    // The eye looks at the vanishing point from viewingDistance in front of the screen plane. The view seen there spans the
    // window exactly, so z = 0 stays pixel-exact. Near and far keep the small stacking steps apart in the depth buffer
    const glm::vec2 vanishingPoint { _vanishingPoint.value_or(_resolution * 0.5f) };
    const f32
        nearDistance { _viewingDistance * 0.25f },
        farDistance { _viewingDistance * 10.0f },
        toNearPlane { nearDistance / _viewingDistance };

    _projectionMatrix = glm::frustum(
            -vanishingPoint.x * toNearPlane, (_resolution.x - vanishingPoint.x) * toNearPlane,
            -vanishingPoint.y * toNearPlane, (_resolution.y - vanishingPoint.y) * toNearPlane,
            nearDistance, farDistance)
        * glm::translate(glm::mat4(1.0f), glm::vec3(-vanishingPoint, -_viewingDistance));
}