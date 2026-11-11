#include "IWidget.hpp"
#include "Scene.hpp"
#include "EnvironmentView.hpp"
#include "PlatformBridge.hpp"
#include "IDropDown.hpp"
#include "Tooltip.hpp"
#include "ContextMenu.hpp"
#include "WidgetRegistry.hpp"
#include "IWidgetDecoration.hpp"
#include "WidgetText.hpp"
#include "StyledPart.hpp"
#include <algorithm>
#include "FillStyle.hpp"
#include "Background.hpp"
#include "Border.hpp"
#include "Window.hpp"
#include "Shader.hpp"
#include <algorithm>
#include <memory>

#if defined(TARGET_PLATFORM_LINUX)
    #define GLFW_EXPOSE_NATIVE_X11
#elif defined(TARGET_PLATFORM_WINDOWS)
    #define GLFW_EXPOSE_NATIVE_WIN32
#endif
#include <GLFW/glfw3native.h>

RetroFuturaGUI::IWidget::IWidget(std::string_view name, Projection* projection, IHierarchyNode* parentWidget, const WidgetTypeID parentWidgetTypeID, GLFWwindow* parentWindow)
    : _name(name), _parentWidget(parentWidget), _parentWidgetTypeID(parentWidgetTypeID), _projection(*projection)
{
    _parentWindow = parentWindow;
    WidgetRegistry::add(this);
}

RetroFuturaGUI::IWidget::~IWidget()
{
    WidgetRegistry::remove(this);
}

void RetroFuturaGUI::IWidget::Connect_OnEnable(const typename Signal<>::Slot& slot, const bool async)
{
    if (async)
        _onEnableAsync.Connect(slot);
    else
        _onEnable.Connect(slot);
}

void RetroFuturaGUI::IWidget::Connect_OnDisable(const typename Signal<>::Slot& slot, const bool async)
{
    if (async)
        _onDisableAsync.Connect(slot);
    else
        _onDisable.Connect(slot);
}

void RetroFuturaGUI::IWidget::Disconnect_OnEnable(const typename Signal<>::Slot& slot)
{
    _onEnable.Disconnect(slot);
    _onEnableAsync.Disconnect(slot);
}

void RetroFuturaGUI::IWidget::Disconnect_OnDisable(const typename Signal<>::Slot& slot)
{
    _onDisable.Disconnect(slot);
    _onDisableAsync.Disconnect(slot);
}

void RetroFuturaGUI::IWidget::SetSize(const glm::vec3& size)
{
    const Transition transition { Transitions::Resolve(_transitionSettings, TransitionProperty::Size) };

    //Before the first frame, without a transition, inside an InstantScope (a container laying the widget out) or with nothing to move, it applies at once, as a plain setter would
    if(!_hasUpdated || 0 >= transition._Duration.count() || (!_isResizing && size == _size))
    {
        _isResizing = false;
        _sizeTransition.Jump(size);
        const Transitions::InstantScope instant { true };
        applySize(size);
        return;
    }

    if(!_isResizing)
        _sizeTransition.Jump(_size); //from where it is now

    _sizeTransition.Set(size, transition);
    _isResizing = true;
}

glm::vec3 RetroFuturaGUI::IWidget::GetSize() const
{
    return _size;
}

void RetroFuturaGUI::IWidget::SetPosition(const glm::vec3& position)
{
    const Transition transition { Transitions::Resolve(_transitionSettings, TransitionProperty::Position) };

    if(!_hasUpdated || 0 >= transition._Duration.count() || (!_isMoving && position == _position))
    {
        _isMoving = false;
        _positionTransition.Jump(position);
        const Transitions::InstantScope instant { true };
        applyPosition(position);
        return;
    }

    if(!_isMoving)
        _positionTransition.Jump(_position);

    _positionTransition.Set(position, transition);
    _isMoving = true;
}

glm::vec3 RetroFuturaGUI::IWidget::GetPosition() const
{
    return _position;
}

void RetroFuturaGUI::IWidget::SetRotation(const glm::vec3& rotation)
{
    const Transition transition { Transitions::Resolve(_transitionSettings, TransitionProperty::Rotation) };

    if(!_hasUpdated || 0 >= transition._Duration.count() || (!_isTurning && rotation == _rotation))
    {
        _isTurning = false;
        _rotationTransition.Jump(rotation);
        const Transitions::InstantScope instant { true };
        applyRotation(rotation);
        return;
    }

    if(!_isTurning)
        _rotationTransition.Jump(_rotation);

    _rotationTransition.Set(rotation, transition);
    _isTurning = true;
}

void RetroFuturaGUI::IWidget::applySize(const glm::vec3& size)
{
    _size = size;
}

void RetroFuturaGUI::IWidget::applyPosition(const glm::vec3& position)
{
    _position = position;
}

void RetroFuturaGUI::IWidget::applyRotation(const glm::vec3& rotation)
{
    _rotation = rotation;
}

void RetroFuturaGUI::IWidget::updateGeometry()
{
    //Each value is read before the InstantScope, inside which it would read as arrived
    if(_isResizing)
    {
        const glm::vec3 size { _sizeTransition.Get() };
        _isResizing = _sizeTransition.IsRunning();
        const Transitions::InstantScope instant { true };
        applySize(size);
    }

    if(_isMoving)
    {
        const glm::vec3 position { _positionTransition.Get() };
        _isMoving = _positionTransition.IsRunning();
        const Transitions::InstantScope instant { true };
        applyPosition(position);
    }

    if(_isTurning)
    {
        const glm::vec3 rotation { _rotationTransition.Get() };
        _isTurning = _rotationTransition.IsRunning();
        const Transitions::InstantScope instant { true };
        applyRotation(rotation);
    }

    if(_isFading)
    {
        _opacity = _opacityTransition.Get();
        _isFading = _opacityTransition.IsRunning();
    }
}

void RetroFuturaGUI::IWidget::DrawAtOpacity()
{
    if(0.0f >= _opacity)
        return; //nothing of it would show

    const Shader::OpacityScope opacity { _opacity };
    const Shader::FrameScope frame { getTilt(), Shader::FrameComposition::Inside };
    _frame = Shader::GetDrawFrame();
    Draw();
}

glm::mat4 RetroFuturaGUI::IWidget::getTilt() const
{
    if(glm::vec3(0.0f) == _rotation)
        return glm::mat4(1.0f);

    // Turned around the center in the same Z * Y * X order every element composes its own rotation in
    const glm::vec3 radians { glm::radians(_rotation) };
    return glm::translate(glm::mat4(1.0f), _position)
        * glm::rotate(glm::mat4(1.0f), radians.z, glm::vec3(0.0f, 0.0f, 1.0f))
        * glm::rotate(glm::mat4(1.0f), radians.y, glm::vec3(0.0f, 1.0f, 0.0f))
        * glm::rotate(glm::mat4(1.0f), radians.x, glm::vec3(1.0f, 0.0f, 0.0f))
        * glm::translate(glm::mat4(1.0f), -_position);
}

std::optional<glm::vec2> RetroFuturaGUI::IWidget::toLayoutPoint(const glm::vec2& windowPoint, const glm::mat4& frame) const
{
    // The view through the window point from the near to the far end of the view-span. Taken back into the untilted layout
    const glm::vec2 resolution { glm::max(_projection.GetResolution(), glm::vec2(1.0f)) };
    const glm::vec2 clip { windowPoint / resolution * 2.0f - 1.0f };
    const glm::mat4 toLayout { glm::inverse(_projection.GetProjectionMatrix() * frame) };
    const glm::vec4
        nearPoint { toLayout * glm::vec4(clip, -1.0f, 1.0f) },
        farPoint { toLayout * glm::vec4(clip, 1.0f, 1.0f) };
    const glm::vec3
        rayStart { glm::vec3(nearPoint) / nearPoint.w },
        rayEnd { glm::vec3(farPoint) / farPoint.w };

    if(std::abs(rayEnd.z - rayStart.z) < 1e-6f)
        return std::nullopt; //the layout's plane is edge-on to the view

    // Where the ray meets the layout's plane at the widget's depth
    const f32 along { (_position.z - rayStart.z) / (rayEnd.z - rayStart.z) };
    return glm::vec2(rayStart + (rayEnd - rayStart) * along);
}

std::optional<glm::vec2> RetroFuturaGUI::IWidget::toOwnLayoutPoint(const glm::vec2& windowPoint) const
{
    return toLayoutPoint(windowPoint, _frame);
}

void RetroFuturaGUI::IWidget::SetOpacity(const f32 opacity)
{
    const f32 target { std::clamp(opacity, 0.0f, 1.0f) };
    const Transition transition { Transitions::Resolve(_transitionSettings, TransitionProperty::Opacity) };

    if(!_hasUpdated || 0 >= transition._Duration.count())
    {
        _isFading = false;
        _opacityTransition.Jump(target);
        _opacity = target;
        return;
    }

    if(!_isFading)
        _opacityTransition.Jump(_opacity); //from where it is now

    _opacityTransition.Set(target, transition);
    _isFading = true;
}

f32 RetroFuturaGUI::IWidget::GetOpacity() const
{
    return _opacity;
}

glm::vec3 RetroFuturaGUI::IWidget::GetRotation() const
{
    return _rotation;
}

const std::string& RetroFuturaGUI::IWidget::GetName() const
{
    return _name;
}

const RetroFuturaGUI::IHierarchyNode* RetroFuturaGUI::IWidget::GetParent() const
{
    return _parentWidget;
}

void RetroFuturaGUI::IWidget::SetName(std::string_view name)
{
    _name = name;
    WidgetRegistry::markChanged(); //this path changed, and with it those of everything inside
}

u32 RetroFuturaGUI::IWidget::GetPathHash() const
{
    return WidgetRegistry::getPathHash(*this);
}

RetroFuturaGUI::WidgetTypeID RetroFuturaGUI::IWidget::GetWidgetTypeID() const
{
    return _widgetTypeID;
}

GLFWwindow* RetroFuturaGUI::IWidget::GetParentWindow() const
{
    return _parentWindow;
}

void RetroFuturaGUI::IWidget::SetEnabled(const bool enable, const bool emitSignal)
{
    _isOwnEnabled = enable;
    const Scene* scene { findScene() };
    applyEnabled(enable && (!scene || scene->IsEnabled()), emitSignal);
}

void RetroFuturaGUI::IWidget::followSceneEnabled()
{
    const Scene* scene { findScene() };

    if(!scene)
        return;

    const bool enabled { _isOwnEnabled && scene->IsEnabled() };

    if(enabled == _isEnabledFlag)
        return;

    applyEnabled(enabled, scene->IsEnabledSignalEmitted());
}

void RetroFuturaGUI::IWidget::applyEnabled(const bool enabled, const bool emitSignal)
{
    const bool changed { enabled != _isEnabledFlag };
    _isEnabledFlag = enabled;

    if(emitSignal && _isEnabledFlag)
    {
        _onEnableAsync.EmitAsync();
        _onEnable.Emit();
    }
    else if(emitSignal)
    {
        _onDisableAsync.EmitAsync();
        _onDisable.Emit();
    }

    setColors(getRestingColorState());

    if(changed)
        enabledChanged();
}

const RetroFuturaGUI::Scene* RetroFuturaGUI::IWidget::findScene() const
{
    for(const IHierarchyNode* node { _parentWidget }; node; node = node->GetParent())
    {
        const Scene* scene { dynamic_cast<const Scene*>(node) };

        if(scene)
            return scene;
    }

    return nullptr;
}

RetroFuturaGUI::ColorState RetroFuturaGUI::IWidget::getRestingColorState() const
{
    return _isEnabledFlag ? ColorState::Enabled : ColorState::Disabled;
}

bool RetroFuturaGUI::IWidget::IsEnabled() const
{
    return _isEnabledFlag;
}

bool RetroFuturaGUI::IWidget::isPointInsideRect(const glm::vec2& point, const glm::vec3& size, const glm::vec3& position, const glm::vec3& rotation) const
{
    // The point is in the widget's untilted layout (readPointer), so only a part's own turn within it applies here
    glm::vec2 translatedPoint = point - glm::vec2(position.x, position.y);
    f32 radians = glm::radians(- rotation.z);
    glm::vec2 rotatedPoint(
        translatedPoint.x * cos(radians) - translatedPoint.y * sin(radians),
        translatedPoint.x * sin(radians) + translatedPoint.y * cos(radians)
    );
    return (rotatedPoint.x >= -size.x * 0.5f &&
            rotatedPoint.x <= size.x * 0.5f &&
            rotatedPoint.y >= -size.y * 0.5f &&
            rotatedPoint.y <= size.y * 0.5f);
}

bool RetroFuturaGUI::IWidget::isPointInside(const glm::vec2& point) const
{
    return isPointInsideRect(point, _size, _position, glm::vec3(0.0f)); //the widget's own rotation is in its frame
}

RetroFuturaGUI::PointerState RetroFuturaGUI::IWidget::readPointer() const
{
    const std::optional<EnvironmentView::ViewPointer>& viewPointer { EnvironmentView::getViewPointer() };

    if(viewPointer && isInside(viewPointer->_World))
        return readPointerThroughView(viewPointer->_WindowPoint, viewPointer->_World);

    PointerState pointer {};
    i32
        mouseX { 0 },
        mouseY { 0 };

#if defined(TARGET_PLATFORM_LINUX)
    pointer._HasPosition = PlatformBridge::Input::GetMouseWindowPosition(glfwGetX11Window(_parentWindow), mouseX, mouseY);
#elif defined(TARGET_PLATFORM_WINDOWS)
    pointer._HasPosition = PlatformBridge::Input::GetMouseWindowPosition(glfwGetWin32Window(_parentWindow), mouseX, mouseY);
#endif

    // PlatformBridge reports native, top-down window coordinates; the library's world space runs bottom-up. The widget sees
    // the point where the pointer meets its untilted layout. For an untilted widget in an orthographic view, it's the same point
    pointer._WindowPosition = glm::vec2(static_cast<f32>(mouseX), _projection.GetResolution().y - static_cast<f32>(mouseY));
    const std::optional<glm::vec2> layoutPoint { toOwnLayoutPoint(pointer._WindowPosition) };
    pointer._HasPosition = pointer._HasPosition && layoutPoint.has_value();
    pointer._Position = layoutPoint.value_or(pointer._WindowPosition);
    pointer._LeftButtonPressed = PlatformBridge::Input::IsMouseButtonDown(PlatformBridge::MouseButton::Left);
    pointer._RightButtonPressed = PlatformBridge::Input::IsMouseButtonDown(PlatformBridge::MouseButton::Right);
    Window::ReadWheel(_parentWindow, pointer);

    filterThroughAncestors(_parentWidget, this, pointer, nullptr);
    IDropDown::CaptureUnderOpenList(this, _parentWindow, pointer);
    return pointer;
}

void RetroFuturaGUI::IWidget::requestCursorIcon(const PlatformBridge::CursorIcon icon) const
{
    Window::RequestCursorIcon(_parentWindow, icon);
}

RetroFuturaGUI::PointerState RetroFuturaGUI::IWidget::readPointerThroughView(const glm::vec2& windowPoint, const IHierarchyNode* world) const
{
    PointerState pointer {};
    pointer._WindowPosition = windowPoint;
    const std::optional<glm::vec2> layoutPoint { toOwnLayoutPoint(windowPoint) }; //through the projection the camera stands in for
    pointer._HasPosition = layoutPoint.has_value();
    pointer._Position = layoutPoint.value_or(windowPoint);
    pointer._LeftButtonPressed = PlatformBridge::Input::IsMouseButtonDown(PlatformBridge::MouseButton::Left);
    pointer._RightButtonPressed = PlatformBridge::Input::IsMouseButtonDown(PlatformBridge::MouseButton::Right);
    Window::ReadWheel(_parentWindow, pointer); //the view's window's wheel

    filterThroughAncestors(_parentWidget, this, pointer, world);
    IDropDown::CaptureUnderOpenList(this, _parentWindow, pointer);
    return pointer;
}

bool RetroFuturaGUI::IWidget::isInside(const IHierarchyNode* node) const
{
    for(const IHierarchyNode* ancestor { _parentWidget }; ancestor; ancestor = ancestor->GetParent())
    {
        if(ancestor == node)
            return true;
    }

    return false;
}

void RetroFuturaGUI::IWidget::filterThroughAncestors(const IHierarchyNode* node, const IHierarchyNode* reader, PointerState& pointer, const IHierarchyNode* top)
{
    if(!node)
        return;

    if(node != top)
        filterThroughAncestors(node->GetParent(), reader, pointer, top);

    const IPointerFilter* filter { dynamic_cast<const IPointerFilter*>(node) };

    if(!filter)
        return;

    filter->FilterPointer(pointer, reader);
}

void RetroFuturaGUI::IWidget::SetTooltipText(std::string_view text)
{
    if(!_tooltip)
        _tooltip = std::make_unique<Tooltip>(&_projection, this);

    if(!_tooltip)
        return;

    _tooltip->GetText().SetString(text, false);
}

void RetroFuturaGUI::IWidget::ShowTooltip(const bool show)
{
    _enableTooltip = show;
}

RetroFuturaGUI::Tooltip* RetroFuturaGUI::IWidget::GetTooltip() const
{
    return _tooltip.get();
}

void RetroFuturaGUI::IWidget::Interact()
{
    followSceneEnabled(); //before the input, so a widget in a scene disabled this frame takes none
    const Shader::FrameScope frame { getTilt(), Shader::FrameComposition::Inside };
    _frame = Shader::GetDrawFrame();
    updateTooltip();
    interact();
}

void RetroFuturaGUI::IWidget::Update()
{
    followSceneEnabled(); //hidden widgets don't interact, so they follow here
    followFlavors(); //before the first frame counts, so flavors taken at the start show at once.
    _hasUpdated = true;
    updateGeometry();
    updateBorderGap();
    update();
}

void RetroFuturaGUI::IWidget::updateTooltip()
{
    if(_enableTooltip && _tooltip)
        _tooltip->Update();

    updateContextMenu();
}

void RetroFuturaGUI::IWidget::AddContextMenuItem(std::string_view itemName, std::string_view node, std::string_view imagePath, std::string_view text, std::string_view note)
{
    if(!_contextMenu)
        _contextMenu = std::make_unique<ContextMenu>(&_projection, this);

    if(!_contextMenu)
        return;

    _contextMenu->AddItem(itemName, node, imagePath, text, note);
}

void RetroFuturaGUI::IWidget::AddContextMenuNode(std::string_view newNodeName, std::string_view node)
{
    if(!_contextMenu)
        _contextMenu = std::make_unique<ContextMenu>(&_projection, this);

    if(!_contextMenu)
        return;

    _contextMenu->AddNode(newNodeName, node);
}

void RetroFuturaGUI::IWidget::SetTransition(const TransitionProperty properties, const Transition& transition)
{
    Transitions::Override(_ownTransitions, properties, transition);
    resolveTransitions();
}

void RetroFuturaGUI::IWidget::ResetTransition(const TransitionProperty properties)
{
    Transitions::Reset(_ownTransitions, properties);
    resolveTransitions();
}

RetroFuturaGUI::Transition RetroFuturaGUI::IWidget::GetTransition(const TransitionProperty property) const
{
    return Transitions::Resolve(_transitionSettings, property);
}

void RetroFuturaGUI::IWidget::useTransitions(std::initializer_list<FillStyle*> elements)
{
    for(FillStyle* element : elements)
    {
        if(!element)
            continue;

        element->SetTransitionSettings(&_transitionSettings);
    }
}

RetroFuturaGUI::ContextMenu* RetroFuturaGUI::IWidget::GetContextMenu() const
{
    return _contextMenu.get();
}

void RetroFuturaGUI::IWidget::updateContextMenu()
{
    if(!_contextMenu)
        return;

    _contextMenu->Update();
}

void RetroFuturaGUI::IWidget::SetBorderGap(const glm::vec4& gap)
{
    _ownBorderGap = gap;
    _borderGap = gap;
    _hasBorderGap = true;
    setBackgroundInset(true);
    updateBorderInwardGlow();
}

void RetroFuturaGUI::IWidget::ResetBorderGap()
{
    _ownBorderGap.reset();
    resolveBorderGap();
}

const glm::vec4& RetroFuturaGUI::IWidget::GetBorderGap() const
{
    return _borderGap;
}

RetroFuturaGUI::StyleKey RetroFuturaGUI::IWidget::styleKey()
{
    return StyleKey {};
}

void RetroFuturaGUI::IWidget::useFlavors(IWidgetDecoration* background, IWidgetDecoration* border, WidgetText* text)
{
    _styledBackground = background;
    _styledBorder = border;
    _styledText = text;

    if(_styledBackground)
        _styledBackground->setStyleSource(&_flavorHashes, StylePart::Background);

    if(_styledBorder)
        _styledBorder->setStyleSource(&_flavorHashes, StylePart::Border);

    if(_styledText)
        _styledText->setStyleSource(&_flavorHashes, StylePart::Text);
}

void RetroFuturaGUI::IWidget::useSubPartFlavors(IWidgetDecoration& part, const StylePart stylePart)
{
    _styledSubParts.push_back(&part);
    part.setStyleSource(&_flavorHashes, stylePart);
}

void RetroFuturaGUI::IWidget::useSubPartFlavors(WidgetText& text, const StylePart stylePart)
{
    _styledSubTexts.push_back(&text);
    text.setStyleSource(&_flavorHashes, stylePart);
}

void RetroFuturaGUI::IWidget::useSubPartFlavors(IStyledPart& part, const StylePart stylePart)
{
    _styledOtherParts.push_back(&part);
    part.setStyleSource(&_flavorHashes, stylePart);
}

void RetroFuturaGUI::IWidget::AddFlavor(std::string_view name)
{
    if(HasFlavor(name))
        return;

    _flavorNames.emplace_back(name);
    _flavorHashes.push_back(StylingManager::GetFlavorHash(name));
    restyleParts();
}

void RetroFuturaGUI::IWidget::RemoveFlavor(std::string_view name)
{
    const auto found { std::ranges::find(_flavorHashes, StylingManager::GetFlavorHash(name)) };

    if(_flavorHashes.end() == found)
        return;

    _flavorNames.erase(_flavorNames.begin() + (found - _flavorHashes.begin()));
    _flavorHashes.erase(found);
    restyleParts();
}

bool RetroFuturaGUI::IWidget::HasFlavor(std::string_view name) const
{
    return std::ranges::contains(_flavorHashes, StylingManager::GetFlavorHash(name));
}

const std::vector<std::string>& RetroFuturaGUI::IWidget::GetFlavors() const
{
    return _flavorNames;
}

void RetroFuturaGUI::IWidget::restyleParts()
{
    _styleGeneration = StylingManager::GetGeneration();
    resolveTransitions();
    resolveBorderGap();

    for(IWidgetDecoration* part : _styledSubParts)
        part->restyle();

    for(WidgetText* text : _styledSubTexts)
        text->setStyleSource(&_flavorHashes, text->_stylePart);

    for(IStyledPart* part : _styledOtherParts)
        part->setStyleSource(&_flavorHashes, part->getStylePart());

    if(_styledBackground)
        _styledBackground->restyle();

    if(_styledBorder)
        _styledBorder->restyle();

    if(_styledText)
        _styledText->setStyleSource(&_flavorHashes, StylePart::Text);
}

void RetroFuturaGUI::IWidget::useBorderGap(Background* background, Border* border)
{
    if(!background)
        return;

    if(!border)
        return;

    _gapBackground = background;
    _gapBorder = border;
    background->_linkedBorder = border;
    border->_linkedBackground = background;
}

void RetroFuturaGUI::IWidget::updateBorderGap()
{
    if(!_gapBackground)
        return;

    if(_hasBorderGap && getGapBorderWidth() != _gapBorderWidth)
        setBackgroundInset(false); //border widths change at once, so the gap follows them at once.

    if(!_isInsetMoving)
        return;

    const glm::vec4 inset { _backgroundInset.Get() };
    _isInsetMoving = _backgroundInset.IsRunning();
    _gapBackground->setInset(inset);
}

void RetroFuturaGUI::IWidget::setBackgroundInset(const bool animate)
{
    if(!_gapBackground)
        return;

    _gapBorderWidth = getGapBorderWidth();
    const glm::vec4 target { _hasBorderGap ? glm::vec4(_gapBorderWidth) + _borderGap : glm::vec4(0.0f) }; //without a gap it spans the whole widget.
    const Transition transition { Transitions::Resolve(_transitionSettings, TransitionProperty::BorderGap) };

    //Before the first frame, without a transition or inside an InstantScope it applies at once, as a plain setter would
    if(!animate || !_hasUpdated || 0 >= transition._Duration.count())
    {
        _isInsetMoving = false;
        _backgroundInset.Jump(target);
        _gapBackground->setInset(target);
        return;
    }

    if(!_isInsetMoving)
        _backgroundInset.Jump(_gapBackground->_inset); //from where it is now, the whole widget the first time

    _backgroundInset.Set(target, transition);
    _isInsetMoving = true;
}

f32 RetroFuturaGUI::IWidget::getGapBorderWidth() const
{
    if(!_gapBorder)
        return 0.0f;

    return _gapBorder->GetWidth();
}

void RetroFuturaGUI::IWidget::updateBorderInwardGlow()
{
    if(!_gapBorder)
        return;

    _gapBorder->_hasBackgroundGap = _hasBorderGap && glm::any(glm::greaterThan(_borderGap, glm::vec4(0.0f)));
}
void RetroFuturaGUI::IWidget::followFlavors()
{
    if(StylingManager::GetGeneration() == _styleGeneration)
        return;

    _styleGeneration = StylingManager::GetGeneration();
    resolveTransitions();
    resolveBorderGap();
}

void RetroFuturaGUI::IWidget::resolveTransitions()
{
    StylingManager::ResolveTransitions(_ownTransitions, _flavorHashes, _transitionSettings);
}

void RetroFuturaGUI::IWidget::resolveBorderGap()
{
    const std::optional<glm::vec4> gap { _ownBorderGap ? _ownBorderGap : StylingManager::FindFirst<std::optional<glm::vec4>>(_flavorHashes, [](const Flavor& flavor) -> const std::optional<glm::vec4>& { return flavor.GetBorderGap(); }) };

    if(gap.has_value() == _hasBorderGap && (!gap || *gap == _borderGap))
        return;

    _borderGap = gap.value_or(glm::vec4(0.0f));
    _hasBorderGap = gap.has_value();
    setBackgroundInset(true);
    updateBorderInwardGlow();
}
