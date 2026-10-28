#include "Environment.hpp"
#include "ScissorClip.hpp"
#include "Window.hpp"
#include <algorithm>
#include <cmath>
#include <optional>
#include <print>
#include <utility>

RetroFuturaGUI::Environment::Environment(const std::string& name, Projection* projection, IHierarchyNode* parentWidget, const WidgetTypeID parentWidgetTypeID, GLFWwindow* parentWindow)
    : IWidget(name, projection, parentWidget, parentWidgetTypeID, parentWindow)
{
    _widgetTypeID = WidgetTypeID::Environment;
    _size = glm::vec3(_projection.GetResolution().x, _projection.GetResolution().y, _projection.GetDepth());
    _position = glm::vec3(_size.x * 0.5f, _size.y * 0.5f, 0.0f);

    if(!SetExtent(glm::vec3(1.0f)))
    {
        std::println("ERROR::ENVIRONMENT::INVALID_EXTENT::{}::falling back to 1 on every axis", name);
        applyTransform();
    }

    _debugBox = std::make_unique<Frame>(&_projection);

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

    //Kept alive until the next Draw, since one of its own slots may be running right now. The emptied entry goes then too,
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

void RetroFuturaGUI::Environment::ShowDebugBox(const bool show)
{
    _drawDebugBox = show;
}

void RetroFuturaGUI::Environment::Draw()
{
    //No slot of the removed widgets runs between frames
    _removedWidgets.clear();
    std::erase_if(_entries, [](const EnvironmentEntry& entry) { return !entry._Widget; });

    std::optional<ScissorClip> clip;

    if(_clippingOn)
        clip.emplace(framebufferBox(_position.x - _size.x * 0.5f, _position.y - _size.y * 0.5f, _size.x, _size.y));

    //By index, since a widget's slot may attach another one while it draws, which can move the entries
    for(uSize index = 0; index < _entries.size(); ++index)
    {
        IWidget* widget { _entries[index]._Widget.get() };

        if(!widget || !_entries[index]._Show)
            continue;

        widget->UpdateTooltip();
        widget->Draw();
    }

    if(_drawDebugBox)
        drawDebugBox();
}

void RetroFuturaGUI::Environment::SetSize(const glm::vec3& size)
{
    IWidget::SetSize(size);
    applyTransform();
}

void RetroFuturaGUI::Environment::SetPosition(const glm::vec3& position)
{
    IWidget::SetPosition(position);
    applyTransform();
}

void RetroFuturaGUI::Environment::SetRotation(const glm::vec3& rotation)
{
    IWidget::SetRotation(rotation);
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
    if(!entry._Widget)
        return;

    // The Environment scales every axis by the same factor, so its transform composed with the widget's own still splits
    // into one position, one rotation and one size - the three values every widget takes. A non-uniform scale would shear
    // a rotated widget, and that has no position/rotation/size equivalent. Size goes first, because some widgets lay out
    // their parts from their size when they are moved.
    entry._Widget->SetSize(entry._Placement._Size * _scale);
    entry._Widget->SetRotation(toEulerDegrees(_orientation * glm::quat(glm::radians(entry._Placement._Rotation))));
    entry._Widget->SetPosition(ToWindow(entry._Placement._Position));
}

void RetroFuturaGUI::Environment::drawDebugBox()
{
    if(!_debugBox)
        return;

    _debugBox->SetPosition(_position);
    _debugBox->SetRotation(_rotation);
    _debugBox->SetSize(glm::vec2(_extent.x, _extent.y) * (2.0f * _scale));

    
    const bool depthTestWasEnabled { static_cast<bool>(glIsEnabled(GL_DEPTH_TEST)) };
    glDisable(GL_DEPTH_TEST); //Draw debug lines over everything
    _debugBox->Draw();

    if(depthTestWasEnabled)
        glEnable(GL_DEPTH_TEST);
}

glm::vec3 RetroFuturaGUI::Environment::toEulerDegrees(const glm::quat& rotation)
{
    // glm matrices are column-major (matrix[column][row])
    const glm::mat3 matrix { glm::mat3_cast(rotation) };

    // Z comes out first, then X is solved with that Z already taken out. Near Y = +-90 degrees X and Z turn about the
    // same axis, so Z is only loosely defined there - but X absorbs whatever Z was found, and the three angles still
    // rebuild the same rotation. Snapping Z to 0 near the lock instead visibly tilts widgets within a degree of it.
    const f32
        angleZ { std::atan2(matrix[0][1], matrix[0][0]) },
        angleY { std::atan2(-matrix[0][2], std::hypot(matrix[0][0], matrix[0][1])) },
        sinZ { std::sin(angleZ) },
        cosZ { std::cos(angleZ) },
        angleX { std::atan2(sinZ * matrix[2][0] - cosZ * matrix[2][1], cosZ * matrix[1][1] - sinZ * matrix[1][0]) };

    return glm::degrees(glm::vec3(angleX, angleY, angleZ));
}