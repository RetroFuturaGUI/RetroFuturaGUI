#include "EnvironmentView.hpp"
#include "Shader.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include <cmath>

RetroFuturaGUI::EnvironmentView::EnvironmentView(std::string_view name, Projection* projection, IHierarchyNode* parentWidget, const WidgetTypeID parentWidgetTypeID, GLFWwindow* parentWindow)
    : IWidget(name, projection, parentWidget, parentWidgetTypeID, parentWindow)
{
    _widgetTypeID = WidgetTypeID::EnvironmentView;
    _plane = std::make_unique<Plane>(projection);
    useTransitions({ _plane.get() });

    //The picture keeps its IDs for the view's whole life; a new size only gives them new storage
    glGenTextures(1, &_pictureTexture);
    glBindTexture(GL_TEXTURE_2D, _pictureTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);
    glGenRenderbuffers(1, &_depthBuffer);
    _framebuffer = std::make_unique<Framebuffer>([texture = _pictureTexture, depthBuffer = _depthBuffer]()
        {
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, depthBuffer);
        });

    if(!_plane)
        return;

    _plane->SetColors(_planeColors);
    _plane->SetBackgroundImageTextureID(_pictureTexture);
    _plane->SetBackgroundImagePremultiplied(true); //drawn into by GL, as the window is
}

RetroFuturaGUI::EnvironmentView::~EnvironmentView()
{
    SetWorld(nullptr);
    glDeleteTextures(1, &_pictureTexture);
    glDeleteRenderbuffers(1, &_depthBuffer);
}

void RetroFuturaGUI::EnvironmentView::Draw()
{
    Environment* world { _world.Get() };

    if(!world)
        return;

    if(!_plane)
        return;

    if(!_framebuffer)
        return;

    if(0.0f >= _size.x || 0.0f >= _size.y)
        return;

    renderWorld(*world, glm::i32vec2(std::max(1L, std::lround(_size.x)), std::max(1L, std::lround(_size.y))));
    _plane->Draw();
}

void RetroFuturaGUI::EnvironmentView::SetWorld(Environment* world)
{
    Environment* previous { _world.Get() };

    if(previous)
        std::erase(previous->_views, this);

    _world = world;

    if(!world)
        return;

    world->_views.push_back(this);
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

void RetroFuturaGUI::EnvironmentView::applySize(const glm::vec3& size)
{
    IWidget::applySize(size);

    if(!_plane)
        return;

    _plane->SetSize(glm::vec2(size));
}

void RetroFuturaGUI::EnvironmentView::applyPosition(const glm::vec3& position)
{
    IWidget::applyPosition(position);

    if(!_plane)
        return;

    _plane->SetPosition(position);
}

bool RetroFuturaGUI::EnvironmentView::interactWorld(const bool isPressHeld)
{
    Environment* world { _world.Get() };

    if(!world)
        return false;

    if(!_isEnabledFlag || 0.0f >= _size.x || 0.0f >= _size.y)
        return false;

    glm::vec2 viewWindowPoint { 0.0f };
    const std::optional<glm::vec2> onView { getPointerOnView(isPressHeld, viewWindowPoint) };

    if(!onView)
        return false;

    //The world's widgets cast this point back through their projection, which the camera stands in for meanwhile
    const glm::vec2 resolution { glm::max(world->_viewProjection.GetResolution(), glm::vec2(1.0f)) };
    _viewPointer = ViewPointer { ._World = world, ._WindowPoint = (*onView * 0.5f + 0.5f) * resolution, ._Window = _parentWindow, ._ViewWindowPoint = viewWindowPoint };
    world->interactThrough(getCamera(_size.x / _size.y, glm::length(world->GetExtent())));
    _viewPointer.reset();
    return true;
}

std::optional<glm::vec2> RetroFuturaGUI::EnvironmentView::getPointerOnView(const bool anywhere, glm::vec2& outWindowPoint) const
{
    const PointerState pointer { readPointer() };
    outWindowPoint = pointer._WindowPosition;

    if(!pointer._HasPosition || pointer._IsCaptured)
        return std::nullopt;

    if(!anywhere && !isPointInside(pointer._Position))
        return std::nullopt;

    return (pointer._Position - glm::vec2(_position)) / (glm::vec2(_size) * 0.5f);
}

void RetroFuturaGUI::EnvironmentView::renderWorld(Environment& world, const glm::i32vec2& pixelSize)
{
    sizePicture(pixelSize);

    GLint
        previousFramebuffer { 0 },
        viewport[4] {};
    GLfloat clearColor[4] {};
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &previousFramebuffer);
    glGetIntegerv(GL_VIEWPORT, viewport);
    glGetFloatv(GL_COLOR_CLEAR_VALUE, clearColor);
    const GLboolean hadScissorTest { glIsEnabled(GL_SCISSOR_TEST) };

    _framebuffer->Bind();
    glViewport(0, 0, pixelSize.x, pixelSize.y);
    glDisable(GL_SCISSOR_TEST);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f); //see-through where the world shows nothing
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    {
        //In the world's own place and at full opacity: the picture then shows tilted, cut and faded with the view
        const Shader::FrameScope frame { glm::mat4(1.0f), Shader::FrameComposition::Instead };
        const Shader::OpacityScope opacity { 1.0f, Shader::FrameComposition::Instead };
        const Shader::UnclippedScope unclipped {};
        _isDrawingWorld = true;
        world.drawThrough(getCamera(static_cast<f32>(pixelSize.x) / static_cast<f32>(pixelSize.y), glm::length(world.GetExtent())));
        _isDrawingWorld = false;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, static_cast<GLuint>(previousFramebuffer));
    glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
    glClearColor(clearColor[0], clearColor[1], clearColor[2], clearColor[3]);

    if(hadScissorTest)
        glEnable(GL_SCISSOR_TEST);
}

void RetroFuturaGUI::EnvironmentView::sizePicture(const glm::i32vec2& pixelSize)
{
    if(pixelSize == _pictureSize)
        return;

    _pictureSize = pixelSize;
    glBindTexture(GL_TEXTURE_2D, _pictureTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, pixelSize.x, pixelSize.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindRenderbuffer(GL_RENDERBUFFER, _depthBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, pixelSize.x, pixelSize.y);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
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

const std::optional<RetroFuturaGUI::EnvironmentView::ViewPointer>& RetroFuturaGUI::EnvironmentView::getViewPointer()
{
    return _viewPointer;
}

GLFWwindow* RetroFuturaGUI::EnvironmentView::redirectWindow(GLFWwindow* window)
{
    if(!_viewPointer || !_viewPointer->_World)
        return window;

    if(window != _viewPointer->_World->GetParentWindow())
        return window;

    return _viewPointer->_Window;
}
