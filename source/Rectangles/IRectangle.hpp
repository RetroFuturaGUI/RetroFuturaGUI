#pragma once
#include "IncludeHelper.hpp"
#include "ContextObjects.hpp"
#include "Shader.hpp"
#include "Projection.hpp"
#include <memory>
#include <span>

namespace RetroFuturaGUI
{
    enum class RasterPattern : u32
    {
        Dotted,
        Checkered
    };

    /// @brief The base of every rectangle: its size, position and rotation, and the quad all rectangles share. It also keeps
    ///        the fill, the corners, the glass and the raster, which Plane and Frame make public.
    class IRectangle
    {
    public:
        /// @brief Constructs a rectangle that draws using the given projection.
        IRectangle(Projection* projection);
        virtual ~IRectangle();
        IRectangle(const IRectangle&) = delete;
        IRectangle(IRectangle&&) = delete;
        auto operator=(const IRectangle&) = delete;
        auto operator=(IRectangle&&) = delete;

        /// @brief Draws the rectangle.
        virtual void Draw() = 0;

        /// @brief Sets the size of the rectangle.
        void SetSize(const glm::vec2& size);

        /// @brief Sets the world position of the rectangle.
        void SetPosition(const glm::vec3& position);

        /// @brief Sets the rotation of the rectangle.
        void SetRotation(const glm::vec3& rotation);

        /// @brief Returns the current size of the rectangle.
        const glm::vec2& GetSize() const;

        /// @brief Returns the current world position of the rectangle.
        const glm::vec3& GetPosition() const;

        /// @brief Returns the current world rotation of the rectangle.
        const glm::vec3& GetRotation() const;

        /// @brief Returns the projection the rectangle draws with.
        Projection& GetProjection() const;

    protected:
        /// @brief The features a shader runs, as the bits of uDIP. Each feature's setter switches its own bit.
        enum ShaderFeatures : u32
        {
            RoundedCorners = 1,
            GlassEffect = 1 << 1,
            GlassEffectWithImage = GlassEffect + (1 << 2),
            Raster = 1 << 3,
            FogEffect = 1 << 4,
            Wave = 1 << 5,
            BackgroundImage = 1 << 6
        };

    //Fill, corners, glass and raster: public on Plane and Frame
        /// @brief Sets the colors of the fill, read according to the current FillType. Keeps a span over them.
        void SetColors(std::span<glm::vec4> colors);

        /// @brief Sets the fill type (solid, linear shift, radial or hue star gradient).
        void SetFillType(const FillType fillType);

        /// @brief Sets the per-corner rounding radii. Any radius above 0 rounds the corners.
        void SetCornerRadii(const glm::vec4& radii);

        /// @brief Sets the offset applied to the gradient's start position.
        void SetGradientOffset(const f32 gradientOffset);

        /// @brief Sets the speed at which the gradient animates over time.
        void SetGradientAnimationSpeed(const f32 animationSpeed);

        /// @brief Sets the angle of a linear shift, in degrees.
        void SetGradientDegree(const f32 degree);

        /// @brief Sets the speed at which the gradient rotates over time.
        void SetGradientRotationSpeed(const f32 rotationSpeed);

        /// @brief Returns the offset of the gradient's start position.
        f32 GetGradientOffset() const;

        /// @brief Returns the angle of the gradient, in degrees.
        f32 GetGradientDegree() const;

        /// @brief Sets the window-wide backdrop the glass shows, which turns the glass on.
        void SetWindowBackgroundImageTextureID(const u32 textureID);

        /// @brief Sets which pattern the raster draws.
        void SetRasterPattern(const RasterPattern rasterPattern);

        /// @brief Sets the primary raster color: the dot color for Dotted, and one of the two square colors for Checkered. Alpha blends over whatever the shader has already drawn.
        void SetPrimaryRasterColor(const glm::vec4& color);

        /// @brief Sets the second of the two square colors used by the Checkered raster pattern. Alpha blends over whatever the shader has already drawn.
        void SetSecondaryRasterColor(const glm::vec4& color);

        /// @brief Sets the spacing between dot centers, in pixels, for the Dotted raster pattern.
        void SetDotDistance(const f32 distance);

        /// @brief Sets the direction, in degrees, along which raster widths are sampled from PrimaryRasterWidthTransfer and along which the pattern animates. Checkered turns its whole board by this angle.
        void SetRasterDegree(const f32 degree);

        /// @brief Sets the per-position raster width curve, in pixels, sampled along the RasterDegree direction across the rectangle. Each value is a dot's full width across for Dotted, and a square's side length for Checkered. Turns the raster on when non-empty.
        void SetPrimaryRasterWidthTransfer(std::span<f32> widthTransfer);

        /// @brief Sets how far each dot's opacity reaches from its center before fading to transparent. 1.0 = fully opaque disc, 0.5 = opaque inner half then fades out, 0.0 = only the center pixel is opaque.
        void SetDotTransparencyTransfer(const f32 transparencyTransfer);

        /// @brief Sets the speed at which the raster pattern animates along the RasterDegree direction.
        void SetRasterAnimationSpeed(const f32 animationSpeed);

    //Drawing
        /// @brief Uploads the projection, the three matrices and the size in pixels.
        void uploadTransform(Shader& shader) const;

        /// @brief Uploads the feature bits, the corner radii and the fill, the raster and the glass when they are on. A gradient
        ///        moves on by one step.
        void uploadFill(Shader& shader);

        /// @brief Draws the shared quad with the shader in use.
        static void drawQuad();

    //Geometry
        Projection& _projection;
        glm::mat4
            _scalingMatrix { 1.0f },
            _translationMatrix { 1.0f },
            _rotationMatrix { 1.0f };
        glm::vec2 _scale { 0.01f };
        glm::vec3
            _position { 0.0f },
            _rotation { 0.0f };

    //Fill
        std::span<glm::vec4> _colors; //this considers the color source may change many during runtime
        FillType _fillType { FillType::Solid };
        uSize _colorCount { 0 };
        f32
            _gradientOffset { 0.0f },
            _gradientAnimationSpeed { 0.0003f },
            _gradientDegree { 45.0f },
            _gradientRotationSpeed { 0.02f };
        glm::vec4 _cornerRadii { 0.0f };
        u32
            _shaderFeatureDIP { 0 },
            _windowBackgroundTextureID { 0 };

    //Raster (ShaderFeatures::Raster) - the transfer, the colors, the angle and the animation serve every pattern;
    //_dotDistance and _dotTransparencyTransfer are Dotted's own.
        RasterPattern _rasterPattern { RasterPattern::Dotted };
        std::span<f32> _primaryRasterWidthTransfer;
        uSize _primaryRasterWidthTransferCount { 0 };
        glm::vec4
            _primaryRasterColor { 1.0f },
            _secondaryRasterColor { 1.0f };
        f32
            _dotDistance { 15.0f },
            _rasterDegree { 45.0f },
            _dotTransparencyTransfer { 1.0f },
            _rasterAnimationSpeed { 0.0f },
            _rasterAnimationOffset { 0.0f };

    private:
    //The quad every rectangle draws, made with the first rectangle and deleted with the last
        static constexpr f32 _quadVertices[3 * 4] =
        {
            -0.5f, -0.5f, 0.0f,
             0.5f, -0.5f, 0.0f,
             0.5f,  0.5f, 0.0f,
            -0.5f,  0.5f, 0.0f
        };

        static constexpr u32 _quadIndices[3 * 2] =
        {
            0, 1, 2,
            2, 3, 0
        };

        static inline u32
            _quadVbo { 0 },
            _quadEbo { 0 };
        static inline std::unique_ptr<VertexArray> _quadVertexArray { nullptr }; //one VAO per context, over the same buffers
        static inline uSize _rectangleCount { 0 };

        static void createQuad();
        static void deleteQuad();

        void uploadRaster(Shader& shader);
        void uploadDotUniforms(Shader& shader) const;
        void uploadCheckeredUniforms(Shader& shader) const;
    };
}
