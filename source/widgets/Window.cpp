#include "Window.hpp"
#include <glm/gtc/type_ptr.hpp>
#include <print>

RetroFuturaGUI::Window::Window(std::string_view name, std::string_view windowTitle, i32 width, i32 height, void* parent)
: _name(name), _windowTitle(windowTitle)
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
	glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_FALSE);
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

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glViewport(0, 0, _width, _height);

	SetBackgroundColor(_backgroundColor);

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
	std::string windowBarName = _name + "/WindowBar";
	IdentityParams identityWB = { windowBarName, this, WidgetTypeID::WindowBar, _window };
	GeometryParams2D geometryWB = { *_projection, glm::vec2(0.0f), glm::vec2(0.0f), 0.0f };

	_windowBar = std::make_unique<WindowBar>(identityWB, geometryWB, glm::vec4(0.5f, 0.0f, 1.0f, 1.0f));
	_windowBar->ConnectMaximizeCallback([this]() { toggleMaximize(); });
	_windowBar->SetElementBackgroundColor(glm::vec4(0.5f, 0.0f, 1.0f, 0.65f), ColorSetState::Enabled, WindowBar::ElementType::Title);
	_windowBar->SetElementBackgroundColor(glm::vec4(1.0f, 0.1f, 0.1f, 0.65f), ColorSetState::Enabled, WindowBar::ElementType::CloseButton);
	_windowBar->SetElementBackgroundColor(glm::vec4(1.0f, 0.2f, 0.2f, 0.65f), ColorSetState::Hover, WindowBar::ElementType::CloseButton);
	_windowBar->SetElementBackgroundColor(glm::vec4(1.0f, 0.3f, 0.3f, 0.75f), ColorSetState::Clicked, WindowBar::ElementType::CloseButton);
	_windowBar->SetElementBackgroundColor(glm::vec4(0.5f, 0.5f, 0.5f, 0.75f), ColorSetState::Enabled, WindowBar::ElementType::MaximizeButton);
	_windowBar->SetElementBackgroundColor(glm::vec4(0.7f, 0.7f, 0.7f, 0.75f), ColorSetState::Hover, WindowBar::ElementType::MaximizeButton);
	_windowBar->SetElementBackgroundColor(glm::vec4(0.8f, 0.8f, 0.8f, 0.85f), ColorSetState::Clicked, WindowBar::ElementType::MaximizeButton);
	_windowBar->SetElementBackgroundColor(glm::vec4(0.5f, 0.5f, 0.5f, 0.75f), ColorSetState::Enabled, WindowBar::ElementType::MinimizeButton);
	_windowBar->SetElementBackgroundColor(glm::vec4(0.7f, 0.7f, 0.7f, 0.75f), ColorSetState::Hover, WindowBar::ElementType::MinimizeButton);
	_windowBar->SetElementBackgroundColor(glm::vec4(0.8f, 0.8f, 0.8f, 0.85f), ColorSetState::Clicked, WindowBar::ElementType::MinimizeButton);
	_windowBar->SetButtonCornerRadii(glm::vec4(10.0f), WindowBar::ElementType::CloseButton);
	_windowBar->SetButtonCornerRadii(glm::vec4(10.0f), WindowBar::ElementType::MaximizeButton);
	_windowBar->SetButtonCornerRadii(glm::vec4(10.0f), WindowBar::ElementType::MinimizeButton);
	_windowBar->SetWindowTitle(_windowTitle);
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

	if (self->_windowBar && self->_windowBar->IsPointInside(self->_cursorPosX, self->_cursorPosY))
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

void RetroFuturaGUI::Window::setResizeState(i32 button, i32 action, i32 mods)
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
			_resizeEdge = ResizeEdge::TOP_LEFT;
		else if (_cursorPosY < _boundaryThreshold && _cursorPosX > _width - _boundaryThreshold) 
			_resizeEdge = ResizeEdge::TOP_RIGHT;
		else if (_cursorPosY > _height - _boundaryThreshold && _cursorPosX < _boundaryThreshold) 
			_resizeEdge = ResizeEdge::BOTTOM_LEFT;
		else if (_cursorPosY > _height - _boundaryThreshold && _cursorPosX > _width - _boundaryThreshold) 
			_resizeEdge = ResizeEdge::BOTTOM_RIGHT;
		else if (_cursorPosX < _boundaryThreshold)
			_resizeEdge = ResizeEdge::LEFT;
		else if (_cursorPosX > _width - _boundaryThreshold) 
			_resizeEdge = ResizeEdge::RIGHT;
		else if (_cursorPosY < _boundaryThreshold) 
			_resizeEdge = ResizeEdge::TOP;
		else if (_cursorPosY > _height - _boundaryThreshold) 
			_resizeEdge = ResizeEdge::BOTTOM;
		else 
			_resizeEdge = ResizeEdge::NONE;
		
		if (_resizeEdge != ResizeEdge::NONE)
			_isResizing = true;
	} 
	else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
	{
		_isResizing = false;
		_resizeEdge = ResizeEdge::NONE;
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
        case ResizeEdge::TOP_LEFT:
            newWidth = _width - (i32)deltaX;
            newHeight = _height - (i32)deltaY;
            newPosX = _windowPosX + (i32)deltaX;
            newPosY = _windowPosY + (i32)deltaY;
        break;
        case ResizeEdge::TOP_RIGHT:
            newWidth = _width + (i32)deltaX;
            newHeight = _height - (i32)deltaY;
            newPosY = _windowPosY + (i32)deltaY;
        break;
        case ResizeEdge::BOTTOM_LEFT:
            newWidth = _width - (i32)deltaX;
            newHeight = _height + (i32)deltaY;
            newPosX = _windowPosX + (i32)deltaX;
        break;
        case ResizeEdge::BOTTOM_RIGHT:
            newWidth = _width + (i32)deltaX;
            newHeight = _height + (i32)deltaY;
        break;
        case ResizeEdge::LEFT:
            newWidth = _width - (i32)deltaX;
            newPosX = _windowPosX + (i32)deltaX;
        break;
        case ResizeEdge::RIGHT:
            newWidth = _width + (i32)deltaX;
        break;
        case ResizeEdge::TOP:
            newHeight = _height - (i32)deltaY;
            newPosY = _windowPosY + (i32)deltaY;
        break;
        case ResizeEdge::BOTTOM:
            newHeight = _height + (i32)deltaY;
        break;
    }

    _prevResizeX = _absoluteCursorPosX;
    _prevResizeY = _absoluteCursorPosY;
    _width = std::max<i32>(newWidth, _minWindowSpan);
    _height = std::max<i32>(newHeight, _minWindowSpan);
    _windowPosX = newPosX;
    _windowPosY = newPosY;
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
	i32 newPosX = _windowDragStartPos.x + (i32)delta.x;
	i32 newPosY = _windowDragStartPos.y + (i32)delta.y;

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

	if (_grid) 
		_grid->SetSize(glm::vec2((f32)_width, (f32)_height));

	if( _backgroundImage)
	{
		_backgroundImage->SetSize(glm::vec2((f32)_width, (f32)_height));
		_backgroundImage->Move(glm::vec2((f32)_width * 0.5f, (f32)_height * 0.5f));
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
	glClearColor(_backgroundColor.r, _backgroundColor.g, _backgroundColor.b, _backgroundColor.a); // Opaque white background
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_FALSE);
	glfwWindowHint(GLFW_ALPHA_BITS, 0); // Disable alpha bits if not needed

	if(_windowSizeChanged)
		updateProjection();

	_backgroundImage->Draw();
	_grid->Draw(false);
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
	GeometryParams2D geometryTexture
	{
		*_projection,
		glm::vec2((f32)_width * 0.5f, (f32)_height * 0.5f),
		glm::vec2(_width, _height),
		0.0f
	};

	_backgroundImage = std::make_unique<Image2D>(geometryTexture, imagePath);
	_windowBar->SetElementBackgroundImageTextureID(_backgroundImage->GetTextureID(), WindowBar::ElementType::Title);
	_windowBar->SetElementBackgroundImageTextureID(_backgroundImage->GetTextureID(), WindowBar::ElementType::CloseButton);
	_windowBar->SetElementBackgroundImageTextureID(_backgroundImage->GetTextureID(), WindowBar::ElementType::MaximizeButton);
	_windowBar->SetElementBackgroundImageTextureID(_backgroundImage->GetTextureID(), WindowBar::ElementType::MinimizeButton);
}

void RetroFuturaGUI::Window::SetGrid(Grid2d* grid)
{
	_grid = grid;
}

i32 RetroFuturaGUI::Window::GetBackgroundImageId() const
{
    return _backgroundImage->GetTextureID();
}

GLFWwindow* RetroFuturaGUI::Window::GetGlfwWindow() const
{
    return _window;
}

RetroFuturaGUI::Projection* RetroFuturaGUI::Window::GetProjection() const
{
    return &*_projection;
}
