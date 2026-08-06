#include "Window.hpp"
#include <glm/gtc/type_ptr.hpp>
#include <print>

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
	SetBackgroundColor(_backgroundColor);
	glViewport(0, 0, _width, _height);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::println("Failed to initialize GLAD");
        return;
    }

	glfwSetCursorPosCallback(_window, cursorPositionCallback);
	glfwSetMouseButtonCallback(_window, mouseButtonClickedCallback);
	glfwSetWindowFocusCallback(_window, windowFocusCallback);
	glfwSetWindowUserPointer(_window, this);

	static bool shadersInitialized = false;
	if (!shadersInitialized)		
	{
		ShaderManager::Init();
		shadersInitialized = true;
	}

	_projection = std::make_unique<Projection>((float)_width, (float)_height);
	//setupWindowBar();
}

void RetroFuturaGUI::Window::SetWindowTitle(std::string_view title, std::string_view fontFamily)
{
	_windowTitle = title;
	glfwSetWindowTitle(_window, _windowTitle.c_str());

	if(_windowBar)
		_windowBar->SetWindowTitle(_windowTitle, fontFamily);
}

void RetroFuturaGUI::Window::cursorPositionCallback(GLFWwindow *window, f64 xpos, f64 ypos)
{
	InputManager::SetHoveredWindow(window);
	InputManager::SetMousePositionInvertedY(xpos, ypos);
	InputManager::SetMousePosition(xpos, ypos);

	Window* self = static_cast<Window*>(glfwGetWindowUserPointer(window));

	if (self) 
	{
		self->setCursorPosition();
		self->setCursorIcon();
		self->resize();
		self->drag();
	}
}

void RetroFuturaGUI::Window::setCursorPosition()
{
	glfwGetCursorPos(_window, &_cursorPosX, &_cursorPosY);
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

void RetroFuturaGUI::Window::mouseButtonClickedCallback(GLFWwindow *window, i32 button, i32 action, i32 mods)
{
	Window* self = static_cast<Window*>(glfwGetWindowUserPointer(window));

	if (action == GLFW_PRESS) 
	{
		InputManager::SetFocusedWindow(window);
	}
	else if (action == GLFW_RELEASE) 
	{
		//std::println("mouse released: {}", button);
		self->_isDragging = false;
	}
	
	InputManager::SetMouseButtonState(button, action == GLFW_PRESS);

	if (self)
		self->setResizeState(button, action, mods);

	if (self->_windowBar && self->_windowBar->IsPointInside(static_cast<f32>(self->_cursorPosX), static_cast<f32>(self->_cursorPosY)))
	{
		if (action == GLFW_PRESS)
		{
			self->_isDragging = true;
			setAbsoluteCursorPosition(self);
			self->_dragStartPos = glm::vec2(self->_absoluteCursorPosX, self->_absoluteCursorPosY);
			self->_windowDragStartPos = glm::vec2(self->_windowPosX, self->_windowPosY);
		}
	}
}

void RetroFuturaGUI::Window::setResizeState(i32 button, i32 action, [[maybe_unused]] i32 mods)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
	{
        glfwGetCursorPos(_window, &_absoluteCursorPosX, &_absoluteCursorPosY);
        glfwGetWindowPos(_window, &_windowPosX, &_windowPosY);
		glfwGetCursorPos(_window, &_cursorPosX, &_cursorPosY);
        _absoluteCursorPosX += _windowPosX;
        _absoluteCursorPosY += _windowPosY;
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

    glfwGetCursorPos(_window, &_absoluteCursorPosX, &_absoluteCursorPosY);
    glfwGetWindowPos(_window, &_windowPosX, &_windowPosY);
    _absoluteCursorPosX += _windowPosX;
    _absoluteCursorPosY += _windowPosY;

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

void RetroFuturaGUI::Window::windowFocusCallback(GLFWwindow *window, i32 focused)
{
	if (focused)
	{
		InputManager::SetFocusedWindow(window);
		//std::println("focused");
	} 
	else if (InputManager::GetFocusedWindow() == window)
	{
		InputManager::SetFocusedWindow(nullptr);
		//std::println("unfocused");
	}
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

	glfwGetCursorPos(self->_window, &self->_absoluteCursorPosX, &self->_absoluteCursorPosY);
	glfwGetWindowPos(self->_window, &self->_windowPosX, &self->_windowPosY);
	self->_absoluteCursorPosX += self->_windowPosX;
	self->_absoluteCursorPosY += self->_windowPosY;
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

	if(_backgroundImage)
	{
		_backgroundImage->SetSize(glm::vec3((f32)_width, (f32)_height, _backgroundImage->GetSize().z));
		_backgroundImage->SetPosition(glm::vec3((f32)_width * 0.5f, (f32)_height * 0.5f, -_projection->GetDepth()));
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
	glClearColor(_backgroundColor.r, _backgroundColor.g, _backgroundColor.b, _backgroundColor.a);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_FALSE);
	glfwWindowHint(GLFW_ALPHA_BITS, 0); // Disable alpha bits if not needed

	if(_windowSizeChanged)
		updateProjection();

	if(_backgroundImage)
		_backgroundImage->Draw();

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

void RetroFuturaGUI::Window::SetBackgroundColor(const glm::vec4 &color)
{
	_backgroundColor = color;
	glClearColor(_backgroundColor.r, _backgroundColor.g, _backgroundColor.b, _backgroundColor.a);
}

void RetroFuturaGUI::Window::SetBackgroundImage(std::string_view imagePath)
{
	_backgroundImage = std::make_unique<Image>(_name + "/BackgroundImage", &*_projection, nullptr, WidgetTypeID::Window, _window, imagePath);
	_backgroundImage->SetSize(glm::vec3(_width, _height, 0.01f));
	_backgroundImage->SetPosition(glm::vec3((f32)_width * 0.5f, (f32)_height * 0.5f, -_projection->GetDepth()));

	if(!_windowBar)
		return;

	_windowBar->SetElementBackgroundImageTextureID(_backgroundImage->GetTextureID(), WindowBar::ElementType::Title);
	_windowBar->SetElementBackgroundImageTextureID(_backgroundImage->GetTextureID(), WindowBar::ElementType::CloseButton);
	_windowBar->SetElementBackgroundImageTextureID(_backgroundImage->GetTextureID(), WindowBar::ElementType::MaximizeButton);
	_windowBar->SetElementBackgroundImageTextureID(_backgroundImage->GetTextureID(), WindowBar::ElementType::MinimizeButton);
}

void RetroFuturaGUI::Window::SetLasagna(Lasagna* lasagna)
{
	_lasagna = lasagna;
}

i32 RetroFuturaGUI::Window::GetBackgroundImageId() const
{
	if(_backgroundImage)
    	return _backgroundImage->GetTextureID();

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