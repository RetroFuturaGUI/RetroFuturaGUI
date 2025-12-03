#include "Window.hpp"
#include <glm/gtc/type_ptr.hpp>
#include <print>

RetroFuturaGUI::Window::Window(const std::string& name, i32 width, i32 height, void* parent)
   // : IWidget(name, glm::mat4(1.0f), width, height, parent)
{
	createWindow();
}

void RetroFuturaGUI::Window::createWindow()
{
	glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
	glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);
	_window = glfwCreateWindow(_width, _height, "glfw test", nullptr, nullptr);

	if (!_window)
	{
		std::println("{0}", "Failed to create GLFW window");
		glfwTerminate();
		return;
	}

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


	static bool shadersInitialized = false;
	if (!shadersInitialized)		
	{
		ShaderManager::Init();
		shadersInitialized = true;
	}


	_projection = std::make_unique<Projection>((float)_width, (float)_height);

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
}

bool RetroFuturaGUI::Window::WindowShouldClose()
{
    return glfwWindowShouldClose(_window);
}

void RetroFuturaGUI::Window::Draw()
{
	glClear(GL_COLOR_BUFFER_BIT);
	_label->Draw();
	_button->Draw();
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