#include "Projection.hpp"

using namespace RetroFuturaGUI;

Projection::Projection(const f32 width, const f32 height)
: _resolution(width, height)
{
    setProjectionMatrix();
}

glm::mat4 &Projection::GetProjectionMatrix()
{
     return _projectionMatrix;
}

void Projection::UpdateProjectionMatrix(const f32 width, const f32 height)
{
    _resolution.x = width;
    _resolution.y = height;
    setProjectionMatrix();
}

glm::vec2& Projection::GetResolution()
{
    return _resolution;
}

void Projection::setProjectionMatrix()
{
    _projectionMatrix = glm::ortho(
        0.0f, _resolution.x,
        0.0f, _resolution.y,
        -1.0f, 1.0f
    );
}