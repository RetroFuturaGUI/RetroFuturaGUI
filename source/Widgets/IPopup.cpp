#include "IPopup.hpp"
#include "Window.hpp"
#include "Scene.hpp"

RetroFuturaGUI::IPopup::IPopup(std::string_view name, const i32 width, const i32 height)
: _openerContext(glfwGetCurrentContext())
{
	_window = std::unique_ptr<Window>(new Window(name, width, height, Window::WindowRole::Popup));
}

RetroFuturaGUI::IPopup::~IPopup()
{
	if(!_window)
		return;

	GLFWwindow* glfwWindow { _window->GetGlfwWindow() };

	if(!glfwWindow)
		return;

	glfwMakeContextCurrent(glfwWindow);
}

RetroFuturaGUI::Window* RetroFuturaGUI::IPopup::GetWindow() const
{
	return _window.get();
}

void RetroFuturaGUI::IPopup::AddScene(std::unique_ptr<ISceneHost> host)
{
	if(!host)
		return;

	if(!_window)
		return;

	Scene* scene { host->GetScene() };

	if(!scene)
		return;

	_window->AddScene(scene);
	_scenes.push_back(std::move(host));
}

bool RetroFuturaGUI::IPopup::requestClose()
{
	if(!_window)
		return false;

	GLFWwindow* glfwWindow { _window->GetGlfwWindow() };

	if(!glfwWindow)
		return false;

	if(_window->IsBlockedByPopup())
		return false;

	_isCloseRequested = true;
	glfwSetWindowShouldClose(glfwWindow, GLFW_TRUE);
	return true;
}

bool RetroFuturaGUI::IPopup::allowsSystemClose()
{
	return true;
}

void RetroFuturaGUI::IPopup::restoreOpenerContext() const
{
	//restore context to the state when this window was opening
	glfwMakeContextCurrent(_openerContext);
}