//#include "Label.hpp"
#include <print>
#include <glad/glad.h>
#include "MainWindow.hpp"
#include "PlatformBridge.hpp"

RetroFuturaGUI::MainWindow::MainWindow(const std::string& name, i32 width, i32 height, void* parent, Sizing minWidth, Sizing minHeight, Sizing maxWidth, Sizing maxHeight)
{
	if(_mainWindowInitialized)
	{
		std::println("Main window already initialized!");
		return;
	}

	if (!glfwInit())
	{
		std::println("glfw couldn't start");
		return;
	}

	glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);

	_mainWindow = std::make_unique<Window>(name, width, height, parent, minWidth, minHeight, maxWidth, maxHeight);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::println("Failed to initialize GLAD");
        return;
    }

	PlatformBridge::Monitors::GetMonitors();

	_mainWindowInitialized = true;
}

RetroFuturaGUI::MainWindow::~MainWindow()
{
    glfwTerminate();
}

void RetroFuturaGUI::MainWindow::Draw()
{
	if(_mainWindow)
		_mainWindow->Draw();
}

bool RetroFuturaGUI::MainWindow::WindowShouldClose()
{
	if(_mainWindow)
		return _mainWindow->WindowShouldClose();

	return true;
}

void RetroFuturaGUI::MainWindow::SetBackgroundColor(const glm::vec4 &color)
{
	if (_mainWindow)
		_mainWindow->SetBackgroundColor(color);
}
