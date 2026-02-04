#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string_view>
#include "config.hpp"
#include "Projection.hpp"

namespace RetroFuturaGUI
{
    enum class FillType: u32
    {
        SOLID,
        GRADIENT,
        RADIAL_GRADIENT
    };

    enum class WidgetTypeID : i32
    {
        None,
        Window,
        WindowBar,
        Grid2d,
        Label,
        Button,
        Unknown = -1
    };   

    enum class SizingMode : u32
    {
        FIXED,
        FILL,
        FILL_X,
        FILL_Y
    };

    struct IdentityParams
    {
        std::string_view _Name;
        void* _Parent;
        WidgetTypeID _ParentTypeID;
        GLFWwindow* _ParentWindow;
    };

    struct GeometryParams2D
    {
        const RetroFuturaGUI::Projection& _Projection;
        const glm::vec2& _Position;
        const glm::vec2& _Size;
        f32 _Rotation;
    };

    enum ColorState : u32
    {
        Enabled,
        Disabled,
        Clicked,
        Hover
    };
}