/* Shared OpenGL includes and the common enums (fill type, widget type ID, sizing
    mode, color state) used across the rendering and widget headers.
*/
#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "config.hpp"
#include <concepts>

namespace RetroFuturaGUI
{
    class Window;
    class WindowBar;
    class Lasagna;
    class Label;
    class Button;
    class TextBox;
    class Model;
    class Image;
    class SvgImage;
    class CheckBox;
    class RadioButton;
    class RadioButtonGroup;
    class Slider;
    class ProgressBar;
    class Table;
    class Prefab;

    enum class FillType: u32
    {
        SOLID,
        LINEAR_GRADIENT,
        RADIAL_GRADIENT,
        HUESTAR_GRADIENT
    };

    template<typename T>
    concept WidgetTypes =
           std::same_as<T, Window> || std::same_as<T, Label>  || std::same_as<T, Button> 
        || std::same_as<T, WindowBar> || std::same_as<T, Lasagna> || std::same_as<T, TextBox>
        || std::same_as<T, Model> || std::same_as<T, Image> || std::same_as<T, SvgImage> 
        || std::same_as<T, CheckBox> || std::same_as<T, RadioButton> || std::same_as<T, RadioButtonGroup>
        || std::same_as<T, Slider> || std::same_as<T, ProgressBar> || std::same_as<T, Table>
        || std::same_as<T, Prefab>;

    enum class WidgetTypeID : i32
    {
        None,
        Window,
        WindowBar,
        Lasagna,
        Label,
        Button,
        TextBox,
        Model,
        Image,
        SvgImage,
        CheckBox,
        RadioButton,
        RadioButtonGroup,
        Slider,
        ProgressBar,
        Table,
        Prefab,
        ComboBox,
        ExtendedComboBox,
        SeparatorLine,
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
        FILL_XZ,
        FILL_YZ,
        FILL_XYZ
    };

    enum ColorState : u32
    {
        Enabled,
        Disabled,
        Clicked,
        Hover,
        Selected
    };
}