#include "ShaderManager.hpp"
#include "PlatformBridge.hpp"
#include <print>

using namespace RetroFuturaGUI;

std::unique_ptr<Shader> ShaderManager::_fill;
std::unique_ptr<Shader> ShaderManager::_fillGradient;

Shader& ShaderManager::GetFillShader()
{
    return *_fill;
}

Shader& ShaderManager::GetFillGradientShader()
{
    return *_fillGradient;
}

void ShaderManager::Init()
{
    SetConsoleOutputCP(CP_UTF8);
    std::println("Compiling shaders...");
    compileShaders();
    std::println("Shaders compiled.");
}

void RetroFuturaGUI::ShaderManager::compileShaders()
{
    std::string exeDir = PlatformBridge::Paths::GetExecutablePath();

#ifdef _WIN32 or _Win64
    exeDir = exeDir.substr(0, exeDir.find_last_of(R"(\)"));
    exeDir.append(R"(\ShaderSource\)");
#else
    exeDir = exeDir.substr(0, exeDir.find_last_of(R"(/)"));
    exeDir.append("/ShaderSource/");
#endif

    _fill = std::make_unique<Shader>(
        std::string(exeDir + "fill.vs").c_str(), 
        std::string(exeDir + "fill.fs").c_str()
    );

    _fillGradient = std::make_unique<Shader>(
        std::string(exeDir + "fillGradient.vs").c_str(), 
        std::string(exeDir + "fillGradient.fs").c_str()
    );
}