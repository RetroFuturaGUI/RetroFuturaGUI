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

RetroFuturaGUI::Shader& RetroFuturaGUI::ShaderManager::GetShadowShader(FillType fillType)
{
    return *_shadowShaders[static_cast<uSize>(fillType)];
}

RetroFuturaGUI::Shader& RetroFuturaGUI::ShaderManager::GetGlowShader(FillType fillType)
{
    return *_glowShaders[static_cast<uSize>(fillType)];
}

RetroFuturaGUI::Shader& RetroFuturaGUI::ShaderManager::GetInnerGlowShader(FillType fillType)
{
    return *_innerGlowShaders[static_cast<uSize>(fillType)];
}

RetroFuturaGUI::Shader& RetroFuturaGUI::ShaderManager::GetColorFieldShader(ColorFieldShape shape)
{
    return *_colorFieldShaders[static_cast<uSize>(shape)];
}

RetroFuturaGUI::Shader& RetroFuturaGUI::ShaderManager::GetLineFillShader()
{
    return *_lineFill;
}

RetroFuturaGUI::Shader& RetroFuturaGUI::ShaderManager::GetTextShader(FillType fillType)
{
    return *_textShaders[static_cast<uSize>(fillType)];
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

RetroFuturaGUI::Shader& RetroFuturaGUI::ShaderManager::GetSvgMaskRadialShiftShader()
{
    return *_svgMaskRadialShift;
}

RetroFuturaGUI::Shader& RetroFuturaGUI::ShaderManager::GetSvgMaskConicShiftShader()
{
    return *_svgMaskConicShift;
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
    constexpr std::array<const char*, _fillTypeCount> fillDefines { "FILL_SOLID", "FILL_LINEAR_SHIFT", "FILL_RADIAL_SHIFT", "FILL_CONIC_SHIFT" };
    constexpr std::array<const char*, _colorFieldShapeCount> shapeDefines { "SHAPE_CIRCLE", "SHAPE_SQUARE" };
    const std::string quadPath { workingDir + "Quad.vs" };
    const std::string planePath { workingDir + "Plane.fs" };
    const std::string framePath { workingDir + "Frame.fs" };
    const std::string shadowPath { workingDir + "Shadow.fs" };
    const std::string colorFieldPath { workingDir + "ColorField.fs" };
    const std::string textVertexPath { workingDir + "Text.vs" };
    const std::string textPath { workingDir + "Text.fs" };

    for(uSize fill { 0 }; fill < _fillTypeCount; ++fill)
    {
        _planeShaders[fill] = std::make_unique<Shader>(quadPath.c_str(), planePath.c_str(), nullptr, std::vector<std::string> { fillDefines[fill] });
        _wavePlaneShaders[fill] = std::make_unique<Shader>(quadPath.c_str(), planePath.c_str(), nullptr, std::vector<std::string> { fillDefines[fill], "WAVE_PLANE" });
        _frameShaders[fill] = std::make_unique<Shader>(quadPath.c_str(), framePath.c_str(), nullptr, std::vector<std::string> { fillDefines[fill] });
        _shadowShaders[fill] = std::make_unique<Shader>(quadPath.c_str(), shadowPath.c_str(), nullptr, std::vector<std::string> { fillDefines[fill] });
        _glowShaders[fill] = std::make_unique<Shader>(quadPath.c_str(), shadowPath.c_str(), nullptr, std::vector<std::string> { fillDefines[fill], "GLOW" });
        _innerGlowShaders[fill] = std::make_unique<Shader>(quadPath.c_str(), shadowPath.c_str(), nullptr, std::vector<std::string> { fillDefines[fill], "INNER_GLOW" });
        _textShaders[fill] = std::make_unique<Shader>(textVertexPath.c_str(), textPath.c_str(), nullptr, std::vector<std::string> { fillDefines[fill] });
    }

    for(uSize shape { 0 }; shape < _colorFieldShapeCount; ++shape)
        _colorFieldShaders[shape] = std::make_unique<Shader>(quadPath.c_str(), colorFieldPath.c_str(), nullptr, std::vector<std::string> { shapeDefines[shape] });

    _lineFill = std::make_unique<Shader>(
        std::string(workingDir + "Line.vs").c_str(),
        std::string(workingDir + "Line.fs").c_str(),
        std::string(workingDir + "Line.gs").c_str()
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

    _svgMaskRadialShift = std::make_unique<Shader>(
        std::string(workingDir + "SvgMask.vs").c_str(),
        std::string(workingDir + "SvgMaskRadialShift.fs").c_str()
    );

    _svgMaskConicShift = std::make_unique<Shader>(
        std::string(workingDir + "SvgMask.vs").c_str(),
        std::string(workingDir + "SvgMaskConicShift.fs").c_str()
    );

    _yuvToRgb = std::make_unique<Shader>(
        std::string(workingDir + "YuvToRgb.vs").c_str(),
        std::string(workingDir + "YuvToRgb.fs").c_str()
    );
}