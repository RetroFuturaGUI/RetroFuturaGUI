#include "Environment.hpp"
#include <algorithm>
#include <cmath>
#include <print>
#include <utility>

namespace
{
    /// @brief Splits a rotation into the X, Y and Z angles, in degrees, that every widget composes as Rz * Ry * Rx (see Model::SetRotation).
    glm::vec3 toEulerDegrees(const glm::quat& rotation)
    {
        // glm matrices are column-major, so matrix[column][row].
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
}

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

    _debugBox = std::make_unique<Rectangle>(&_projection);

    if(!_debugBox)
        return;

    _debugBox->SetRectangleMode(RectangleMode::Border);
    _debugBox->SetColors(_debugBoxColor);
    _debugBox->SetBorderWidth(2.0f);
}

bool RetroFuturaGUI::Environment::AttachWidget(IWidget* widget, const EnvironmentPlacement& placement)
{
    if(!widget)
        return false;

    if(widget == this) //avoid recursive calls
        return false;

    if(findEntry(widget))
    {
        std::println("ERROR::ENVIRONMENT::WIDGET_ALREADY_ATTACHED::{}::{}", _name, widget->GetName());
        return false;
    }

    _entries.push_back(EnvironmentEntry { ._Widget = widget, ._Placement = placement, ._Show = true });
    placeWidget(_entries.back());
    return true;
}

bool RetroFuturaGUI::Environment::DetachWidget(const IWidget* widget)
{
    const auto entryIterator { std::ranges::find(_entries, widget, &EnvironmentEntry::_Widget) };

    if(entryIterator == _entries.end())
        return false;

    _entries.erase(entryIterator);
    return true;
}

bool RetroFuturaGUI::Environment::SetPlacement(const IWidget* widget, const EnvironmentPlacement& placement)
{
    EnvironmentEntry* entry { findEntry(widget) };

    if(!entry)
        return false;

    entry->_Placement = placement;
    placeWidget(*entry);
    return true;
}

const RetroFuturaGUI::EnvironmentPlacement* RetroFuturaGUI::Environment::GetPlacement(const IWidget* widget) const
{
    const EnvironmentEntry* entry { findEntry(widget) };

    if(!entry)
        return nullptr;

    return &entry->_Placement;
}

void RetroFuturaGUI::Environment::ShowWidget(const IWidget* widget, const bool show)
{
    EnvironmentEntry* entry { findEntry(widget) };

    if(!entry)
        return;

    entry->_Show = show;
}

bool RetroFuturaGUI::Environment::IsWidgetShown(const IWidget* widget) const
{
    const EnvironmentEntry* entry { findEntry(widget) };

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

void RetroFuturaGUI::Environment::SetClipping(const bool clip)
{
    _clip = clip;
}

void RetroFuturaGUI::Environment::ShowDebugBox(const bool show)
{
    _drawDebugBox = show;
}

void RetroFuturaGUI::Environment::Draw()
{
    ScissorState scissorState {};

    if(_clip)
        scissorState = pushScissor(_position.x - _size.x * 0.5f, _position.y - _size.y * 0.5f, _size.x, _size.y);

    for(const EnvironmentEntry& entry : _entries)
    {
        if(!entry._Widget || !entry._Show)
            continue;

        entry._Widget->Draw();
    }

    if(_drawDebugBox)
        drawDebugBox();

    if(_clip)
        popScissor(scissorState);
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

RetroFuturaGUI::EnvironmentEntry* RetroFuturaGUI::Environment::findEntry(const IWidget* widget)
{
    return const_cast<EnvironmentEntry*>(std::as_const(*this).findEntry(widget));
}

const RetroFuturaGUI::EnvironmentEntry* RetroFuturaGUI::Environment::findEntry(const IWidget* widget) const
{
    const auto entryIterator { std::ranges::find(_entries, widget, &EnvironmentEntry::_Widget) };

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

RetroFuturaGUI::Environment::ScissorState RetroFuturaGUI::Environment::pushScissor(const f32 left, const f32 bottom, const f32 width, const f32 height)
{
    ScissorState state {};
    state._WasEnabled = static_cast<bool>(glIsEnabled(GL_SCISSOR_TEST));
    glGetIntegerv(GL_SCISSOR_BOX, state._Previous);

    i32
        clipLeft { static_cast<i32>(left) },
        clipBottom { static_cast<i32>(bottom) },
        clipRight { clipLeft + static_cast<i32>(width) },
        clipTop { clipBottom + static_cast<i32>(height) };


    if(state._WasEnabled) // A clip already active, such as an enclosing Table's, stays in force: the Environment only narrows it.
    {
        clipLeft = std::max(clipLeft, state._Previous[0]);
        clipBottom = std::max(clipBottom, state._Previous[1]);
        clipRight = std::min(clipRight, state._Previous[0] + state._Previous[2]);
        clipTop = std::min(clipTop, state._Previous[1] + state._Previous[3]);
    }

    glEnable(GL_SCISSOR_TEST);
    glScissor(clipLeft, clipBottom, std::max(clipRight - clipLeft, 0), std::max(clipTop - clipBottom, 0));
    return state;
}

void RetroFuturaGUI::Environment::popScissor(const ScissorState& state)
{
    if(state._WasEnabled)
        glScissor(state._Previous[0], state._Previous[1], state._Previous[2], state._Previous[3]);
    else
        glDisable(GL_SCISSOR_TEST);
}
