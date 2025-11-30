#include "ShaderManager.hpp"
#include "PlatformBridge.hpp"
#include <print>

using namespace RetroFuturaGUI;

std::unique_ptr<Shader> ShaderManager::_fill;
std::unique_ptr<Shader> ShaderManager::_fillGradient;
std::unique_ptr<Shader> ShaderManager::_lineFill;
std::unique_ptr<Shader> ShaderManager::_textFill;

Shader& ShaderManager::GetFillShader()
{
    return *_fill;
}

Shader& ShaderManager::GetFillGradientShader()
{
    return *_fillGradient;
}

Shader& RetroFuturaGUI::ShaderManager::GetLineFillShader()
{
    return *_lineFill;
}

Shader& RetroFuturaGUI::ShaderManager::GetTextFillShader()
{
    return *_textFill;
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

#ifdef _WIN32 or _WIN64
    exeDir = exeDir.substr(0, exeDir.find_last_of(R"(\)"));
    exeDir.append(R"(\ShaderSource\)");
#else
    exeDir = exeDir.substr(0, exeDir.find_last_of(R"(/)"));
    exeDir.append("/ShaderSource/");
#endif

    _fill = std::make_unique<Shader>(
        std::string(exeDir + "Fill.vs").c_str(), 
        std::string(exeDir + "Fill.fs").c_str()
    );

    _fillGradient = std::make_unique<Shader>(
        std::string(exeDir + "FillGradient.vs").c_str(), 
        std::string(exeDir + "FillGradient.fs").c_str()
    );

    _lineFill = std::make_unique<Shader>(
        std::string(exeDir + "Line.vs").c_str(), 
        std::string(exeDir + "Line.fs").c_str(),
        std::string(exeDir + "Line.gs").c_str()
    );

    _textFill = std::make_unique<Shader>(
        std::string(exeDir + "Text.vs").c_str(), 
        std::string(exeDir + "Text.fs").c_str()
    );
}