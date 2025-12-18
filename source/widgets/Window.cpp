#include "Window.hpp"
#include <glm/gtc/type_ptr.hpp>
#include <print>

RetroFuturaGUI::Window::Window(const std::string& name, i32 width, i32 height, void* parent)
   // : IWidget(name, glm::mat4(1.0f), width, height, parent)
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
	glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
	_window = glfwCreateWindow(_width, _height, "glfw test", nullptr, nullptr);

	if (!_window)
	{
		std::println("{0}", "Failed to create GLFW window");
		glfwTerminate();
		return;
	}
	_prevResizeX = (f64)_width;
	_prevResizeY = (f64)_height;

	glfwMakeContextCurrent(_window);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) //maybe move this to MainWindow?
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
	




	IdentityParams identityG = { "testGrid", this, WidgetTypeID::Grid2d };
	GeometryParams2D geometryG = { *_projection, glm::vec2(0.0f, 0.0f), glm::vec2((float)_width, (float)_height), 0.0f };
	Grid2dAxisDefinition axisDefinition = 
	{
		{ 0.3f, 0.5f, 0.2f },
		{ 0.6f, 0.4f }
	};

	_grid = std::make_unique<Grid2d>(identityG, geometryG, axisDefinition);








	GeometryParams2D geometryTexture
	{
		*_projection,
		glm::vec2((f32)_width * 0.5f, (f32)_height * 0.5f),
		glm::vec2(_width, _height),
		0.0f
	};

	_backgroundImage = std::make_unique<Image2D>(geometryTexture);


	IdentityParams identity = { "testLabel", this, WidgetTypeID::Window };
	GeometryParams2D geometry = { *_projection, glm::vec2(800.0f, 600.0f), glm::vec2(300.0f, 90.0f), 0.0f };
	std::string tempPath = PlatformBridge::Fonts::GetFontsInformation().front().second;
	TextParams textParams = { "Test Label", tempPath, glm::vec4(1.0f), glm::vec2(30.0f), TextAlignment::CENTER, 5.0f };

	IdentityParams identityB = { "testButton", this, WidgetTypeID::Window };
	GeometryParams2D geometryB = { *_projection, glm::vec2(300.0f, 300.0f), glm::vec2(300.0f, 90.0f), 0.0f };
	TextParams textParamsB = { "Test Button", tempPath, glm::vec4(1.0f), glm::vec2(30.0f), TextAlignment::CENTER, 5.0f };
	BorderParams borderParams = { glm::vec4(0.3f, 0.3f, 0.3f, 1.0f), 5.0f };

	IdentityParams identityWB = { "testWindowbar", this, WidgetTypeID::WindowBar };
	GeometryParams2D geometryWB = { *_projection, glm::vec2(0.0f), glm::vec2(0.0f), 0.0f };

	_button = std::make_unique<Button>(identityB, geometryB, textParamsB, borderParams);
	_label = std::make_unique<Label>(identity, geometry, textParams);
	_windowBar = std::make_unique<WindowBar>(identityWB, geometryWB, glm::vec4(0.5f, 0.0f, 1.0f, 1.0f));

	auto buttonPressed = [](){ std::println("buttonPressed Slot"); };
	auto buttonReleased = [](){ std::println("buttonReleased Slot"); };
	auto whileHover = [](){ std::println("whileHover Slot"); };
	auto mouseEnter = [](){ std::println("mouseEnter Slot"); };
	auto mouseLeave = [](){ std::println("mouseLeave Slot"); };
	_button->Connect_OnClick(buttonPressed, true);
	_button->Connect_OnRelease(buttonReleased, true);
	//_button->Connect_WhileHover(whileHover, true);
	_button->Connect_OnMouseEnter(mouseEnter, true);
	_button->Connect_OnMouseLeave(mouseLeave, true);

	_grid->AttachWidget(0, 0, std::unique_ptr<IWidget>(std::move(_button)));

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
	if (action == GLFW_PRESS) 
	{
		InputManager::SetFocusedWindow(window);
		// std::println("mouse click: {}", button);
	}
	else if (action == GLFW_RELEASE) 
	{
		//std::println("mouse released: {}", button);
	}
	
	InputManager::SetMouseButtonState(button, action == GLFW_PRESS);

	Window* self = static_cast<Window*>(glfwGetWindowUserPointer(window));

	if (self) 
		self->setResizeState(button, action, mods);
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
	glViewport(0, 0, _width, _height); // causes lags
	moveWindow(_windowPosX, _windowPosY);
    _projection->UpdateProjectionMatrix((f32)_width, (f32)_height);
	_grid->Resize(glm::vec2((f32)_width, (f32)_height));
}

void RetroFuturaGUI::Window::windowFocusCallback(GLFWwindow *window, i32 focused)
{
	if (focused)
	{
		InputManager::SetFocusedWindow(window);
		std::println("focused");
	} 
	else if (InputManager::GetFocusedWindow() == window)
	{
		InputManager::SetFocusedWindow(nullptr);
		std::println("unfocused");
	}
}

void RetroFuturaGUI::Window::moveWindow(const i32 posX, const i32 posY)
{
	glfwSetWindowPos(_window, posX, posY);
	_windowPosX = posX;
	_windowPosY = posY;
}

bool RetroFuturaGUI::Window::WindowShouldClose()
{
    return glfwWindowShouldClose(_window);
}

void RetroFuturaGUI::Window::Draw()
{
	glClear(GL_COLOR_BUFFER_BIT);
	//_backgroundImage->Draw();
	//_label->Draw();
	//_button->Draw();
	//_windowBar->Draw();
	_grid->Draw(true);
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