#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string_view>
#include "config.hpp"

enum class FillType: u32
{
    SOLID,
    GRADIENT
};

enum class WidgetTypeID : i32
{
    None,
    Window,
    Grid,
    Label,
    Button,
    Unknown = -1
};

struct IdentityParams
{
    std::string_view _Name;
    void* _Parent;
    WidgetTypeID _ParentTypeID;
    GLFWwindow* _ParentWindow;
};