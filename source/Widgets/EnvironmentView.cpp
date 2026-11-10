#include "EnvironmentView.hpp"
#include "Shader.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include <cmath>

RetroFuturaGUI::EnvironmentView::EnvironmentView(std::string_view name, Projection* projection, IHierarchyNode* parentWidget, const WidgetTypeID parentWidgetTypeID, GLFWwindow* parentWindow)
    : IWidget(name, projection, parentWidget, parentWidgetTypeID, parentWindow)
{
    _widgetTypeID = WidgetTypeID::EnvironmentView;
}

void RetroFuturaGUI::EnvironmentView::Draw()
{
    Environment* world { _world.Get() };

    if(!world)
        return;

    //Its rectangle in the window's framebuffer, which counts from the bottom as the layout does
    const glm::ivec4 box
    {
        static_cast<i32>(std::lround(_position.x - _size.x * 0.5f)),
        static_cast<i32>(std::lround(_position.y - _size.y * 0.5f)),
        static_cast<i32>(std::lround(_size.x)),
        static_cast<i32>(std::lround(_size.y))
    };

    if(0 >= box.z || 0 >= box.w)
        return;

    GLint
        viewport[4] {},
        scissorBox[4] {};
    glGetIntegerv(GL_VIEWPORT, viewport);
    glGetIntegerv(GL_SCISSOR_BOX, scissorBox);
    const GLboolean hadScissorTest { glIsEnabled(GL_SCISSOR_TEST) };

    glViewport(box.x, box.y, box.z, box.w);
    glEnable(GL_SCISSOR_TEST);
    glScissor(box.x, box.y, box.z, box.w);
    glClear(GL_DEPTH_BUFFER_BIT); //the world's depth starts afresh inside the view

    {
        const Shader::FrameScope frame { glm::mat4(1.0f), Shader::FrameComposition::Instead }; //in the world's own place, not this window's
        const Shader::UnclippedScope unclipped {};
        _isDrawingWorld = true;
        world->drawThrough(getCamera(static_cast<f32>(box.z) / static_cast<f32>(box.w), glm::length(world->GetExtent())));
        _isDrawingWorld = false;
    }

    glClear(GL_DEPTH_BUFFER_BIT); //the world's depth means nothing to what this window draws after it
    glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
    glScissor(scissorBox[0], scissorBox[1], scissorBox[2], scissorBox[3]);

    if(!hadScissorTest)
        glDisable(GL_SCISSOR_TEST);
}

void RetroFuturaGUI::EnvironmentView::SetWorld(Environment* world)
{
    _world = world;
}

RetroFuturaGUI::Environment* RetroFuturaGUI::EnvironmentView::GetWorld() const
{
    return _world.Get();
}

void RetroFuturaGUI::EnvironmentView::SetCamera(const glm::vec3& eye, const glm::vec3& target, const glm::vec3& up)
{
    _eye = eye;
    _target = target;
    _up = up;
}

const glm::vec3& RetroFuturaGUI::EnvironmentView::GetEye() const
{
    return _eye;
}

const glm::vec3& RetroFuturaGUI::EnvironmentView::GetTarget() const
{
    return _target;
}

const glm::vec3& RetroFuturaGUI::EnvironmentView::GetUp() const
{
    return _up;
}

void RetroFuturaGUI::EnvironmentView::SetPerspective(const f32 fieldOfView)
{
    _cameraMode = CameraMode::Perspective;
    _fieldOfView = std::clamp(fieldOfView, 1.0f, 179.0f);
}

void RetroFuturaGUI::EnvironmentView::SetOrthographic(const f32 height)
{
    _cameraMode = CameraMode::Orthographic;
    _orthographicHeight = 0.0f < height ? height : _orthographicHeight;
}

RetroFuturaGUI::CameraMode RetroFuturaGUI::EnvironmentView::GetCameraMode() const
{
    return _cameraMode;
}

bool RetroFuturaGUI::EnvironmentView::IsDrawingWorld()
{
    return _isDrawingWorld;
}

glm::mat4 RetroFuturaGUI::EnvironmentView::getCamera(const f32 aspect, const f32 worldRadius) const
{
    const f32
        distance { glm::length(_eye - _target) },
        reach { distance + 2.0f * worldRadius + 1.0f }; //the whole world behind the target
    const glm::mat4 view { glm::lookAt(_eye, _target, _up) };

    if(CameraMode::Orthographic == _cameraMode)
    {
        const f32
            halfHeight { _orthographicHeight * 0.5f },
            halfWidth { halfHeight * aspect };
        return glm::ortho(-halfWidth, halfWidth, -halfHeight, halfHeight, -reach, reach) * view;
    }

    const f32 nearPlane { std::max(distance * 0.01f, 0.001f) };
    return glm::perspective(glm::radians(_fieldOfView), aspect, nearPlane, reach) * view;
}