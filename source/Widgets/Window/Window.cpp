#include "Window.hpp"
#include "Scene.hpp"
#include "WindowTab.hpp"
#include "ContextObjects.hpp"
#include "ShaderManager.hpp"
#include <print>

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
        getAllWindows().push_back(this);
}

RetroFuturaGUI::Window::~Window()
{
    //Popups go from the top down, the reverse of how they opened, which a vector's own destruction wouldn't do. Their results
    //don't go out: the slots may be gone by now.
    while(!_popups.empty())
        _popups.pop_back();

    if(_dockSnap._Tab && isRegisteredWindowTab(_dockSnap._Tab))
        _dockSnap._Tab->hideDropPreview();

    std::erase(getAllWindows(), this);
    Undock();

    if(!_window)
        return;

    //The background's and the border's GL objects belong to this window's context, so they go while it still exists and is current.
    //A background picture is ResourceManager's and stays
    glfwMakeContextCurrent(_window);
    ContextObject::DeletePending(_window);
    _background.reset();
    _backgroundSvg.reset();
    _border.reset();

    //A later window must not try to share with a destroyed one
    if(_shareWindow == _window)
        _shareWindow = nullptr;

    glfwDestroyWindow(_window);
    ContextObject::ForgetContext(_window); //its VAOs and framebuffers went with it
}

bool RetroFuturaGUI::Window::WindowShouldClose()
{
    if(!_window)
        return true;

    refuseCloseWhileBlocked();
    return GLFW_TRUE == glfwWindowShouldClose(_window);
}

void RetroFuturaGUI::Window::Interact()
{
    if(!_window)
        return;

    if(IsInTab())
        return; //the window holding its WindowTab has it interact

    //Another window may have interacted last, and the GL calls of a slot below have to land in this window's context
    glfwMakeContextCurrent(_window);

    //Each WindowTab marks itself again when it interacts. One in an inactive scene, which doesn't, takes no dropped windows.
    for(WindowTab* tab : _windowTabs)
    {
        if(!tab)
            continue;

        tab->_isShown = false;
    }

    pollInput();
    emitDroppedFiles(); //before the scenes, so a slot's changes show in this frame
    updateSize(); //so the widgets test the pointer against the new layout
    interactContent();
    setCursorIcon(); //after the content, whose widgets ask for their icons while they interact
    interactPopups();
}

void RetroFuturaGUI::Window::Draw()
{
    if(!_window)
        return;

    if(IsInTab())
        return; //the window holding its WindowTab draws it

    //Another window may have drawn last, and every GL call below has to land in this window's context
    glfwMakeContextCurrent(_window);
    ContextObject::DeletePending(_window); //VAOs and framebuffers of objects destroyed while another window's context was current
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //Color blends as usual. Alpha only ever adds up, so a semi-transparent widget on the opaque background leaves the window
    //opaque there, instead of lowering its alpha and letting the desktop show through. The result is premultiplied by alpha,
    //which is what the compositor expects of a transparent framebuffer.
    glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

    updateProjection();
    drawContent();
    glfwSwapBuffers(_window);

    if(_showAfterDraw)
    {
        _showAfterDraw = false;
        glfwShowWindow(_window);
    }

    glfwPollEvents();
    refuseCloseWhileBlocked();
    followCursor(); //before the docking, which looks at where the window is now
    followDocking(); //After the scenes, because a window bar may have dragged this window while it interacted
    updateDockSnap();
    drawPopups();
}

void RetroFuturaGUI::Window::SetWindowSize(i32 width, i32 height)
{
    SetWidth(width);
    SetHeight(height);
}

void RetroFuturaGUI::Window::SetWidth(i32 width)
{
    if(IsInTab())
    {
        _untabbedSize.x = width;
        return;
    }

    _width = width;
    glfwSetWindowSize(_window, _width, _height);
}

void RetroFuturaGUI::Window::SetHeight(i32 height)
{
    if(IsInTab())
    {
        _untabbedSize.y = height;
        return;
    }

    _height = height;
    glfwSetWindowSize(_window, _width, _height);
}

void RetroFuturaGUI::Window::SetWindowTitle(std::string_view title)
{
    if(!_window)
        return;

    _windowTitle = title;
    glfwSetWindowTitle(_window, _windowTitle.c_str());
}

const std::string& RetroFuturaGUI::Window::GetWindowTitle() const
{
    return _windowTitle;
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

RetroFuturaGUI::Window* RetroFuturaGUI::Window::FromGlfwWindow(GLFWwindow* glfwWindow)
{
    if(!glfwWindow)
        return nullptr;

    return static_cast<Window*>(glfwGetWindowUserPointer(glfwWindow));
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
    glfwSetDropCallback(_window, &Window::dropCallback);

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

    _background = std::make_unique<Plane>(_projection.get());

    if(!_background)
        return;

    _background->SetPosition(glm::vec3((f32)_width * 0.5f, (f32)_height * 0.5f, -1.0f));
    _background->SetSize(glm::vec2((f32)_width, (f32)_height));
    _background->SetRotation(glm::vec3(0.0f));

    _border = std::make_unique<Frame>(_projection.get());

    if(!_border)
        return;

    _border->SetPosition(glm::vec3((f32)_width * 0.5f, (f32)_height * 0.5f, 0.0f));
    _border->SetSize(glm::vec2((f32)_width, (f32)_height));
    _border->SetRotation(glm::vec3(0.0f));
    //No frame until one is asked for, since a Frame starts with a 5 px border
    _border->SetBorderWidth(0.0f);

    SetBackgroundColors(std::array { glm::vec4(0.086f, 0.086f, 0.1137f, 1.0f) }, ColorState::Enabled);
    setBorderColors();
}

std::vector<RetroFuturaGUI::Window*>& RetroFuturaGUI::Window::getAllWindows()
{
    //Deliberately never freed: a window owned by a static is destroyed after every other static, and still takes itself off
    static std::vector<Window*>* windows { new std::vector<Window*>() };
    return *windows;
}