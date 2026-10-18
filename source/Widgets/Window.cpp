#include "Window.hpp"
#include "Scene.hpp"
#include "ShaderManager.hpp"
#include "PlatformBridge.hpp"
#include "Tooltip.hpp"
#include <glm/gtc/type_ptr.hpp>
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <print>
#include <utility>

#if defined(TARGET_PLATFORM_LINUX)
    #define GLFW_EXPOSE_NATIVE_X11
#elif defined(TARGET_PLATFORM_WINDOWS)
    #define GLFW_EXPOSE_NATIVE_WIN32
#endif
#include <GLFW/glfw3native.h>

RetroFuturaGUI::Window::Window(std::string_view name, const i32 width, const i32 height)
: Window(name, width, height, WindowRole::Normal)
{
}

RetroFuturaGUI::Window::Window(std::string_view name, const i32 width, const i32 height, const WindowRole role)
: _width(width), _height(height), _name(name)
{
	_role = role;
	createWindow();

	//Docking previews and popups are never docking targets
	if(WindowRole::Normal == _role)
		allWindows().push_back(this);

	if(_cursorsInitialized)
		return;
		
	_resizeCursorHorizontal = glfwCreateStandardCursor(GLFW_HRESIZE_CURSOR);
	_resizeCursorVertical = glfwCreateStandardCursor(GLFW_VRESIZE_CURSOR);
	_resizeCursorTLBR = glfwCreateStandardCursor(GLFW_RESIZE_NWSE_CURSOR);
	_resizeCursorTRBL = glfwCreateStandardCursor(GLFW_RESIZE_NESW_CURSOR);
	_defaultCursor = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
	_cursorsInitialized = true;
}

RetroFuturaGUI::Window::~Window()
{
	//Popups go from the top down, the reverse of how they opened, which a vector's own destruction wouldn't do. Their results
	//don't go out: the slots may be gone by now.
	while(!_popups.empty())
		_popups.pop_back();

	std::erase(allWindows(), this);
	Undock();

	if(!_window)
		return;

	//The background's and the border's GL objects belong to this window's context, so they go while it still exists and is current
	glfwMakeContextCurrent(_window);
	_background.reset();
	_backgroundImage.reset();
	_border.reset();

	//A later window must not try to share with a destroyed one
	if(_shareWindow == _window)
		_shareWindow = nullptr;

	glfwDestroyWindow(_window);
}

void RetroFuturaGUI::Window::createWindow()
{
	//Hints outlive the window they were set for, so every one this window depends on is set each time
	glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

	//enable transparency on cut out corners
	glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);
	glfwWindowHint(GLFW_ALPHA_BITS, 8);

	//A docking preview shows only once it is placed, stays above the windows it previews and never takes the focus. A popup
	//shows once it is centered and has drawn, and takes the focus then.
	glfwWindowHint(GLFW_VISIBLE, WindowRole::Normal == _role ? GLFW_TRUE : GLFW_FALSE);
	glfwWindowHint(GLFW_FLOATING, WindowRole::DockPreview == _role ? GLFW_TRUE : GLFW_FALSE);
	glfwWindowHint(GLFW_FOCUS_ON_SHOW, WindowRole::DockPreview == _role ? GLFW_FALSE : GLFW_TRUE);

	//Shares GL objects with the first window, so the shaders compiled once below and the fonts and images loaded there work in this one too.
	//VAOs are never shared, so a widget still has to be built, drawn and destroyed with its own window's context current.
	_window = glfwCreateWindow(_width, _height, _windowTitle.c_str(), nullptr, _shareWindow);
	glfwSetWindowTitle(_window, _windowTitle.c_str());

	if (!_window)
	{
		std::println("{0}", "Failed to create GLFW window");
		glfwTerminate();
		return;
	}

	if(!_shareWindow)
		_shareWindow = _window;

	//Lets a window bar, which only gets the GLFW window, reach this one, e.g. to undock it
	glfwSetWindowUserPointer(_window, this);

	_prevResizeX = (f64)_width;
	_prevResizeY = (f64)_height;

	glfwMakeContextCurrent(_window);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::println("Failed to initialize GLAD in Window::createWindow");
        glfwDestroyWindow(_window);
        _window = nullptr;
        glfwTerminate();
        return;
    }

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f); //what is left where nothing draws, the corners outside the background: invisible
	glViewport(0, 0, _width, _height);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::println("Failed to initialize GLAD");
        return;
    }

	static bool shadersInitialized = false;
	if (!shadersInitialized)		
	{
		ShaderManager::Init();
		shadersInitialized = true;
	}

	_projection = std::make_unique<Projection>((float)_width, (float)_height);

	_background = std::make_unique<Rectangle>(_projection.get());

	if(_background)
	{
		_background->SetRectangleMode(RectangleMode::Plane);
		_background->SetPosition(glm::vec3((f32)_width * 0.5f, (f32)_height * 0.5f, -1.0f));
		_background->SetSize(glm::vec2((f32)_width, (f32)_height));
		_background->SetRotation(glm::vec3(0.0f));
	}

	_border = std::make_unique<Rectangle>(_projection.get());

	if(_border)
	{
		_border->SetRectangleMode(RectangleMode::Border);
		_border->SetPosition(glm::vec3((f32)_width * 0.5f, (f32)_height * 0.5f, 0.0f));
		_border->SetSize(glm::vec2((f32)_width, (f32)_height));
		_border->SetRotation(glm::vec3(0.0f));
		//No frame until one is asked for, since a Rectangle starts with a 5 px border
		_border->SetBorderWidth(0.0f);
	}

	SetBackgroundColor(glm::vec4(0.086f, 0.086f, 0.1137f, 1.0f), ColorState::Enabled);
	setBorderColors();
}

void RetroFuturaGUI::Window::SetWindowTitle(std::string_view title)
{
	if(!_window)
		return;

	_windowTitle = title;
	glfwSetWindowTitle(_window, _windowTitle.c_str());
}

bool RetroFuturaGUI::Window::getCursorPosition(i32& x, i32& y) const
{
#if defined(TARGET_PLATFORM_LINUX)
	return PlatformBridge::Input::GetMouseWindowPosition(glfwGetX11Window(_window), x, y);
#elif defined(TARGET_PLATFORM_WINDOWS)
	return PlatformBridge::Input::GetMouseWindowPosition(glfwGetWin32Window(_window), x, y);
#else
	return false;
#endif
}

void RetroFuturaGUI::Window::pollInput()
{
	setCursorPosition();
	setCursorIcon();

	const bool isMouseButtonDown { PlatformBridge::Input::IsMouseButtonDown(PlatformBridge::MouseButton::Left) };

	//Once the button is up, a press held from before this window took the mouse is over
	if(!isMouseButtonDown)
		_ignoreHeldPress = false;

	if (isMouseButtonDown && !_wasMouseButtonDown && !_ignoreHeldPress) //press
		setResizeState(GLFW_MOUSE_BUTTON_LEFT, GLFW_PRESS, 0);
	else if (!isMouseButtonDown && _wasMouseButtonDown) //release
		setResizeState(GLFW_MOUSE_BUTTON_LEFT, GLFW_RELEASE, 0);

	_wasMouseButtonDown = isMouseButtonDown;

	resize();
}

void RetroFuturaGUI::Window::setCursorPosition()
{
	i32 x { 0 }, y { 0 };
	getCursorPosition(x, y);
	_cursorPosX = x;
	_cursorPosY = y;
}

void RetroFuturaGUI::Window::setCursorIcon()
{
	ResizeEdge edge { resizeEdgeAt(_cursorPosX, _cursorPosY) };

	if(!canResize(edge))
		edge = ResizeEdge::None;

	switch(edge)
	{
		case ResizeEdge::TopLeft:
		case ResizeEdge::BottomRight:
			glfwSetCursor(_window, _resizeCursorTLBR);
		break;
		case ResizeEdge::TopRight:
		case ResizeEdge::BottomLeft:
			glfwSetCursor(_window, _resizeCursorTRBL);
		break;
		case ResizeEdge::Left:
		case ResizeEdge::Right:
			glfwSetCursor(_window, _resizeCursorHorizontal);
		break;
		case ResizeEdge::Top:
		case ResizeEdge::Bottom:
			glfwSetCursor(_window, _resizeCursorVertical);
		break;
		default:
			glfwSetCursor(_window, _defaultCursor);
		break;
	}
}

RetroFuturaGUI::Window::ResizeEdge RetroFuturaGUI::Window::resizeEdgeAt(const f64 x, const f64 y) const
{
	const bool
		nearLeft { x < _boundaryThreshold },
		nearRight { x > _width - _boundaryThreshold },
		nearTop { y < _boundaryThreshold },
		nearBottom { y > _height - _boundaryThreshold };

	//Corners first, so the cursor near one resizes both of its edges
	if(nearTop && nearLeft)
		return ResizeEdge::TopLeft;

	if(nearTop && nearRight)
		return ResizeEdge::TopRight;

	if(nearBottom && nearLeft)
		return ResizeEdge::BottomLeft;

	if(nearBottom && nearRight)
		return ResizeEdge::BottomRight;

	if(nearLeft)
		return ResizeEdge::Left;

	if(nearRight)
		return ResizeEdge::Right;

	if(nearTop)
		return ResizeEdge::Top;

	if(nearBottom)
		return ResizeEdge::Bottom;

	return ResizeEdge::None;
}

bool RetroFuturaGUI::Window::canResize(const ResizeEdge edge) const
{
	if(IsBlockedByPopup())
		return false;

	if(!_dockHost)
		return true;

	switch(_dockHostEdge)
	{
		case DockEdge::Top:
			return ResizeEdge::Top == edge;
		case DockEdge::Right:
			return ResizeEdge::Right == edge;
		case DockEdge::Bottom:
			return ResizeEdge::Bottom == edge;
		case DockEdge::Left:
			return ResizeEdge::Left == edge;
		default:
			return false;
	}
}

void RetroFuturaGUI::Window::setResizeState(i32 button, i32 action, [[maybe_unused]] i32 mods)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
	{
        i32 x { 0 }, y { 0 };
        getCursorPosition(x, y);
        glfwGetWindowPos(_window, &_windowPosX, &_windowPosY);
        _absoluteCursorPosX = x + _windowPosX;
        _absoluteCursorPosY = y + _windowPosY;
        _cursorPosX = x;
        _cursorPosY = y;
        _prevResizeX = _absoluteCursorPosX;
        _prevResizeY = _absoluteCursorPosY;

		//The button is read for the whole screen, so a press in another window arrives here too. Outside this window every
		//position would count as near an edge, so only a press inside it may start a resize.
		if(0.0 > _cursorPosX || 0.0 > _cursorPosY || static_cast<f64>(_width) <= _cursorPosX || static_cast<f64>(_height) <= _cursorPosY)
			return;

		_resizeEdge = resizeEdgeAt(_cursorPosX, _cursorPosY);

		if(!canResize(_resizeEdge))
			_resizeEdge = ResizeEdge::None;

		if (_resizeEdge != ResizeEdge::None)
			_isResizing = true;
	} 
	else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
	{
		_isResizing = false;
		_resizeEdge = ResizeEdge::None;
	}
}

void RetroFuturaGUI::Window::resize()
{
	if (!_isResizing) 
		return;

    i32 rx { 0 }, ry { 0 };
    getCursorPosition(rx, ry);
    glfwGetWindowPos(_window, &_windowPosX, &_windowPosY);
    _absoluteCursorPosX = rx + _windowPosX;
    _absoluteCursorPosY = ry + _windowPosY;

    f64
		deltaX { _absoluteCursorPosX - _prevResizeX },
     	deltaY { _absoluteCursorPosY - _prevResizeY };
    i32
		newWidth { _width },
     	newHeight { _height },
     	newPosX { _windowPosX },
     	newPosY { _windowPosY };

    switch(_resizeEdge)
    {
        case ResizeEdge::TopLeft:
            newWidth = _width - (i32)deltaX;
            newHeight = _height - (i32)deltaY;
            newPosX = _windowPosX + (i32)deltaX;
            newPosY = _windowPosY + (i32)deltaY;
        break;
        case ResizeEdge::TopRight:
            newWidth = _width + (i32)deltaX;
            newHeight = _height - (i32)deltaY;
            newPosY = _windowPosY + (i32)deltaY;
        break;
        case ResizeEdge::BottomLeft:
            newWidth = _width - (i32)deltaX;
            newHeight = _height + (i32)deltaY;
            newPosX = _windowPosX + (i32)deltaX;
        break;
        case ResizeEdge::BottomRight:
            newWidth = _width + (i32)deltaX;
            newHeight = _height + (i32)deltaY;
        break;
        case ResizeEdge::Left:
            newWidth = _width - (i32)deltaX;
            newPosX = _windowPosX + (i32)deltaX;
        break;
        case ResizeEdge::Right:
            newWidth = _width + (i32)deltaX;
        break;
        case ResizeEdge::Top:
            newHeight = _height - (i32)deltaY;
            newPosY = _windowPosY + (i32)deltaY;
        break;
        case ResizeEdge::Bottom:
            newHeight = _height + (i32)deltaY;
        break;
		default:
		break;
    }

    _prevResizeX = _absoluteCursorPosX;
    _prevResizeY = _absoluteCursorPosY;
    _width = std::max<i32>(newWidth, _minWindowSpan);
    _height = std::max<i32>(newHeight, _minWindowSpan);
    _windowPosX = newPosX;
    _windowPosY = newPosY;
    glfwSetWindowPos(_window, _windowPosX, _windowPosY);
    glfwSetWindowSize(_window, _width, _height);
	_windowSizeChanged = true;
}

void RetroFuturaGUI::Window::updateProjection()
{ 
	glfwGetWindowSize(_window, &_width, &_height);

	if (_lastSize == glm::ivec2(_width, _height))
		return;
		
	_projection->UpdateProjectionMatrix((f32)_width, (f32)_height);
	glViewport(0, 0, _width, _height);

	UpdateLayout();

	if(_background)
	{
		_background->SetSize(glm::vec2((f32)_width, (f32)_height));
		_background->SetPosition(glm::vec3((f32)_width * 0.5f, (f32)_height * 0.5f, -1.0f));
	}

	if(_border)
	{
		_border->SetSize(glm::vec2((f32)_width, (f32)_height));
		_border->SetPosition(glm::vec3((f32)_width * 0.5f, (f32)_height * 0.5f, 0.0f));
	}

	_lastSize = { _width, _height };
	_windowSizeChanged = false;
}

bool RetroFuturaGUI::Window::WindowShouldClose()
{
	if(!_window)
		return true;

	refuseCloseWhileBlocked();
	return GLFW_TRUE == glfwWindowShouldClose(_window);
}

void RetroFuturaGUI::Window::Draw()
{
	if(!_window)
		return;

	//Another window may have drawn last, and every GL call below has to land in this window's context
	glfwMakeContextCurrent(_window);

	if(WindowRole::DockPreview != _role)
		pollInput();

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//Color blends as usual. Alpha only ever adds up, so a semi-transparent widget on the opaque background leaves the window
	//opaque there, instead of lowering its alpha and letting the desktop show through. The result is premultiplied by alpha,
	//which is what the compositor expects of a transparent framebuffer.
	glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

	updateProjection();
	applyCornerRadii();
	drawBackground();

	if(_lasagna)
		_lasagna->Draw();

	for(Scene* _scene : _scenes)
	{
		if(!_scene)
			continue;

		//Each scene starts with a fresh depth buffer, so a scene added later always draws over the ones loaded before
		glClear(GL_DEPTH_BUFFER_BIT);
		_scene->Draw();
	}

	drawFrame();
	Tooltip::DrawShown(); //last, so it covers every scene
	glfwSwapBuffers(_window);
	glfwPollEvents();
	refuseCloseWhileBlocked();
	followDocking();//After the scenes, because a window bar may have dragged this window while it drew
	updateDockSnap();
	drawPopups();
}

void RetroFuturaGUI::Window::drawFrame()
{
	if(!_border)
		return;

	//Width 0, the default, means no frame
	if(0.0f >= _border->GetBorderWidth())
		return;

	//A fresh depth buffer, so the frame lies over every scene
	glClear(GL_DEPTH_BUFFER_BIT);
	drawBorder();
}

void RetroFuturaGUI::Window::SetWindowSize(i32 width, i32 height)
{
	SetWidth(width);
	SetHeight(height);
}

void RetroFuturaGUI::Window::SetWidth(i32 width)
{
	_width = width;
	glfwSetWindowSize(_window, _width, _height);
}

void RetroFuturaGUI::Window::SetHeight(i32 height)
{
	_height = height;
	glfwSetWindowSize(_window, _width, _height);
}

void RetroFuturaGUI::Window::SetBackgroundImage(std::string_view imagePath)
{
	IBackground::SetBackgroundImage(imagePath);
}

void RetroFuturaGUI::Window::SetLasagna(Lasagna* lasagna)
{
	_lasagna = lasagna;
	UpdateLayout();
}

void RetroFuturaGUI::Window::AddScene(Scene* scene)
{
	if(!scene)
		return;

	if(std::find(_scenes.begin(), _scenes.end(), scene) != _scenes.end())
		return;

	_scenes.push_back(scene);
	UpdateLayout();
}

void RetroFuturaGUI::Window::RemoveScene(Scene* scene)
{
	if(!scene)
		return;

	const std::vector<Scene*>::iterator _found { std::find(_scenes.begin(), _scenes.end(), scene) };

	if(_found == _scenes.end())
		return;

	_scenes.erase(_found);
	UpdateLayout();
}

const RetroFuturaGUI::ScreenRect& RetroFuturaGUI::Window::GetClientRect() const
{
	return _clientRect;
}

RetroFuturaGUI::ScreenRect RetroFuturaGUI::Window::reserveEdge(const DockEdge edge, const f32 thickness, ScreenRect& client)
{
	const bool horizontal { edge == DockEdge::Left || edge == DockEdge::Right };
	const f32 taken { std::clamp(thickness, 0.0f, horizontal ? client._Extent.x : client._Extent.y) };

	ScreenRect strip { ._Origin = client._Origin, ._Extent = client._Extent };

	switch(edge)
	{
	case DockEdge::Top:
		strip._Origin.y = client._Origin.y + client._Extent.y - taken;
		strip._Extent.y = taken;
		client._Extent.y -= taken;
	break;
	case DockEdge::Bottom:
		strip._Extent.y = taken;
		client._Origin.y += taken;
		client._Extent.y -= taken;
	break;
	case DockEdge::Left:
		strip._Extent.x = taken;
		client._Origin.x += taken;
		client._Extent.x -= taken;
	break;
	case DockEdge::Right:
		strip._Origin.x = client._Origin.x + client._Extent.x - taken;
		strip._Extent.x = taken;
		client._Extent.x -= taken;
	break;
	}

	return strip;
}

void RetroFuturaGUI::Window::UpdateLayout()
{
	ScreenRect client { ._Origin = glm::vec2(0.0f), ._Extent = glm::vec2((f32)_width, (f32)_height) };

	//Docked scenes take their strip in the order they were added, so the first one to claim an edge also owns the corner where two claims meet.
	for(Scene* _scene : _scenes)
	{
		if(!_scene || !_scene->IsActive() || !_scene->HasReservedEdge())
			continue;

		const ScreenRect strip { reserveEdge(_scene->GetReservedEdge(), _scene->GetReservedThickness(), client) };

		_scene->SetPosition(glm::vec3(strip.Center(), _scene->GetPosition().z));
		_scene->SetSize(glm::vec3(strip._Extent, _scene->GetSize().z));
	}

	_clientRect = client;

	//Overlay scenes and the root Lasagna share whatever the docked scenes left.
	for(Scene* _scene : _scenes)
	{
		if(!_scene || _scene->HasReservedEdge())
			continue;

		_scene->SetPosition(glm::vec3(client.Center(), _scene->GetPosition().z));
		_scene->SetSize(glm::vec3(client._Extent, _scene->GetSize().z));
	}

	if(!_lasagna)
		return;

	_lasagna->SetPosition(glm::vec3(client.Center(), _lasagna->GetPosition().z));
	_lasagna->SetSize(glm::vec3(client._Extent, _lasagna->GetSize().z));
}

i32 RetroFuturaGUI::Window::GetBackgroundImageId() const
{
	if(_backgroundImage)
    	return _backgroundImage->GetID();

	return -1;
}

GLFWwindow* RetroFuturaGUI::Window::GetGlfwWindow() const
{
    return _window;
}

RetroFuturaGUI::Projection* RetroFuturaGUI::Window::GetProjection() const
{
    return &*_projection;
}

const std::string& RetroFuturaGUI::Window::GetName() const
{
	return _name;
}

const RetroFuturaGUI::IHierarchyNode* RetroFuturaGUI::Window::GetParent() const
{
	return nullptr;
}

void RetroFuturaGUI::Window::ToggleMaximize()
{
    if(!_window)
        return;

    if(IsMaximized())
        glfwRestoreWindow(_window);
    else
        glfwMaximizeWindow(_window);
}

bool RetroFuturaGUI::Window::IsMaximized() const
{
    if(!_window)
        return false;

    return GLFW_TRUE == glfwGetWindowAttrib(_window, GLFW_MAXIMIZED);
}

void RetroFuturaGUI::Window::SetCornerRadii(const glm::vec4& radii)
{
	_cornerRadii = radii;
}

bool RetroFuturaGUI::Window::DockWindow(Window* window, const DockEdge edge)
{
	return dockWindowAt(window, edge, dockedWindows(edge).size());
}

bool RetroFuturaGUI::Window::dockWindowAt(Window* window, const DockEdge edge, const uSize index)
{
	if(!window)
		return false;

	if(!window->_window)
		return false;

	if(window == this)
		return false;

	//Docking previews and popups never dock
	if(WindowRole::Normal != _role || WindowRole::Normal != window->_role)
		return false;

	//A window has one host at most, the window whose edge sets its length and position, and never one docked to it, so every
	//group stays a tree. It can still border more windows: the ones docked to its own edges, and its neighbors on the host's edge.
	if(window->_dockHost)
		return false;

	if(isDockedTo(window))
		return false;

	glm::ivec2 size { 0 };
	glfwGetWindowSize(window->_window, &size.x, &size.y);

	window->_dockHost = this;
	window->_dockHostEdge = edge;
	//The length this window's edge takes over, so undocking can give it back
	window->_undockedLength = DockEdge::Left == edge || DockEdge::Right == edge ? size.y : size.x;

	std::list<Window*>& docked { dockedWindows(edge) };
	docked.insert(std::next(docked.begin(), static_cast<i64>((std::min)(index, docked.size()))), window);

	layoutDockedWindows();
	return true;
}

void RetroFuturaGUI::Window::Undock()
{
	leaveDockHost();
	releaseDockedWindows();
}

bool RetroFuturaGUI::Window::IsDocked() const
{
	return _dockHost != nullptr
		|| !_topDockedWindows.empty()
		|| !_rightDockedWindows.empty()
		|| !_bottomDockedWindows.empty()
		|| !_leftDockedWindows.empty();
}

void RetroFuturaGUI::Window::SetDockingPadding(const f32 padding)
{
	_dockingPadding = (std::max)(padding, 0.0f);
    dockRoot()->layoutDockedWindows();
}

void RetroFuturaGUI::Window::SetNoCornerRadiiWhenDocked(const bool noCornerRadii)
{
	_noCornerRadiiWhenDocked = noCornerRadii;
}

RetroFuturaGUI::Window* RetroFuturaGUI::Window::FromGlfwWindow(GLFWwindow* glfwWindow)
{
	if(!glfwWindow)
		return nullptr;

	return static_cast<Window*>(glfwGetWindowUserPointer(glfwWindow));
}

std::list<RetroFuturaGUI::Window*>& RetroFuturaGUI::Window::dockedWindows(const DockEdge edge)
{
	return const_cast<std::list<Window*>&>(std::as_const(*this).dockedWindows(edge));
}

const std::list<RetroFuturaGUI::Window*>& RetroFuturaGUI::Window::dockedWindows(const DockEdge edge) const
{
	switch(edge)
	{
		case DockEdge::Right:
			return _rightDockedWindows;
		case DockEdge::Bottom:
			return _bottomDockedWindows;
		case DockEdge::Left:
			return _leftDockedWindows;
		default:
			return _topDockedWindows;
	}
}

RetroFuturaGUI::Window* RetroFuturaGUI::Window::dockRoot()
{
	Window* root { this };

	while(root->_dockHost)
		root = root->_dockHost;

	return root;
}

bool RetroFuturaGUI::Window::isDockedTo(const Window* window) const
{
	for(const Window* host { _dockHost }; host; host = host->_dockHost)
	{
		if(host == window)
			return true;
	}

	return false;
}

void RetroFuturaGUI::Window::leaveDockHost()
{
	if(!_dockHost)
		return;

	Window* host { _dockHost };
	host->dockedWindows(_dockHostEdge).remove(this);
	_dockHost = nullptr;
	restoreUndockedLength();
	host->layoutDockedWindows(); //The windows left on that edge share all of it again, so they grow into the gap
}

void RetroFuturaGUI::Window::releaseDockedWindows()
{
	for(const DockEdge edge : { DockEdge::Top, DockEdge::Right, DockEdge::Bottom, DockEdge::Left })
	{
		std::list<Window*>& docked { dockedWindows(edge) };

		for(Window* window : docked)
		{
			if(!window)
				continue;

			window->_dockHost = nullptr;
			window->restoreUndockedLength();
		}

		docked.clear();
	}
}

void RetroFuturaGUI::Window::restoreUndockedLength()
{
	const i32 length { _undockedLength };
	_undockedLength = 0;

	if(!_window)
		return;

	if(0 >= length)
		return;

	if(isMinimizedOrMaximized())
		return;

	glm::ivec2 size { 0 };
	glfwGetWindowSize(_window, &size.x, &size.y);

	if(DockEdge::Left == _dockHostEdge || DockEdge::Right == _dockHostEdge)
		size.y = length;
	else
		size.x = length;

	glfwSetWindowSize(_window, size.x, size.y);
}

void RetroFuturaGUI::Window::layoutDockedWindows()
{
	if(!_window)
		return;

	//Its docked windows stay where they are rather than stacking off screen or over it, until it is restored
	if(isMinimizedOrMaximized())
	{
		_dockingLayoutPending = true;
		return;
	}

	const DockRect host { screenRect() };
	_dockedPosition = host._Position;
	_dockedSize = host._Size;

	for(const DockEdge edge : { DockEdge::Top, DockEdge::Right, DockEdge::Bottom, DockEdge::Left })
	{
		const std::list<Window*>& docked { dockedWindows(edge) };
		const uSize count { docked.size() };
		const i32 gap { edgeGap(edge, nullptr) };
		uSize index { 0 };

		for(Window* window : docked)
		{
			const uSize slot { index };
			++index;

			if(!window)
				continue;

			if(!window->_window)
				continue;

			const DockRect placed { dockSlot(host, edge, slot, count, window->dockDepth(edge), gap) };
			window->setDockedGeometry(placed._Position, placed._Size);
			window->layoutDockedWindows();
		}
	}
}

RetroFuturaGUI::Window::DockRect RetroFuturaGUI::Window::screenRect() const
{
	DockRect rect {};

	if(!_window)
		return rect;

	glfwGetWindowPos(_window, &rect._Position.x, &rect._Position.y);
	glfwGetWindowSize(_window, &rect._Size.x, &rect._Size.y);
	return rect;
}

i32 RetroFuturaGUI::Window::edgeGap(const DockEdge edge, const Window* joining) const
{
	f32 padding { _dockingPadding };

	for(const Window* window : dockedWindows(edge))
	{
		if(!window)
			continue;

		padding = (std::max)(padding, window->_dockingPadding);
	}

	if(!joining)
		return static_cast<i32>(std::lround(padding));

	return static_cast<i32>(std::lround((std::max)(padding, joining->_dockingPadding)));
}

i32 RetroFuturaGUI::Window::dockDepth(const DockEdge edge) const
{
	const DockRect rect { screenRect() };
	return DockEdge::Left == edge || DockEdge::Right == edge ? rect._Size.x : rect._Size.y;
}

RetroFuturaGUI::Window::DockRect RetroFuturaGUI::Window::dockSlot(const DockRect& host, const DockEdge edge, const uSize index, const uSize count, const i32 depth, const i32 gap) const
{
	//The windows on an edge share its whole length, minus the gaps between them, and keep their own depth away from it
	const bool alongHeight { DockEdge::Left == edge || DockEdge::Right == edge };
	const i32
		edgeLength { alongHeight ? host._Size.y : host._Size.x }, //Screen coordinates, so y runs downward.
		windowCount { (std::max)(static_cast<i32>(count), 1) },
		slot { static_cast<i32>(index) },
		share { (edgeLength - gap * (windowCount - 1)) / windowCount },
		offset { slot * (share + gap) },
		//The last one takes what the division left over
		length { (std::max)(windowCount - 1 == slot ? edgeLength - offset : share, _minWindowSpan) };

	switch(edge)
	{
		case DockEdge::Top:
			return DockRect { ._Position = host._Position + glm::ivec2(offset, -gap - depth), ._Size = glm::ivec2(length, depth) };
		case DockEdge::Right:
			return DockRect { ._Position = host._Position + glm::ivec2(host._Size.x + gap, offset), ._Size = glm::ivec2(depth, length) };
		case DockEdge::Bottom:
			return DockRect { ._Position = host._Position + glm::ivec2(offset, host._Size.y + gap), ._Size = glm::ivec2(length, depth) };
		default:
			return DockRect { ._Position = host._Position + glm::ivec2(-gap - depth, offset), ._Size = glm::ivec2(depth, length) };
	}
}

void RetroFuturaGUI::Window::setDockedGeometry(const glm::ivec2& position, const glm::ivec2& size)
{
	if(!_window)
		return;

	if(isMinimizedOrMaximized())
	{
		_dockingLayoutPending = true;
		return;
	}

	glm::ivec2
		currentPosition { 0 },
		currentSize { 0 };
	glfwGetWindowPos(_window, &currentPosition.x, &currentPosition.y);
	glfwGetWindowSize(_window, &currentSize.x, &currentSize.y);

	if(size != currentSize)
		glfwSetWindowSize(_window, size.x, size.y);

	if(position != currentPosition)
		glfwSetWindowPos(_window, position.x, position.y);

	_dockedPosition = position;
	_dockedSize = size;
}

void RetroFuturaGUI::Window::followDocking()
{
	if(!_window)
		return;

	if(!IsDocked())
		return;

	if(isMinimizedOrMaximized())
		return;

	Window* root { dockRoot() };

	if(_dockingLayoutPending)
	{
		_dockingLayoutPending = false;
		root->layoutDockedWindows();
		return;
	}

	glm::ivec2
		position { 0 },
		size { 0 };
	glfwGetWindowPos(_window, &position.x, &position.y);
	glfwGetWindowSize(_window, &size.x, &size.y);

	if(position == _dockedPosition && size == _dockedSize)
		return;

	//Moved or resized by something other than the docking layout: a window bar drag, an edge drag or the system. A docked
	//window that was moved, not resized, carries its group along, so the root moves by as much and the layout then places
	//every other window around it. A resize moves nothing: the layout already puts a resized window where its new size goes.
	if(root != this && size == _dockedSize)
	{
		//A minimized or maximized root can't move
		if(root->isMinimizedOrMaximized())
		{
			_dockedPosition = position;
			root->_dockingLayoutPending = true;
			return;
		}

		const glm::ivec2 shift { position - _dockedPosition };
		glm::ivec2 rootPosition { 0 };
		glfwGetWindowPos(root->_window, &rootPosition.x, &rootPosition.y);
		glfwSetWindowPos(root->_window, rootPosition.x + shift.x, rootPosition.y + shift.y);
	}

	root->layoutDockedWindows();
}

bool RetroFuturaGUI::Window::isMinimizedOrMaximized() const
{
	if(!_window)
		return false;

	return GLFW_TRUE == glfwGetWindowAttrib(_window, GLFW_ICONIFIED) || GLFW_TRUE == glfwGetWindowAttrib(_window, GLFW_MAXIMIZED);
}

glm::bvec4 RetroFuturaGUI::Window::squaredCorners() const
{
	glm::bvec4 squared { false };

	for(const DockEdge edge : { DockEdge::Top, DockEdge::Right, DockEdge::Bottom, DockEdge::Left })
	{
		if(dockedWindows(edge).empty())
			continue;

		squareEdgeCorners(squared, edge);
	}

	if(!_dockHost)
		return squared;

	//The edge facing the host lies against it from end to end
	squareEdgeCorners(squared, oppositeEdge(_dockHostEdge));

	//Neighbors on the same edge of the host sit before and after this window, across the docking padding
	const std::list<Window*>& neighbors { _dockHost->dockedWindows(_dockHostEdge) };
	const bool alongHeight { DockEdge::Left == _dockHostEdge || DockEdge::Right == _dockHostEdge };

	if(neighbors.front() != this)
		squareEdgeCorners(squared, alongHeight ? DockEdge::Top : DockEdge::Left);

	if(neighbors.back() != this)
		squareEdgeCorners(squared, alongHeight ? DockEdge::Bottom : DockEdge::Right);

	return squared;
}

glm::vec4 RetroFuturaGUI::Window::GetDrawnCornerRadii() const
{
	glm::vec4 radii { _cornerRadii };

	if(!_noCornerRadiiWhenDocked)
		return radii;

	const glm::bvec4 squared { squaredCorners() };

	for(glm::length_t corner = 0; 4 > corner; ++corner)
	{
		if(squared[corner])
			radii[corner] = 0.0f;
	}

	return radii;
}

void RetroFuturaGUI::Window::applyCornerRadii()
{
	const glm::vec4 radii { GetDrawnCornerRadii() };

	if(radii == _appliedCornerRadii)
		return;

	_appliedCornerRadii = radii;
	setBackgroundCornerRadii(radii);
	setBorderCornerRadii(radii);
}

void RetroFuturaGUI::Window::squareEdgeCorners(glm::bvec4& squared, const DockEdge edge)
{
	switch(edge)
	{
		case DockEdge::Top:
			squared.x = true;
			squared.y = true;
		break;
		case DockEdge::Right:
			squared.y = true;
			squared.z = true;
		break;
		case DockEdge::Bottom:
			squared.z = true;
			squared.w = true;
		break;
		case DockEdge::Left:
			squared.w = true;
			squared.x = true;
		break;
		default:
		break;
	}
}

RetroFuturaGUI::DockEdge RetroFuturaGUI::Window::oppositeEdge(const DockEdge edge)
{
	switch(edge)
	{
		case DockEdge::Top:
			return DockEdge::Bottom;
		case DockEdge::Right:
			return DockEdge::Left;
		case DockEdge::Bottom:
			return DockEdge::Top;
		default:
			return DockEdge::Right;
	}
}

void RetroFuturaGUI::Window::BeginDrag()
{
	if(!_window)
		return;

	_isDragged = true;
	_dockSnap = {};
	glfwGetWindowPos(_window, &_dragStartPosition.x, &_dragStartPosition.y);
}

void RetroFuturaGUI::Window::EndDrag()
{
	if(!_isDragged)
		return;

	_isDragged = false;
	hideDockPreview();

	const DockSnap snap { _dockSnap };
	_dockSnap = {};

	if(!snap._Target)
		return;

	if(std::ranges::find(allWindows(), snap._Target) == allWindows().end())
		return;

	snap._Target->dockWindowAt(this, snap._Edge, snap._Index);
}

void RetroFuturaGUI::Window::updateDockSnap()
{
	if(!_isDragged)
		return;

	_dockSnap = findDockSnap();

	if(!_dockSnap._Target)
	{
		hideDockPreview();
		return;
	}

	//The slot this window would take, with the windows already on that edge making room for it
	const DockRect host { _dockSnap._Target->screenRect() };
	const uSize count { _dockSnap._Target->dockedWindows(_dockSnap._Edge).size() + 1 };
	const i32 gap { _dockSnap._Target->edgeGap(_dockSnap._Edge, this) };
	showDockPreview(_dockSnap._Target->dockSlot(host, _dockSnap._Edge, _dockSnap._Index, count, dockDepth(_dockSnap._Edge), gap), _dockSnap._Edge);
}

RetroFuturaGUI::Window::DockSnap RetroFuturaGUI::Window::findDockSnap() const
{
	if(!_window)
		return {};

	if(WindowRole::Normal != _role)
		return {}; //popups never dock

	//Only a window docked to nothing docks by dragging. Dragging a docked one carries its whole group along instead.
	if(_dockHost)
		return {};

	if(isMinimizedOrMaximized())
		return {};

	const DockRect dragged { screenRect() };

	//A click on the title is no drag: the window has to have moved
	if(dragged._Position == _dragStartPosition)
		return {};

	DockSnap snap {};
	i32 closest { _snapDistance + 1 };

	for(Window* target : allWindows())
	{
		if(!target)
			continue;

		if(target == this)
			continue;

		if(!target->_window)
			continue;

		//A window of its own group would make the group a loop
		if(target->isDockedTo(this))
			continue;

		if(target->isMinimizedOrMaximized())
			continue;

		if(GLFW_FALSE == glfwGetWindowAttrib(target->_window, GLFW_VISIBLE))
			continue;

		const DockRect host { target->screenRect() };

		for(const DockEdge edge : { DockEdge::Top, DockEdge::Right, DockEdge::Bottom, DockEdge::Left })
		{
			//Measured to where it would really sit, with the gap it would get there
			const i32 distance { dockDistance(dragged, host, target->edgeGap(edge, this), edge) };

			if(distance >= closest)
				continue;

			if(0 >= dockOverlap(dragged, host, edge))
				continue;

			closest = distance;
			snap = DockSnap { ._Target = target, ._Edge = edge, ._Index = target->dockIndexFor(dragged, edge) };
		}
	}

	return snap;
}

uSize RetroFuturaGUI::Window::dockIndexFor(const DockRect& dragged, const DockEdge edge) const
{
	//Along the edge: y beside the window, x above or below it
	const glm::length_t axis { DockEdge::Left == edge || DockEdge::Right == edge ? 1 : 0 };
	const i32 draggedMiddle { dragged._Position[axis] + dragged._Size[axis] / 2 };
	uSize index { 0 };

	for(const Window* window : dockedWindows(edge))
	{
		if(!window)
			continue;

		const DockRect docked { window->screenRect() };

		if(docked._Position[axis] + docked._Size[axis] / 2 < draggedMiddle)
			++index;
	}

	return index;
}

void RetroFuturaGUI::Window::showDockPreview(const DockRect& slot, const DockEdge edge)
{
	if(!_dockPreview)
	{
		_dockPreview = std::unique_ptr<Window>(new Window("DockPreview", slot._Size.x, slot._Size.y, WindowRole::DockPreview));
		_dockPreview->SetWindowTitle("Dock preview");
		_dockPreview->SetBackgroundColor(glm::vec4(0.3f, 0.55f, 1.0f, 0.25f), ColorState::Enabled);
		_dockPreview->SetBorderWidth(2.0f);
		_dockPreview->SetBorderColor(glm::vec4(0.45f, 0.7f, 1.0f, 0.85f), ColorState::Enabled);
	}

	if(!_dockPreview->_window)
		return;

	//Rounded like this window, with the corners that would meet the target squared like they will be once docked
	glm::vec4 radii { _cornerRadii };

	if(_noCornerRadiiWhenDocked)
	{
		glm::bvec4 squared { false };
		squareEdgeCorners(squared, oppositeEdge(edge));

		for(glm::length_t corner = 0; 4 > corner; ++corner)
		{
			if(squared[corner])
				radii[corner] = 0.0f;
		}
	}

	_dockPreview->SetCornerRadii(radii);

	const DockRect current { _dockPreview->screenRect() };

	if(slot._Size != current._Size)
		glfwSetWindowSize(_dockPreview->_window, slot._Size.x, slot._Size.y);

	if(slot._Position != current._Position)
		glfwSetWindowPos(_dockPreview->_window, slot._Position.x, slot._Position.y);

	_dockPreview->Draw(); //Drawn before it shows, so it never shows a frame it hasn't drawn

	if(GLFW_FALSE == glfwGetWindowAttrib(_dockPreview->_window, GLFW_VISIBLE))
		glfwShowWindow(_dockPreview->_window);
}

void RetroFuturaGUI::Window::hideDockPreview()
{
	if(!_dockPreview)
		return;

	if(!_dockPreview->_window)
		return;

	if(GLFW_FALSE == glfwGetWindowAttrib(_dockPreview->_window, GLFW_VISIBLE))
		return;

	glfwHideWindow(_dockPreview->_window);
}

i32 RetroFuturaGUI::Window::dockDistance(const DockRect& dragged, const DockRect& host, const i32 gap, const DockEdge edge)
{
	switch(edge)
	{
		case DockEdge::Top:
			return std::abs(dragged._Position.y + dragged._Size.y - (host._Position.y - gap));
		case DockEdge::Right:
			return std::abs(dragged._Position.x - (host._Position.x + host._Size.x + gap));
		case DockEdge::Bottom:
			return std::abs(dragged._Position.y - (host._Position.y + host._Size.y + gap));
		default:
			return std::abs(dragged._Position.x + dragged._Size.x - (host._Position.x - gap));
	}
}

i32 RetroFuturaGUI::Window::dockOverlap(const DockRect& dragged, const DockRect& host, const DockEdge edge)
{
	//Along the edge: y beside the host, x above or below it
	const glm::length_t axis { DockEdge::Left == edge || DockEdge::Right == edge ? 1 : 0 };
	const i32
		start { (std::max)(dragged._Position[axis], host._Position[axis]) },
		end { (std::min)(dragged._Position[axis] + dragged._Size[axis], host._Position[axis] + host._Size[axis]) };

	return end - start;
}

std::vector<RetroFuturaGUI::Window*>& RetroFuturaGUI::Window::allWindows()
{
	//Deliberately never freed: a window owned by a static is destroyed after every other static, and still takes itself off
	static std::vector<Window*>* windows { new std::vector<Window*>() };
	return *windows;
}

bool RetroFuturaGUI::Window::PushPopup(std::unique_ptr<IPopup> popup)
{
	if(!popup)
		return false;

	//A popup's window keeps no stack of its own: a popup opened from it goes on top of the stack it is on
	if(WindowRole::Popup == _role)
	{
		if(!_popupHost)
			return false;

		return _popupHost->PushPopup(std::move(popup));
	}

	if(!_window)
		return false;

	Window* popupWindow { popup->GetWindow() };

	if(!popupWindow)
		return false;

	if(!popupWindow->_window)
		return false;

	if(popupWindow->_popupHost)
		return false; //Already on a stack

	//It opens over the current top, which owns it natively
	Window* owner { topPopupWindow() };
	popupWindow->_popupHost = this;
	popupWindow->_ignoreHeldPress = true; //the click that opened it may still be down!
	popupWindow->_showAfterDraw = true;
	popupWindow->setNativeOwner(owner);
	popupWindow->centerOver(owner);

	_popups.push_back(std::move(popup));
	_popups.back()->restoreOpenerContext();
	return true;
}

bool RetroFuturaGUI::Window::HasPopups() const
{
	return !_popups.empty();
}

bool RetroFuturaGUI::Window::IsBlockedByPopup() const
{
	if(!_popups.empty())
		return true;

	if(!_popupHost)
		return false;

	//A popup's window is blocked unless it is the top of the stack
	return _popupHost->topPopupWindow() != this;
}

void RetroFuturaGUI::Window::FilterPointer(PointerState& pointer) const
{
	if(_ignoreHeldPress)
		pointer._LeftButtonPressed = false;

	//Only the capture, so a press stays visible and a text box below still ends its editing on a click elsewhere
	if(IsBlockedByPopup())
		pointer._IsCaptured = true;
}

void RetroFuturaGUI::Window::drawPopups()
{
	//By index: a button in a popup may open another popup while this loop runs, and push_back would invalidate an iterator
	for(uSize index = 0; index < _popups.size(); ++index)
	{
		IPopup* popup { _popups[index].get() };

		if(!popup)
			continue;

		Window* popupWindow { popup->GetWindow() };

		if(!popupWindow)
			continue;

		popupWindow->Draw();

		if(!popupWindow->_showAfterDraw)
			continue;

		//Shown only once it has drawn, so it never shows a frame it hasn't drawn
		popupWindow->_showAfterDraw = false;
		glfwShowWindow(popupWindow->_window);
	}

	closeTopPopup();
}

void RetroFuturaGUI::Window::closeTopPopup()
{
	if(_popups.empty())
		return;

	Window* topWindow { topPopupWindow() };

	if(topWindow == this)
		return;

	if(!topWindow->WindowShouldClose())
		return;

	std::unique_ptr<IPopup> closing { std::move(_popups.back()) }; //Off the stack before its result goes out, so a slot that opens a follow-up popup puts it on top
	_popups.pop_back();
	topPopupWindow()->_ignoreHeldPress = true; //The window below takes the mouse again, but not the press on the button that closed the popup

	if(!closing)
		return;

	closing->emitClosed();
	closing.reset();

	//Destroying the popup's window left no context current
	glfwMakeContextCurrent(_window);
}

RetroFuturaGUI::Window* RetroFuturaGUI::Window::topPopupWindow()
{
	if(_popups.empty())
		return this;

	IPopup* top { _popups.back().get() };

	if(!top)
		return this;

	Window* topWindow { top->GetWindow() };

	if(!topWindow)
		return this;

	return topWindow;
}

void RetroFuturaGUI::Window::refuseCloseWhileBlocked()
{
	if(!_window)
		return;

	if(!IsBlockedByPopup())
		return;

	glfwSetWindowShouldClose(_window, GLFW_FALSE);
}

void RetroFuturaGUI::Window::centerOver(const Window* window)
{
	if(!window)
		return;

	if(!_window)
		return;

	if(!window->_window)
		return;

	const DockRect
		over { window->screenRect() },
		own { screenRect() };
	const glm::ivec2 position { over._Position + (over._Size - own._Size) / 2 };
	glfwSetWindowPos(_window, position.x, position.y);
}

void RetroFuturaGUI::Window::setNativeOwner(const Window* owner)
{
	if(!owner)
		return;

	if(!_window)
		return;

	if(!owner->_window)
		return;

#if defined(TARGET_PLATFORM_LINUX)
	PlatformBridge::NativeWindow::SetOwner(glfwGetX11Display(), glfwGetX11Window(_window), glfwGetX11Window(owner->_window));
#elif defined(TARGET_PLATFORM_WINDOWS)
	PlatformBridge::NativeWindow::SetOwner(glfwGetWin32Window(_window), glfwGetWin32Window(owner->_window));
#endif
}