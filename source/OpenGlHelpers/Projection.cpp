#include "Projection.hpp"

using namespace RetroFuturaGUI;

Projection::Projection(const float width, const float height)
: _resolution(width, height)
{
    setProjectionMatrix();
}

glm::mat4 &Projection::GetProjectionMatrix()
{
     return _projectionMatrix;
}

void Projection::UpdateProjectionMatrix(const float width, const float height)
{
    _resolution.x = width;
    _resolution.y = height;
    setProjectionMatrix();
}

glm::vec2 &Projection::GetResolution()
{
    return _resolution;
}

void Projection::setProjectionMatrix()
{
    _projectionMatrix = glm::ortho(
        -_resolution.x * 0.01f,
        _resolution.x * 0.01f,
        -_resolution.y * 0.01f,
        _resolution.y * 0.01f,
        -1.0f, 1.0f
    );
}
