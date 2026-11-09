/* Every header from outside the library that RetroFuturaGUI's headers and sources include, for the RetroFuturaGUI module
   (RETROFUTURAGUI_MODULES). The module and each of its implementation units include this ahead of their module
   declaration, so these headers stay outside the module; the same includes inside the library's own headers then find
   them included already and add nothing. A header missing here would end up inside the module, which fails to compile
*/
#pragma once
#include "config.hpp"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <concepts>
#include <optional>
#include <span>
#include <vector>