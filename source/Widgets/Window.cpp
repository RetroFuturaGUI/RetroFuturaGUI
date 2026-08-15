#include "Window.hpp"
#include "PlatformBridge.hpp"
#include <glm/gtc/type_ptr.hpp>
#include <print>

#if defined(TARGET_PLATFORM_LINUX)
    #define GLFW_EXPOSE_NATIVE_X11
#elif defined(TARGET_PLATFORM_WINDOWS)
    #define GLFW_EXPOSE_NATIVE_WIN32
#endif
#include <GLFW/glfw3native.h>

RetroFuturaGUI::Window::Window(std::string_view name, const i32 width, const i32 height)
: _width(width), _height(height), _name(name)
{
	createWindow();

	if(_cursorsInitialized)
		return;
		
	_resizeCursorHorizontal = glfwCreateStandardCursor(GLFW_HRESIZE_CURSOR);
	_resizeCursorVertical = glfwCreateStandardCursor(GLFW_VRESIZE_CURSOR);
	_resizeCursorTLBR = glfwCreateStandardCursor(GLFW_RESIZE_NWSE_CURSOR);
	_resizeCursorTRBL = glfwCreateStandardCursor(GLFW_RESIZE_NESW_CURSOR);
	_defaultCursor = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
	_cursorsInitialized = true;
}

void RetroFuturaGUI::Window::createWindow()
{
	glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
	glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_FALSE); //Using GLFW_TRUE enables the possibility of rendering transparent backgrounds (desired) but causes any other object with transparency to be bugged (makes everything behind the window is visible)
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

	_window = glfwCreateWindow(_width, _height, _windowTitle.c_str(), nullptr, nullptr);
	glfwSetWindowTitle(_window, _windowTitle.c_str());

	if (!_window)
	{
		std::println("{0}", "Failed to create GLFW window");
		glfwTerminate();
		return;
	}
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
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
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

	SetBackgroundColor(glm::vec4(0.086f, 0.086f, 0.1137f, 1.0f), ColorState::Enabled);
	//setupWindowBar();
}

void RetroFuturaGUI::Window::SetWindowTitle(std::string_view title, std::string_view fontFamily)
{
	_windowTitle = title;
	glfwSetWindowTitle(_window, _windowTitle.c_str());

	if(_windowBar)
		_windowBar->SetWindowTitle(_windowTitle, fontFamily);
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

	if (isMouseButtonDown && !_wasMouseButtonDown) //press
	{
		setResizeState(GLFW_MOUSE_BUTTON_LEFT, GLFW_PRESS, 0);

		if (_windowBar && _windowBar->IsPointInside(static_cast<f32>(_cursorPosX), static_cast<f32>(_cursorPosY)))
		{
			_isDragging = true;
			setAbsoluteCursorPosition(this);
			_dragStartPos = glm::vec2(_absoluteCursorPosX, _absoluteCursorPosY);
			_windowDragStartPos = glm::vec2(_windowPosX, _windowPosY);
		}
	}
	else if (!isMouseButtonDown && _wasMouseButtonDown) //release
	{
		_isDragging = false;
		setResizeState(GLFW_MOUSE_BUTTON_LEFT, GLFW_RELEASE, 0);
	}

	_wasMouseButtonDown = isMouseButtonDown;

	resize();
	drag();
}

void RetroFuturaGUI::Window::setCursorPosition()
{
	i32 x { 0 }, y { 0 };
	getCursorPosition(x, y);
	_cursorPosX = x;
	_cursorPosY = y;
}

void RetroFuturaGUI::Window::setCursorIcon() // clean up this abomination
{
	if ((_cursorPosY < _boundaryThreshold && _cursorPosX < _boundaryThreshold)
		|| (_cursorPosY > _height - _boundaryThreshold && _cursorPosX > _width - _boundaryThreshold))
			glfwSetCursor(_window, _resizeCursorTLBR);
	else if ((_cursorPosY < _boundaryThreshold && _cursorPosX > _width - _boundaryThreshold)
		|| (_cursorPosY > _height - _boundaryThreshold && _cursorPosX < _boundaryThreshold))
			glfwSetCursor(_window, _resizeCursorTRBL);
	else if (_cursorPosX < _boundaryThreshold || _cursorPosX > _width - _boundaryThreshold)
		glfwSetCursor(_window, _resizeCursorHorizontal);
	else if (_cursorPosY < _boundaryThreshold || _cursorPosY > _height - _boundaryThreshold)
		glfwSetCursor(_window, _resizeCursorVertical);
	else
		glfwSetCursor(_window, _defaultCursor);
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

		if(_cursorPosY < _boundaryThreshold && _cursorPosX < _boundaryThreshold) 
			_resizeEdge = ResizeEdge::TopLeft;
		else if (_cursorPosY < _boundaryThreshold && _cursorPosX > _width - _boundaryThreshold) 
			_resizeEdge = ResizeEdge::TopRight;
		else if (_cursorPosY > _height - _boundaryThreshold && _cursorPosX < _boundaryThreshold) 
			_resizeEdge = ResizeEdge::BottomLeft;
		else if (_cursorPosY > _height - _boundaryThreshold && _cursorPosX > _width - _boundaryThreshold) 
			_resizeEdge = ResizeEdge::BottomRight;
		else if (_cursorPosX < _boundaryThreshold)
			_resizeEdge = ResizeEdge::Left;
		else if (_cursorPosX > _width - _boundaryThreshold) 
			_resizeEdge = ResizeEdge::Right;
		else if (_cursorPosY < _boundaryThreshold) 
			_resizeEdge = ResizeEdge::Top;
		else if (_cursorPosY > _height - _boundaryThreshold) 
			_resizeEdge = ResizeEdge::Bottom;
		else 
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

void RetroFuturaGUI::Window::drag()
{
	if (!_isDragging) 
		return;

	setAbsoluteCursorPosition(this);
	glm::vec2 currentPos = glm::vec2(_absoluteCursorPosX, _absoluteCursorPosY);
	glm::vec2 delta = currentPos - _dragStartPos;
	i32 newPosX = (i32)_windowDragStartPos.x + (i32)delta.x;
	i32 newPosY = (i32)_windowDragStartPos.y + (i32)delta.y;
	moveWindow(newPosX, newPosY);
}

void RetroFuturaGUI::Window::moveWindow(const i32 posX, const i32 posY)
{
	glfwSetWindowPos(_window, posX, posY);
	_windowPosX = posX;
	_windowPosY = posY;
}

void RetroFuturaGUI::Window::setAbsoluteCursorPosition(Window *self)
{
	if(!self)
		return;

	i32 x { 0 }, y { 0 };
	self->getCursorPosition(x, y);
	glfwGetWindowPos(self->_window, &self->_windowPosX, &self->_windowPosY);
	self->_absoluteCursorPosX = x + self->_windowPosX;
	self->_absoluteCursorPosY = y + self->_windowPosY;
}

void RetroFuturaGUI::Window::toggleMaximize()
{
	if (_maximizeState == MaximizeState::RESTORE)
	{

		_preMaximizeSize = glm::ivec2(_width, _height);
		_preMaximizePos = glm::ivec2(_windowPosX, _windowPosY);
		glfwMaximizeWindow(_window);
		_maximizeState = MaximizeState::MAXIMIZE;
	} 
	else
	{
		glfwRestoreWindow(_window);
		SetWindowSize(_preMaximizeSize.x, _preMaximizeSize.y);
		moveWindow(_preMaximizePos.x, _preMaximizePos.y);
		_maximizeState = MaximizeState::RESTORE;
	}
	
	_windowSizeChanged = true;
}

void RetroFuturaGUI::Window::updateProjection()
{ 
	glfwGetWindowSize(_window, &_width, &_height);

	if (_lastSize == glm::ivec2(_width, _height))
		return;
		
	_projection->UpdateProjectionMatrix((f32)_width, (f32)_height);
	glViewport(0, 0, _width, _height);

	if (_windowBar)
		_windowBar->Resize();

	if (_lasagna) 
		_lasagna->SetSize(glm::vec3((f32)_width, (f32)_height, _lasagna->GetSize().z));

	if(_background)
	{
		_background->SetSize(glm::vec2((f32)_width, (f32)_height));
		_background->SetPosition(glm::vec3((f32)_width * 0.5f, (f32)_height * 0.5f, -1.0f));
	}

	_lastSize = { _width, _height };
	_windowSizeChanged = false;
}

bool RetroFuturaGUI::Window::WindowShouldClose()
{
    return glfwWindowShouldClose(_window);
}

void RetroFuturaGUI::Window::Draw()
{
	pollInput();

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_FALSE);
	glfwWindowHint(GLFW_ALPHA_BITS, 0); // Disable alpha bits if not needed

	if(_windowSizeChanged)
		updateProjection();

	drawBackground();

	if(_lasagna)
		_lasagna->Draw(false);

	if(_windowBar)
		_windowBar->Draw();

	glfwSwapBuffers(_window);
	glfwPollEvents();

	if(_windowBar->WindowShouldClose())
		glfwSetWindowShouldClose(_window, GLFW_TRUE);
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
/*
	if(!_windowBar)
		return;

	_windowBar->SetElementBackgroundImageTextureID(_backgroundImage->GetID(), WindowBar::ElementType::Title);
	_windowBar->SetElementBackgroundImageTextureID(_backgroundImage->GetID(), WindowBar::ElementType::CloseButton);
	_windowBar->SetElementBackgroundImageTextureID(_backgroundImage->GetID(), WindowBar::ElementType::MaximizeButton);
	_windowBar->SetElementBackgroundImageTextureID(_backgroundImage->GetID(), WindowBar::ElementType::MinimizeButton);*/
}

void RetroFuturaGUI::Window::SetLasagna(Lasagna* lasagna)
{
	_lasagna = lasagna;
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

RetroFuturaGUI::WindowBar& RetroFuturaGUI::Window::GetWindowBar()
{
	return *_windowBar;
}

void RetroFuturaGUI::Window::setupWindowBar()
{
	_windowBar = std::make_unique<WindowBar>(_name + "/WindowBar", const_cast<Projection*>(&*_projection.get()), nullptr, WidgetTypeID::Window, _window);

	if(!_windowBar)
		return;
		
	_windowBar->ConnectMaximizeCallback([this]() { toggleMaximize(); });
}

void RetroFuturaGUI::Window::destroyWindowBar()
{
	if(!_windowBar)
		return;

	_windowBar.reset();
}

void RetroFuturaGUI::Window::ShowWindowBar(const bool show)
{
	if(show)
		setupWindowBar();
	else
	    destroyWindowBar();
}


const std::string& RetroFuturaGUI::Window::GetName() const
{
	return _name;
}