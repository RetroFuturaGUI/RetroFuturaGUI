#include "ShaderManager.hpp"
#include "PlatformBridge.hpp"
#include <print>

RetroFuturaGUI::Shader& RetroFuturaGUI::ShaderManager::GetPlaneShader(FillType fillType)
{
    return *_planeShaders[static_cast<uSize>(fillType)];
}

RetroFuturaGUI::Shader& RetroFuturaGUI::ShaderManager::GetWavePlaneShader(FillType fillType)
{
    return *_wavePlaneShaders[static_cast<uSize>(fillType)];
}

RetroFuturaGUI::Shader& RetroFuturaGUI::ShaderManager::GetFrameShader(FillType fillType)
{
    return *_frameShaders[static_cast<uSize>(fillType)];
}

RetroFuturaGUI::Shader& RetroFuturaGUI::ShaderManager::GetColorFieldShader(ColorFieldShape shape)
{
    return *_colorFieldShaders[static_cast<uSize>(shape)];
}

RetroFuturaGUI::Shader& RetroFuturaGUI::ShaderManager::GetLineFillShader()
{
    return *_lineFill;
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

RetroFuturaGUI::Shader& RetroFuturaGUI::ShaderManager::GetSvgMaskLinearShiftShader()
{
    return *_svgMaskLinearShift;
}

RetroFuturaGUI::Shader& RetroFuturaGUI::ShaderManager::GetSvgMaskRadialGradientShader()
{
    return *_svgMaskRadialGradient;
}

RetroFuturaGUI::Shader& RetroFuturaGUI::ShaderManager::GetSvgMaskHueStarGradientShader()
{
    return *_svgMaskHueStarGradient;
}

RetroFuturaGUI::Shader& RetroFuturaGUI::ShaderManager::GetYuvToRgbShader()
{
    return *_yuvToRgb;
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

    //One source per class, built once per fill or shape: the define picks it
    constexpr std::array<const char*, _fillTypeCount> fillDefines { "FILL_SOLID", "FILL_LINEAR_SHIFT", "FILL_RADIAL_GRADIENT", "FILL_HUESTAR_GRADIENT" };
    constexpr std::array<const char*, _colorFieldShapeCount> shapeDefines { "SHAPE_CIRCLE", "SHAPE_SQUARE" };
    const std::string quadPath { workingDir + "Quad.vs" };
    const std::string planePath { workingDir + "Plane.fs" };
    const std::string framePath { workingDir + "Frame.fs" };
    const std::string colorFieldPath { workingDir + "ColorField.fs" };

    for(uSize fill { 0 }; fill < _fillTypeCount; ++fill)
    {
        _planeShaders[fill] = std::make_unique<Shader>(quadPath.c_str(), planePath.c_str(), nullptr, std::vector<std::string> { fillDefines[fill] });
        _wavePlaneShaders[fill] = std::make_unique<Shader>(quadPath.c_str(), planePath.c_str(), nullptr, std::vector<std::string> { fillDefines[fill], "WAVE_PLANE" });
        _frameShaders[fill] = std::make_unique<Shader>(quadPath.c_str(), framePath.c_str(), nullptr, std::vector<std::string> { fillDefines[fill] });
    }

    for(uSize shape { 0 }; shape < _colorFieldShapeCount; ++shape)
        _colorFieldShaders[shape] = std::make_unique<Shader>(quadPath.c_str(), colorFieldPath.c_str(), nullptr, std::vector<std::string> { shapeDefines[shape] });

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

    _svgMaskLinearShift = std::make_unique<Shader>(
        std::string(workingDir + "SvgMask.vs").c_str(),
        std::string(workingDir + "SvgMaskLinearShift.fs").c_str()
    );

    _svgMaskRadialGradient = std::make_unique<Shader>(
        std::string(workingDir + "SvgMask.vs").c_str(),
        std::string(workingDir + "SvgMaskRadialGradient.fs").c_str()
    );

    _svgMaskHueStarGradient = std::make_unique<Shader>(
        std::string(workingDir + "SvgMask.vs").c_str(),
        std::string(workingDir + "SvgMaskHueStarGradient.fs").c_str()
    );

    _yuvToRgb = std::make_unique<Shader>(
        std::string(workingDir + "YuvToRgb.vs").c_str(),
        std::string(workingDir + "YuvToRgb.fs").c_str()
    );
}