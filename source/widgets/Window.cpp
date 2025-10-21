//#include "Label.hpp"
#include <print>
#include "Window.hpp"

RetroFuturaGUI::Window::Window(const std::string& name, i32 width, i32 height, void* parent, Sizing minWidth, Sizing minHeight, Sizing maxWidth, Sizing maxHeight)
    : IWidget(name, width, height, parent, minWidth, minHeight, maxWidth, maxHeight)
{
	createWindow();
}

void RetroFuturaGUI::Window::createWindow()
{
	glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
	_window = glfwCreateWindow(_width, _height, "glfw test", nullptr, nullptr);

	//std::println("Creating window: {0} ({1}x{2})", _name, _width, _height);

	if (!_window)
	{
		std::println("{0}", "Failed to create GLFW window");
		glfwTerminate();
		return;
	}

	glfwMakeContextCurrent(_window);
	glfwSetMouseButtonCallback(_window, mouseButtonCallback);
}

void RetroFuturaGUI::Window::mouseButtonCallback(GLFWwindow* window, i32 button, i32 action, i32 mods)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
	{
		// Get the mouse position
		f64 mouseX, mouseY;
		glfwGetCursorPos(window, &mouseX, &mouseY);
        i32 width, height;
        glfwGetWindowSize(window, &width, &height);

		// Convert mouse coordinates to OpenGL normalized device coordinates
		f32 normalizedX = (mouseX / width) * 2.0f - 1.0f;
		f32 normalizedY = 1.0f - (mouseY / 
            height) * 2.0f;

		// Check if the click is within the close button bounds
		if (normalizedX >= 0.9f && normalizedX <= 1.0f &&
			normalizedY >= 0.9f && normalizedY <= 1.0f)
		{
			// Close the window
			glfwSetWindowShouldClose(window, GLFW_TRUE);
		}
	}
}

void RetroFuturaGUI::Window::renderCustomCloseButton()
{
	// Render a simple rectangle as the custom top bar
	glBegin(GL_QUADS);
	glColor3f(1.0f, 0.0f, 0.0f); // Blue color
	glVertex2f(0.9f, 1.0f);     // Top-left
	glVertex2f(1.0f, 1.0f);      // Top-right
	glVertex2f(1.0f, 0.9f);      // Bottom-right
	glVertex2f(0.9f, 0.9f);     // Bottom-left
	glEnd();
}

void RetroFuturaGUI::Window::renderCustomTopBar()
{
	// Render a simple rectangle as the custom top bar
	glBegin(GL_QUADS);
	glColor3f(0.2f, 0.2f, 0.8f); // Blue color
	glVertex2f(-1.0f, 1.0f);     // Top-left
	glVertex2f(1.0f, 1.0f);      // Top-right
	glVertex2f(1.0f, 0.9f);      // Bottom-right
	glVertex2f(-1.0f, 0.9f);     // Bottom-left

	renderCustomCloseButton();

	glEnd();
}

bool RetroFuturaGUI::Window::WindowShouldClose()
{
    return glfwWindowShouldClose(_window);
}

void RetroFuturaGUI::Window::Draw()
{
    // Clear the screen
		glClear(GL_COLOR_BUFFER_BIT);

		// Render the custom top bar
		renderCustomTopBar();

		// Swap buffers and poll events
		glfwSwapBuffers(_window);
		glfwPollEvents();

        if(_child != nullptr)
        {
            //GetChild<RetroFuturaGUI::Label>()->Draw();
        }
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