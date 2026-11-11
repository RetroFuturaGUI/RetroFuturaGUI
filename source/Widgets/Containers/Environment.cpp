#include "Environment.hpp"
#include "Shader.hpp"
#include "Window.hpp"
#include "EnvironmentView.hpp"
#include "IDropDown.hpp"
#include "PlatformBridge.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include <cmath>
#include <optional>
#include <print>
#include <utility>

RetroFuturaGUI::Environment::Environment(std::string_view name, Projection* projection, IHierarchyNode* parentWidget, const WidgetTypeID parentWidgetTypeID, GLFWwindow* parentWindow)
    : IWidget(name, projection, parentWidget, parentWidgetTypeID, parentWindow), _viewProjection(_projection)
{
    _widgetTypeID = WidgetTypeID::Environment;
    _size = glm::vec3(_projection.GetResolution().x, _projection.GetResolution().y, _projection.GetDepth());
    _position = glm::vec3(_size.x * 0.5f, _size.y * 0.5f, 0.0f);

    if(!SetExtent(glm::vec3(1.0f)))
    {
        std::println("ERROR::ENVIRONMENT::INVALID_EXTENT::{}::falling back to 1 on every axis", name);
        applyTransform();
    }

    _debugBox = std::make_unique<Frame>(&_viewProjection);

    if(!_debugBox)
        return;

    _debugBox->SetColors(_debugBoxColor);
    _debugBox->SetBorderWidth(2.0f);
}

bool RetroFuturaGUI::Environment::RemoveWidget(std::string_view name)
{
    EnvironmentEntry* entry { findEntry(name) };

    if(!entry)
        return false;

    //Kept alive until the next Interact, since one of its own slots may be running right now. The emptied entry goes then too,
    //as Draw may be walking the entries at this moment
    _removedWidgets.push_back(std::move(entry->_Widget));
    return true;
}

bool RetroFuturaGUI::Environment::SetPlacement(std::string_view name, const EnvironmentPlacement& placement)
{
    EnvironmentEntry* entry { findEntry(name) };

    if(!entry)
        return false;

    entry->_Placement = placement;
    placeWidget(*entry);
    return true;
}

const RetroFuturaGUI::EnvironmentPlacement* RetroFuturaGUI::Environment::GetPlacement(std::string_view name) const
{
    const EnvironmentEntry* entry { findEntry(name) };

    if(!entry)
        return nullptr;

    return &entry->_Placement;
}

void RetroFuturaGUI::Environment::ShowWidget(std::string_view name, const bool show)
{
    EnvironmentEntry* entry { findEntry(name) };

    if(!entry)
        return;

    entry->_Show = show;
}

bool RetroFuturaGUI::Environment::IsWidgetShown(std::string_view name) const
{
    const EnvironmentEntry* entry { findEntry(name) };

    if(!entry)
        return false;

    return entry->_Show;
}

bool RetroFuturaGUI::Environment::SetExtent(const glm::vec3& extent)
{
    if(0.0f >= extent.x || 0.0f >= extent.y || 0.0f >= extent.z)
        return false;

    _extent = extent;
    applyTransform();
    return true;
}

const glm::vec3& RetroFuturaGUI::Environment::GetExtent() const
{
    return _extent;
}

void RetroFuturaGUI::Environment::SetFit(const EnvironmentFit fit)
{
    _fit = fit;
    applyTransform();
}

RetroFuturaGUI::EnvironmentFit RetroFuturaGUI::Environment::GetFit() const
{
    return _fit;
}

f32 RetroFuturaGUI::Environment::GetScale() const
{
    return _scale;
}

glm::vec3 RetroFuturaGUI::Environment::ToWindow(const glm::vec3& point) const
{
    return _position + _orientation * (point * _scale);
}

void RetroFuturaGUI::Environment::EnableClipping(const bool enable)
{
    _clippingOn = enable;
}

void RetroFuturaGUI::Environment::SetPerspective(const f32 viewingDistance)
{
    _hasOwnPerspective = true;
    _viewProjection.SetPerspective(viewingDistance, glm::vec2(_position));
}

void RetroFuturaGUI::Environment::FollowWindowProjection()
{
    _hasOwnPerspective = false;
    followProjection();
}

void RetroFuturaGUI::Environment::ShowDebugBox(const bool show)
{
    _drawDebugBox = show;
}

void RetroFuturaGUI::Environment::Draw()
{
    followProjection();
    std::optional<Shader::ClipScope> clip;

    if(_clippingOn)
        clip.emplace(glm::vec4(_position.x - _size.x * 0.5f, _position.y - _size.y * 0.5f, _size.x, _size.y), _position.z, _viewProjection); //its own box, tilted with it.

    drawWidgets();

    if(_drawDebugBox)
        drawDebugBox();
}

void RetroFuturaGUI::Environment::drawWidgets()
{
    //By index, since a slot that runs while a widget draws, like a Video's, may attach another one
    for(uSize index = 0; index < _entries.size(); ++index)
    {
        IWidget* widget { _entries[index]._Widget.get() };

        if(!widget || !_entries[index]._Show)
            continue;

        widget->DrawAtOpacity();
    }
}

void RetroFuturaGUI::Environment::drawThrough(const glm::mat4& camera)
{
    if(0.0f >= _scale)
        return;

    //The lists open in the world go into the view's picture too, apart from those the view's window waits to draw
    std::vector<IDropDown*> windowLists { IDropDown::takeShown() };
    _viewProjection._viewOverride = camera * getUnitsFromLayout();
    drawWidgets();
    IDropDown::DrawShown();
    _viewProjection._viewOverride.reset();
    IDropDown::restoreShown(std::move(windowLists));
}

void RetroFuturaGUI::Environment::interactThrough(const glm::mat4& camera)
{
    if(0.0f >= _scale)
        return;

    const Shader::FrameScope frame { glm::mat4(1.0f), Shader::FrameComposition::Instead }; //as the view draws them
    _viewProjection._viewOverride = camera * getUnitsFromLayout();
    interactWidgets();
    _viewProjection._viewOverride.reset();
}

glm::mat4 RetroFuturaGUI::Environment::getUnitsFromLayout() const
{
    return glm::scale(glm::mat4(1.0f), glm::vec3(1.0f / _scale)) * glm::translate(glm::mat4(1.0f), -_position);
}

void RetroFuturaGUI::Environment::interact()
{
    followProjection(); //the widgets take the pointer through the projection they draw through

    //No slot of the removed widgets runs between frames
    _removedWidgets.clear();
    std::erase_if(_entries, [](const EnvironmentEntry& entry) { return !entry._Widget; });

    //Once a frame only, so hover and presses don't flip between two pointers. A held press stays where it began: here, or on
    //a view, which keeps it anywhere until it is let go. Otherwise through a view the pointer is over, if any
    const bool isPressHeld { PlatformBridge::Input::IsMouseButtonDown(PlatformBridge::MouseButton::Left) || PlatformBridge::Input::IsMouseButtonDown(PlatformBridge::MouseButton::Right) };

    if(!isPressHeld)
    {
        _heldView = nullptr;
        _isPressHeldInWindow = false;
    }

    if(_isPressHeldInWindow)
    {
        interactWidgets();
        return;
    }

    const bool isHeldViewHere { _heldView && _views.end() != std::find(_views.begin(), _views.end(), _heldView) };

    if(isHeldViewHere && _heldView->interactWorld(true))
        return;

    _heldView = nullptr;

    for(EnvironmentView* view : _views)
    {
        if(!view || !view->interactWorld(false))
            continue;

        _heldView = isPressHeld ? view : nullptr;
        return;
    }

    _isPressHeldInWindow = isPressHeld;
    interactWidgets();
}

void RetroFuturaGUI::Environment::interactWidgets()
{
    //By index, since a widget's slot may attach another one, which can move the entries
    for(uSize index = 0; index < _entries.size(); ++index)
    {
        IWidget* widget { _entries[index]._Widget.get() };

        if(!widget || !_entries[index]._Show)
            continue;

        widget->Interact();
    }
}

void RetroFuturaGUI::Environment::update()
{
    //By index, since a widget's slot may attach another one, which can move the entries
    for(uSize index = 0; index < _entries.size(); ++index)
    {
        IWidget* widget { _entries[index]._Widget.get() };

        if(!widget)
            continue;

        widget->Update();
    }
}

void RetroFuturaGUI::Environment::applySize(const glm::vec3& size)
{
    IWidget::applySize(size);
    applyTransform();
}

void RetroFuturaGUI::Environment::applyPosition(const glm::vec3& position)
{
    IWidget::applyPosition(position);
    applyTransform();
}

void RetroFuturaGUI::Environment::applyRotation(const glm::vec3& rotation)
{
    IWidget::applyRotation(rotation);
    applyTransform();
}

RetroFuturaGUI::EnvironmentEntry* RetroFuturaGUI::Environment::findEntry(std::string_view name)
{
    return const_cast<EnvironmentEntry*>(std::as_const(*this).findEntry(name));
}

const RetroFuturaGUI::EnvironmentEntry* RetroFuturaGUI::Environment::findEntry(std::string_view name) const
{
    const auto entryIterator { std::ranges::find_if(_entries, [name](const EnvironmentEntry& entry) { return entry._Widget && name == entry._Widget->GetName(); }) };

    if(entryIterator == _entries.end())
        return nullptr;

    return &*entryIterator;
}

void RetroFuturaGUI::Environment::applyTransform()
{
    const f32
        widthScale { _size.x / (2.0f * _extent.x) },
        heightScale { _size.y / (2.0f * _extent.y) };

    switch(_fit)
    {
        case EnvironmentFit::Cover:
            _scale = std::max(widthScale, heightScale);
        break;
        case EnvironmentFit::KeepHeight:
            _scale = heightScale;
        break;
        case EnvironmentFit::KeepWidth:
            _scale = widthScale;
        break;
        case EnvironmentFit::Contain:
        default:
            _scale = std::min(widthScale, heightScale);
        break;
    }

    // glm builds a quaternion from Euler angles as Z * Y * X, the same order every widget composes its rotation matrix in.
    _orientation = glm::quat(glm::radians(_rotation));

    for(const EnvironmentEntry& entry : _entries)
        placeWidget(entry);
}

void RetroFuturaGUI::Environment::placeWidget(const EnvironmentEntry& entry) const
{
    const Transitions::InstantScope instant { true }; //a container lays its widgets out at once, whatever transitions they have
    if(!entry._Widget)
        return;

    // Placed untilted with its own rotation only. The Environment's frame turns every widget around the box's center. Size
    // goes first, because some widgets lay out their parts from their size when they are moved!!
    entry._Widget->SetSize(entry._Placement._Size * _scale);
    entry._Widget->SetRotation(entry._Placement._Rotation);
    entry._Widget->SetPosition(_position + entry._Placement._Position * _scale);
}

void RetroFuturaGUI::Environment::followProjection()
{
    if(!_hasOwnPerspective)
    {
        _viewProjection = _projection;
        return;
    }

    const glm::vec2 resolution { _projection.GetResolution() };

    if(resolution != _viewProjection.GetResolution())
        _viewProjection.UpdateProjectionMatrix(resolution.x, resolution.y);

    _viewProjection.SetPerspective(_viewProjection.GetViewingDistance(), glm::vec2(_position)); //vanishing at its center, wherever it moved.
}

void RetroFuturaGUI::Environment::drawDebugBox()
{
    if(!_debugBox)
        return;

    _debugBox->SetPosition(_position); //untilted: the Environment's frame turns it
    _debugBox->SetSize(glm::vec2(_extent.x, _extent.y) * (2.0f * _scale));

    
    const bool depthTestWasEnabled { static_cast<bool>(glIsEnabled(GL_DEPTH_TEST)) };
    glDisable(GL_DEPTH_TEST); //Draw debug lines over everything
    _debugBox->Draw();

    if(depthTestWasEnabled)
        glEnable(GL_DEPTH_TEST);
}