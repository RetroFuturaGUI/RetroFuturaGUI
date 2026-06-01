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
        LINEAR_GRADIENT,
        RADIAL_GRADIENT,
        HUESTAR_GRADIENT
    };

    enum class WidgetTypeID : i32
    {
        None,
        Window,
        WindowBar,
        Lasagna,
        Label,
        Button,
        Unknown = -1
    };   

    enum class SizingMode : u32
    {
        FIXED,
        FILL,
        FILL_X,
        FILL_Y,
        FILL_Z,
        FILL_XY,
    };

    struct IdentityParams
    {
        std::string_view _Name;
        void* _Parent;
        WidgetTypeID _ParentTypeID;
        GLFWwindow* _ParentWindow;
    };

    struct GeometryParams3D
    {
        const RetroFuturaGUI::Projection& _Projection;
        const glm::vec3& _Position;
        const glm::vec3& _Size;
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