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

RetroFuturaGUI::Shader& RetroFuturaGUI::ShaderManager::GetRadialGradientShader()
{
    return *_radialGradient;
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
    std::string workingDir = PlatformBridge::Paths::GetWorkingDir();

#ifdef _WIN32
    workingDir.append(R"(\ShaderSource\)");
#else
    workingDir.append("/ShaderSource/");
#endif

    _fill = std::make_unique<Shader>(
        std::string(workingDir + "Fill.vs").c_str(), 
        std::string(workingDir + "Fill.fs").c_str()
    );

    _fillGradient = std::make_unique<Shader>(
        std::string(workingDir + "FillGradient.vs").c_str(), 
        std::string(workingDir + "FillGradient.fs").c_str()
    );

    _radialGradient = std::make_unique<Shader>(
        std::string(workingDir + "RadialGradient.vs").c_str(), 
        std::string(workingDir + "RadialGradient.fs").c_str()
    );

    _borderFill = std::make_unique<Shader>(
        std::string(workingDir + "BorderFill.vs").c_str(), 
        std::string(workingDir + "BorderFill.fs").c_str()
    );

    _borderFillGradient = std::make_unique<Shader>(
        std::string(workingDir + "BorderFillGradient.vs").c_str(), 
        std::string(workingDir + "BorderFillGradient.fs").c_str()
    );

    _lineFill = std::make_unique<Shader>(
        std::string(workingDir + "Line.vs").c_str(), 
        std::string(workingDir + "Line.fs").c_str(),
        std::string(workingDir + "Line.gs").c_str()
    );

    _textFill = std::make_unique<Shader>(
        std::string(workingDir + "Text.vs").c_str(), 
        std::string(workingDir + "Text.fs").c_str()
    );

    _textureFill = std::make_unique<Shader>(
        std::string(workingDir + "Texture.vs").c_str(), 
        std::string(workingDir + "Texture.fs").c_str()
    );
}