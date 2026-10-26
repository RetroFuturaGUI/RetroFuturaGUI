#pragma once
#include "ITexture.hpp"
#include "ContextObjects.hpp"
#include <memory>
#include "config.hpp"
#include <array>

// FFmpeg's types, declared rather than included: only VideoTexture.cpp needs their insides
struct AVFrame;
struct SwsContext;

namespace RetroFuturaGUI
{
    /// @brief A texture whose pictures come from a video.
    class VideoTexture final : public ITexture
    {
    public:
        VideoTexture();
        VideoTexture(const VideoTexture&) = delete;
        VideoTexture& operator=(const VideoTexture&) = delete;
        VideoTexture(VideoTexture&&) = delete;
        VideoTexture& operator=(VideoTexture&&) = delete;
        ~VideoTexture() override;

        /// @brief Shows a decoded picture: uploads its planes and converts them into the texture.
        bool Update(const AVFrame* frame);

        /// @brief Whether a picture has been shown yet. Before the first Update the texture is empty.
        bool HasPicture() const;

    private:
        /// @brief What the YuvToRgb shader needs for one picture
        struct ColorConversion
        {
            glm::vec3 _Offset { 0.0f };
            glm::vec3 _Scale { 1.0f };
            glm::mat3 _Matrix { 1.0f };
        };

        /// @brief The GL state the conversion pass changes. Update runs between the draws of other widgets, which expect their state exactly as they left it - so it's saved before the pass and restored after.
        struct SavedGlState
        {
            i32
                _DrawFramebuffer { 0 },
                _ReadFramebuffer { 0 },
                _Program { 0 },
                _VertexArray { 0 },
                _ActiveTexture { 0 },
                _UnpackRowLength { 0 },
                _UnpackAlignment { 4 };
            i32 _Viewport[4] {};
            i32 _Textures[3] {};
            GLboolean
                _Blend { GL_FALSE },
                _DepthTest { GL_FALSE },
                _ScissorTest { GL_FALSE },
                _CullFace { GL_FALSE };
        };

        /// @brief (Re)creates the RGBA texture, the plane textures and the framebuffer for a picture size.
        bool allocate(const i32 width, const i32 height);
        const AVFrame* toYuv420p(const AVFrame* frame);
        void uploadPlanes(const AVFrame* frame);
        void convert(const AVFrame* frame);
        void releaseVideoResources();

        /// @brief coefficientRed and coefficientBlue are the red and blue weights of the colour space's luma. everything else follows from them
        static ColorConversion makeConversion(const f32 coefficientRed, const f32 coefficientBlue, const bool fullRange);

        static ColorConversion colorConversionFor(const AVFrame* frame);

        static SavedGlState saveGlState();

        static void setEnabled(const GLenum capability, const GLboolean enabled);

        static void restoreGlState(const SavedGlState& state);

        /// @brief Width or height of a chroma plane in 4:2:0 - half, rounded up for odd sizes
        static i32 chromaSize(const i32 size);

        std::array<u32, 3> _planeTextureIds {};
        std::unique_ptr<Framebuffer> _framebuffer { nullptr }; //one per context that converts a picture, all rendering into the same texture
        std::unique_ptr<VertexArray> _emptyVertexArray { nullptr }; //the conversion pass makes its triangle in the vertex shader, so there is no layout
        SwsContext* _converter { nullptr };
        AVFrame* _convertedFrame { nullptr };
        bool _hasPicture { false };
    };
}