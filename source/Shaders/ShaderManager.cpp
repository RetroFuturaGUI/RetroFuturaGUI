#include "ShaderManager.hpp"
#include "PlatformBridge.hpp"
#include <print>

RetroFuturaGUI::Shader& RetroFuturaGUI::ShaderManager::GetFillShader()
{
    return *_fill;
}

RetroFuturaGUI::Shader& RetroFuturaGUI::ShaderManager::GetFillGradientShader()
{
    return *_fillGradient;
}

RetroFuturaGUI::Shader& RetroFuturaGUI::ShaderManager::GetLineFillShader()
{
    return *_lineFill;
}

RetroFuturaGUI::Shader& RetroFuturaGUI::ShaderManager::GetBorderFillShader()
{
    return *_borderFill;
}

RetroFuturaGUI::Shader& RetroFuturaGUI::ShaderManager::GetBorderFillGradientShader()
{
    return *_borderFillGradient;
}

RetroFuturaGUI::Shader& RetroFuturaGUI::ShaderManager::GetTextFillShader()
{
    return *_textFill;
}

RetroFuturaGUI::Shader& RetroFuturaGUI::ShaderManager::GetTextureFillShader()
{
    return *_textureFill;
}

void RetroFuturaGUI::ShaderManager::Init()
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

    _borderFill = std::make_unique<Shader>(
        std::string(exeDir + "BorderFill.vs").c_str(), 
        std::string(exeDir + "BorderFill.fs").c_str()
    );

    _borderFillGradient = std::make_unique<Shader>(
        std::string(exeDir + "BorderFillGradient.vs").c_str(), 
        std::string(exeDir + "BorderFillGradient.fs").c_str()
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

    _textureFill = std::make_unique<Shader>(
        std::string(exeDir + "Texture.vs").c_str(), 
        std::string(exeDir + "Texture.fs").c_str()
    );
}