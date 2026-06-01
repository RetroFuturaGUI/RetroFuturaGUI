#include "Projection.hpp"

using namespace RetroFuturaGUI;

Projection::Projection(const f32 width, const f32 height)
: _resolution(width, height)
{
    _depth = 1000.0f;
    setProjectionMatrix();
}

const glm::mat4& Projection::GetProjectionMatrix() const
{
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

void Projection::setProjectionMatrix()
{
    _projectionMatrix = glm::ortho(
        0.0f, _resolution.x,
        0.0f, _resolution.y,
        -_depth, _depth
    );

    /*float aspectRatio = _resolution.x / _resolution.y;
    _projectionMatrix = glm::perspective(
    glm::radians(45.0f),  // FOV
    aspectRatio,
    0.1f,                 // near plane
    1000.0f               // far plane
); */
}