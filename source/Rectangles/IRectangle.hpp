#pragma once
#include "IncludeHelper.hpp"
#include "ContextObjects.hpp"
#include "FillStyle.hpp"
#include "Shader.hpp"
#include "Projection.hpp"
#include <memory>

namespace RetroFuturaGUI
{
    /// @brief The corners cut off straight instead of rounded, as a mask: SetCutCorners(CutCorners::TopLeft | CutCorners::BottomRight).
    ///        A cut corner takes its corner radius as the length cut off along each edge; a corner with a radius of 0 stays square
    enum class CutCorners : u32
    {
        None = 0,
        TopLeft = 1,
        TopRight = 1 << 1,
        BottomRight = 1 << 2,
        BottomLeft = 1 << 3,
        All = TopLeft | TopRight | BottomRight | BottomLeft
    };

    constexpr CutCorners operator |(const CutCorners left, const CutCorners right)
    {
        return static_cast<CutCorners>(static_cast<u32>(left) | static_cast<u32>(right));
    }

    constexpr bool HasCutCorner(const CutCorners corners, const CutCorners corner)
    {
        return 0 != (static_cast<u32>(corners) & static_cast<u32>(corner));
    }

    /// @brief The base of every rectangle: its size, position, rotation and corners, and the quad all rectangles share. The
    ///        fill, the glass and the raster come from FillStyle, and Plane and Frame make them public.
    class IRectangle : public FillStyle
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
        /// @brief Sets the per-corner rounding radii. Any radius above 0 rounds the corners. With transition settings the
        ///        corners move to them over the corner radii transition. Public on Plane and Frame.
        void SetCornerRadii(const glm::vec4& radii);
        const glm::vec4& GetCornerRadii() const;

        /// @brief Returns the corner radii drawn now. On the way to GetCornerRadii while a transition runs. Public on Plane and Frame
        const glm::vec4& GetShownCornerRadii() const;

        /// @brief Sets which corners are cut off straight instead of rounded, by their corner radius. Switches at once, while the
        ///        radii themselves may still be moving. Public on Plane and Frame
        void SetCutCorners(const CutCorners corners);
        CutCorners GetCutCorners() const;

        /// @brief Uploads the projection, the three matrices, the size in pixels, and the corner radii shown now when the
        ///        corners are rounded.
        void uploadTransform(Shader& shader);

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
        CutCorners _cutCorners { CutCorners::None };
        glm::vec4 _cornerRadii { 0.0f }; //the radii given; _shownCornerRadii are drawn, on the way to them while a transition runs

    private:
    //Corner radii transition
        glm::vec4
            _fromCornerRadii { 0.0f },
            _shownCornerRadii { 0.0f };
        std::chrono::steady_clock::time_point _cornerRadiiTransitionStart {};
        bool
            _hasShownCornerRadii { false }, //radii given before the first draw show at once
            _isCornerRadiiTransitionRunning { false };

        /// @brief Moves the shown radii on to where the running transition is now, and ends it once it has arrived.
        void updateShownCornerRadii();

        /// @brief Switches the rounded corners on or off in the shader.
        void setRoundedCornersFeature(const bool rounded);

        /// @brief Whether any of the radii is above 0.
        static bool isRounded(const glm::vec4& radii);

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
    };
}
