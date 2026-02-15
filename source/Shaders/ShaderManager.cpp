#include "ShaderManager.hpp"
#include "PlatformBridge.hpp"
#include <print>

RetroFuturaGUI::Shader& RetroFuturaGUI::ShaderManager::GetSolidFillShader()
{
    return *_solidFill;
}

RetroFuturaGUI::Shader& RetroFuturaGUI::ShaderManager::GetLinearGradientShader()
{
    return *_linearGradient;
}

RetroFuturaGUI::Shader& RetroFuturaGUI::ShaderManager::GetRadialGradientShader()
{
    return *_radialGradient;
}

RetroFuturaGUI::Shader& RetroFuturaGUI::ShaderManager::GetHueStarGradientShader()
{
    return *_hueStarGradient;
}

RetroFuturaGUI::Shader& RetroFuturaGUI::ShaderManager::GetLineFillShader()
{
    return *_lineFill;
}

RetroFuturaGUI::Shader& RetroFuturaGUI::ShaderManager::GetBorderSolidFillShader()
{
    return *_borderSolidFill;
}

RetroFuturaGUI::Shader& RetroFuturaGUI::ShaderManager::GetBorderLinearGradientShader()
{
    return *_borderLinearGradient;
}

RetroFuturaGUI::Shader& RetroFuturaGUI::ShaderManager::GetBorderRadialGradientShader()
{
    return *_borderRadialGradient;
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

    _solidFill = std::make_unique<Shader>(
        std::string(workingDir + "SolidFill.vs").c_str(), 
        std::string(workingDir + "SolidFill.fs").c_str()
    );

    _linearGradient = std::make_unique<Shader>(
        std::string(workingDir + "LinearGradient.vs").c_str(), 
        std::string(workingDir + "LinearGradient.fs").c_str()
    );

    _radialGradient = std::make_unique<Shader>(
        std::string(workingDir + "RadialGradient.vs").c_str(), 
        std::string(workingDir + "RadialGradient.fs").c_str()
    );

    _hueStarGradient = std::make_unique<Shader>(
        std::string(workingDir + "HueStarGradient.vs").c_str(), 
        std::string(workingDir + "HueStarGradient.fs").c_str()
    );

    _borderSolidFill = std::make_unique<Shader>(
        std::string(workingDir + "BorderSolidFill.vs").c_str(), 
        std::string(workingDir + "BorderSolidFill.fs").c_str()
    );

    _borderLinearGradient = std::make_unique<Shader>(
        std::string(workingDir + "BorderLinearGradient.vs").c_str(), 
        std::string(workingDir + "BorderLinearGradient.fs").c_str()
    );

    _borderRadialGradient = std::make_unique<Shader>(
        std::string(workingDir + "BorderRadialGradient.vs").c_str(), 
        std::string(workingDir + "BorderRadialGradient.fs").c_str()
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