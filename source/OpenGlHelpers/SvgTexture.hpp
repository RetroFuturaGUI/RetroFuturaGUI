#pragma once
#include "IncludeHelper.hpp"
#include "Projection.hpp"
#include "config.hpp"
#include <string>
#include <string_view>
#include <vector>
#include <memory>
#include <lunasvg.h>

namespace RetroFuturaGUI
{
    /// @brief Runtime color-effect override for a single named SVG path.
    struct SvgPathFill
    {
        FillType fillType { FillType::SOLID };
        std::vector<glm::vec4> colors;
        f32 gradientDegree { 0.0f };
        f32 gradientOffset { 0.0f };
        f32 gradientAnimationSpeed { 0.0f };
        f32 gradientRotationSpeed { 0.0f };
    };

    /// @brief Identity and placement of a named path/shape discovered inside an SVG document.
    struct SvgPathInfo
    {
        std::string name;
        glm::vec4 bounds; // x, y, w, h in the SVG's intrinsic user-space units
    };

    /// @brief Renders an SVG icon whose individually named (id-bearing) paths can be
    /// recolored at runtime with the same FillType effects Rectangle uses, without
    /// re-rasterizing the vector data on every color change.
    class SvgTexture
    {
    public:
        /// @brief Loads an SVG
        /// @param projection Only needed if this texture will draw itself as a standalone quad. Call SetProjection() later if there is no projection available yet
        /// @param initialResolution Target pixel size for the initial rasterization; {0,0} uses the SVG's intrinsic size.
        SvgTexture(std::string_view path, const bool flipVertically, Projection* projection, glm::i32vec2 initialResolution = glm::i32vec2(0));
        SvgTexture(const SvgTexture&) = delete;
        SvgTexture& operator=(const SvgTexture&) = delete;
        SvgTexture(SvgTexture&& other) noexcept;
        SvgTexture& operator=(SvgTexture&& other) noexcept;
        ~SvgTexture();

        /// @brief Draws the base layer plus any actively-overridden named path layers.
        void Draw();

        /// @brief Sets the desired Projection
        void SetProjection(Projection* projection);

        /// @brief Sets the size of the standalone quad. Pure GPU-side transform; never re-rasterizes.
        void SetSize(const glm::vec2& size);

        /// @brief Sets the world position of the standalone quad.
        void SetPosition(const glm::vec3& position);

        /// @brief Sets the rotation of the standalone quad.
        void SetRotation(const glm::vec3& rotation);

        /// @brief Re-rasterizes the base layer and every already-materialized path mask at
        /// targetResolution. The ONLY method that touches the CPU rasterizer or re-uploads
        /// pixel data - never called implicitly by SetSize/SetPosition/SetRotation. Callers
        /// should invoke this on resize-end, not on every resize-delta.
        void Rasterize(glm::i32vec2 targetResolution);

        /// @brief Returns every named (id-bearing) path/shape discovered in the SVG.
        const std::vector<SvgPathInfo>& GetNamedPaths() const;

        /// @brief Sets or replaces a color-effect override for a named path. Lazily
        /// materializes that path's alpha mask on first call.
        void SetPathFill(std::string_view name, const SvgPathFill& fill);

        /// @brief Clears a path's override. Its mask texture (if materialized) stays cached.
        void ClearPathFill(std::string_view name);

        /// @brief Returns the pixel resolution of the texture.
        glm::i32vec2 GetResolution() const;

        /// @brief Returns the width-to-height aspect ratio of the texture.
        f32 GetAspectRatio() const;

        /// @brief Returns whether the texture data was flipped vertically on load.
        bool IsTextureVerticallyFlipped() const;

        /// @brief Returns the OpenGL texture object ID of the base (unrecolored) layer.
        u32 GetBaseLayerTextureID() const;

    private:
        struct PathLayer
        {
            std::string name;
            lunasvg::Element element;
            u32 maskTextureId { 0 };
            bool hasActiveOverride { false };
            SvgPathFill fill;
        };

        void loadDocument(std::string_view path);
        void discoverNamedPaths();
        void rasterizeBaseLayer(glm::i32vec2 targetResolution);
        void materializeMask(PathLayer& layer, glm::i32vec2 targetResolution);
        lunasvg::Matrix computeScaleMatrix(glm::i32vec2 targetResolution) const;
        void uploadBaseLayer(const std::vector<u8>& rgba, glm::i32vec2 resolution);
        void uploadMask(PathLayer& layer, const std::vector<u8>& alphaOnly, glm::i32vec2 resolution);
        void setupQuad();
        void drawMaskedLayer(PathLayer& layer);
        PathLayer* findLayer(std::string_view name);
        void releaseGPUResources();

        std::unique_ptr<lunasvg::Document> _document;
        glm::vec2 _documentIntrinsicSize { 0.0f };
        std::vector<PathLayer> _layers;
        std::vector<SvgPathInfo> _pathInfoCache;

        u32 _baseLayerTextureId { 0 };
        bool _verticallyFlipped = false;
        glm::i32vec2 _resolution { 0 };
        f32 _aspectRatio { 0.0f };
        std::string _path {};

    // Standalone quad rendering (only used when constructed with a Projection) — reused for
    // the base layer draw and every masked-path overlay draw
        f32 _vertices[(3 + 2) * 4] =
        {   //   positions     | tex coords
            -0.5f, -0.5f, 0.0f, 0.0f, 0.0f,
             0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
             0.5f,  0.5f, 0.0f, 1.0f, 1.0f,
            -0.5f,  0.5f, 0.0f, 0.0f, 1.0f
        };
        u32 _indices[6] =
        {
            0, 1, 2,
            2, 3, 0
        };
        u32
            _vao { 0 },
            _vbo { 0 },
            _ebo { 0 };
        Projection* _projection { nullptr };
        glm::mat4
            _scalingMatrix { 1.0f },
            _translationMatrix { 1.0f },
            _rotationMatrix { 1.0f };
    };
}
