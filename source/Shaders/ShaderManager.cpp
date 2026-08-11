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

RetroFuturaGUI::Shader& RetroFuturaGUI::ShaderManager::GetBorderHueStarGradientShader()
{
    return *_borderHueStarGradient;
}

RetroFuturaGUI::Shader& RetroFuturaGUI::ShaderManager::GetFontAtlasFillShader()
{
    return *_fontAtlasFill;
}

RetroFuturaGUI::Shader& RetroFuturaGUI::ShaderManager::GetTextureFillShader()
{
    return *_textureFill;
}

RetroFuturaGUI::Shader& RetroFuturaGUI::ShaderManager::GetSvgMaskSolidFillShader()
{
    return *_svgMaskSolidFill;
}

RetroFuturaGUI::Shader& RetroFuturaGUI::ShaderManager::GetSvgMaskLinearGradientShader()
{
    return *_svgMaskLinearGradient;
}

RetroFuturaGUI::Shader& RetroFuturaGUI::ShaderManager::GetSvgMaskRadialGradientShader()
{
    return *_svgMaskRadialGradient;
}

RetroFuturaGUI::Shader& RetroFuturaGUI::ShaderManager::GetSvgMaskHueStarGradientShader()
{
    return *_svgMaskHueStarGradient;
}

void RetroFuturaGUI::ShaderManager::Init()
{
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

    _borderHueStarGradient = std::make_unique<Shader>(
        std::string(workingDir + "BorderHueStarGradient.vs").c_str(), 
        std::string(workingDir + "BorderHueStarGradient.fs").c_str()
    );

    _lineFill = std::make_unique<Shader>(
        std::string(workingDir + "Line.vs").c_str(), 
        std::string(workingDir + "Line.fs").c_str(),
        std::string(workingDir + "Line.gs").c_str()
    );

    _fontAtlasFill = std::make_unique<Shader>(
        std::string(workingDir + "FontAtlas.vs").c_str(), 
        std::string(workingDir + "FontAtlas.fs").c_str()
    );

    _textureFill = std::make_unique<Shader>(
        std::string(workingDir + "Texture.vs").c_str(),
        std::string(workingDir + "Texture.fs").c_str()
    );

    _svgMaskSolidFill = std::make_unique<Shader>(
        std::string(workingDir + "SvgMask.vs").c_str(),
        std::string(workingDir + "SvgMaskSolidFill.fs").c_str()
    );

    _svgMaskLinearGradient = std::make_unique<Shader>(
        std::string(workingDir + "SvgMask.vs").c_str(),
        std::string(workingDir + "SvgMaskLinearGradient.fs").c_str()
    );

    _svgMaskRadialGradient = std::make_unique<Shader>(
        std::string(workingDir + "SvgMask.vs").c_str(),
        std::string(workingDir + "SvgMaskRadialGradient.fs").c_str()
    );

    _svgMaskHueStarGradient = std::make_unique<Shader>(
        std::string(workingDir + "SvgMask.vs").c_str(),
        std::string(workingDir + "SvgMaskHueStarGradient.fs").c_str()
    );
}